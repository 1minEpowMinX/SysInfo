#include "integration_server.h"

#include "core/settings/settings_manager.h"
#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QByteArrayList>
#include <QHostAddress>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>

#include <algorithm>

namespace {

/**
 * @brief Lists the officially published SysInfo extension IDs.
 *
 * Used when SettingsManager::allowedExtensionIds() returns empty (the
 * common "out of the box" case). Administrators can override the list
 * via the Integration/AllowedExtensionIds key without rebuilding.
 */
const QStringList kDefaultAllowedExtensionIds = {
    QStringLiteral("mjdcgdoembmihkaajaabkffkejompofj"),  // Chrome / Edge prod
    QStringLiteral("sysinfo-addon@pivdenny.ua"),          // Firefox prod
};

/// Generous bound on an extension identifier: a Chromium ID is 32 characters,
/// a Firefox per-installation UUID is 36.
constexpr qsizetype kMaxExtensionIdLength = 64;

/// Characters permitted in the identifier half of an extension origin: a
/// superset of the two real formats (a Chromium ID is 32 lowercase letters, a
/// Firefox UUID is hex digits and hyphens), admitting anything that is neither
/// a separator, whitespace nor a control byte.
bool isExtensionIdChar(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '-';
}

/**
 * @brief Reports whether @p origin is a well-formed browser-extension URL.
 *
 * Extension SWs in every supported browser stamp Origin as
 *   chrome-extension://<id>     (Chrome / Chromium)
 *   moz-extension://<uuid>      (Firefox; UUID is per-installation)
 *   edge-extension://<id>       (Edge — Chromium variant)
 *
 * A legitimate web page fetching this server cross-origin will instead
 * stamp Origin as https://<host> — never matches these prefixes, so this
 * one check filters out the entire class of cross-site JS attacks.
 *
 * Validation covers the identifier after the scheme as well: applyCors()
 * echoes this exact value back in Access-Control-Allow-Origin, so the
 * character whitelist is what bounds the bytes that can reach a response
 * header.
 */
bool isBrowserExtensionOrigin(const QByteArray& origin)
{
    static const QByteArrayList kSchemes = {
        QByteArrayLiteral("chrome-extension://"),
        QByteArrayLiteral("moz-extension://"),
        QByteArrayLiteral("edge-extension://"),
    };

    for (const QByteArray& scheme : kSchemes) {
        if (!origin.startsWith(scheme)) {
            continue;
        }

        const char* const idBegin = origin.constData() + scheme.size();
        const char* const idEnd   = origin.constData() + origin.size();
        if (idBegin == idEnd || idEnd - idBegin > kMaxExtensionIdLength) {
            return false;
        }
        return std::all_of(idBegin, idEnd, isExtensionIdChar);
    }
    return false;
}

/**
 * @brief Detects whether the request is sent by a browser at all.
 *
 * Browsers stamp at least one of two header families:
 *   - Sec-Fetch-* — Chrome 76+, Firefox 90+, all Chromium-Edge;
 *   - Origin — for every cross-origin or non-GET request.
 *
 * Both header names are forbidden — page JS cannot suppress or fake them.
 * curl / Postman / Qt tests / native clients send neither.
 */
bool isFromBrowser(const QHttpServerRequest& req)
{
    const auto& headers = req.headers();
    return !headers.value("Sec-Fetch-Site").toByteArray().isEmpty()
        || !headers.value("Origin").toByteArray().isEmpty();
}

/**
 * @brief Rejects requests arriving from contexts the server never serves.
 *
 * Two filters, both based on browser-stamped, page-untouchable headers:
 *
 *   1. Sec-Fetch-Mode == "navigate"
 *      Direct navigation in the address bar / bookmark / clicked link.
 *      Keeps /systeminfo JSON out of browser history.
 *
 *   2. Origin from a regular web page (https://, http://, ...)
 *      A page making a cross-origin fetch — even with X-Sysinfo-Client
 *      faked. Origin from such a page can never start with
 *      <browser>-extension://, so the prefix test is reliable.
 *
 * Allowed contexts:
 *   - Origin starts with chrome-extension:// / moz-extension:// /
 *     edge-extension:// (legitimate extension SW, regardless of how the
 *     particular browser stamps Sec-Fetch-Site — Firefox sends
 *     "cross-site" here and Chrome "none"; neither enters the decision);
 *   - No Origin and no Sec-Fetch-* (curl, tests, dev tooling).
 */
bool isContextAllowed(const QHttpServerRequest& req)
{
    const auto& headers = req.headers();

    if (headers.value("Sec-Fetch-Mode").toByteArray() == "navigate") {
        return false;
    }

    const QByteArray origin = headers.value("Origin").toByteArray();
    if (origin.isEmpty()) {
        // No Origin — non-browser client (curl, tests). Sec-Fetch-Mode
        // already filtered above.
        return true;
    }

    // Origin present — must be a known extension scheme. A page Origin
    // (https://evil.com, etc.) falls through to false.
    return isBrowserExtensionOrigin(origin);
}

/**
 * @brief Checks the X-Sysinfo-Client claim against the whitelist.
 *
 * The header is a CLAIM, not a PROOF — see IntegrationServer's class
 * docstring. It works as a defence layer because (a) presence of a
 * non-simple header forces a CORS preflight that the browser filters by
 * Origin, and (b) hostile origins are already rejected in isContextAllowed().
 *
 * Policy:
 *   - Browser request:
 *       header missing       ⇒ rejected (old SysInfo extension version,
 *                              foreign extension, or stripped fetch),
 *       header in whitelist  ⇒ allowed,
 *       header but not match ⇒ rejected.
 *   - Non-browser request (no Sec-Fetch-* and no Origin):
 *       header missing       ⇒ allowed (curl, tests, dev tooling),
 *       header in whitelist  ⇒ allowed,
 *       header but not match ⇒ rejected.
 *
 * The browser-vs-non-browser branch is what stops a sibling browser
 * extension that simply forgot (or refused) to set X-Sysinfo-Client
 * from sneaking in via the curl-friendly fallback.
 */
bool isClientAllowed(const QHttpServerRequest& req,
                     const QStringList& allowedIds)
{
    const QByteArray clientId =
        req.headers().value("X-Sysinfo-Client").toByteArray();

    if (clientId.isEmpty()) {
        // Allow only non-browser clients to omit the identifier.
        return !isFromBrowser(req);
    }

    return allowedIds.contains(QString::fromUtf8(clientId));
}

QHttpServerResponse forbidden()
{
    return QHttpServerResponse(QHttpServerResponse::StatusCode::Forbidden);
}

/// How long a sysinfo::collect() snapshot stays good — see
/// IntegrationServer::cachedInfo().
constexpr qint64 kInfoCacheTtlMs = 1000;

} // namespace

using Method = QHttpServerRequest::Method;

IntegrationServer::IntegrationServer(SettingsManager& settings, QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{
    httpServer.route("/systeminfo", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isRequestAllowed(req)) {
            return forbidden();
        }

        // Labels are emitted only for non-browser callers (curl, support
        // tooling, manual inspection). The browser extension localises
        // field names client-side via browser.i18n / chrome.i18n APIs,
        // so it receives the bare data payload — smaller wire format,
        // less duplicated translation logic.
        const sysinfo::Info& data = cachedInfo();
        const QJsonObject info = isFromBrowser(req)
                ? sysinfo::presenter::toJson(data)
                : sysinfo::presenter::toJsonWithLabels(data);

        // Compact: the only readers are the extension and support tooling,
        // both of which parse rather than eyeball the response.
        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(info).toJson(QJsonDocument::Compact));
        applyCors(req, response);
        return response;
    });

    httpServer.route("/status", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isRequestAllowed(req)) {
            return forbidden();
        }

        QHttpServerResponse response("OK");
        applyCors(req, response);
        return response;
    });

    httpServer.route("/version", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isRequestAllowed(req)) {
            return forbidden();
        }

        QJsonObject body;
        body["version"] = QString::fromUtf8(PROJECT_VERSION);
        body["build"]   = QString::fromUtf8(BUILD_DATE);

        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(body).toJson(QJsonDocument::Compact));
        applyCors(req, response);
        return response;
    });

    // CORS preflight (OPTIONS). Preflight is a handshake — it carries no
    // X-Sysinfo-Client (that header is only on the actual GET) and no body.
    // Gating is on context only: if Origin is hostile or it is a navigate
    // request, reject. The actual client-id whitelist is enforced on the
    // GET that follows.
    auto preflight = [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isContextAllowed(req)) {
            return forbidden();
        }

        QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
        applyCors(req, response);
        return response;
    };
    httpServer.route("/systeminfo", Method::Options, preflight);
    httpServer.route("/status",     Method::Options, preflight);
    httpServer.route("/version",    Method::Options, preflight);
}

IntegrationServer::~IntegrationServer()
{
    stop();
}

bool IntegrationServer::start(quint16 port) {
    if (!tcpServer.listen(QHostAddress::LocalHost, port)) {
        return false;
    }

    if (!httpServer.bind(&tcpServer)) {
        tcpServer.close();
        return false;
    }

    return true;
}

void IntegrationServer::stop() {
    if (tcpServer.isListening()) {
        tcpServer.close();
    }
}

quint16 IntegrationServer::boundPort() const {
    return tcpServer.serverPort();
}

bool IntegrationServer::isListening() const {
    return tcpServer.isListening();
}

QStringList IntegrationServer::allowedExtensionIds() const
{
    const QStringList override = m_settings.allowedExtensionIds();
    return override.isEmpty() ? kDefaultAllowedExtensionIds : override;
}

bool IntegrationServer::isRequestAllowed(const QHttpServerRequest& req) const
{
    return isContextAllowed(req)
        && isClientAllowed(req, allowedExtensionIds());
}

const sysinfo::Info& IntegrationServer::cachedInfo() const
{
    if (!m_cacheAge.isValid() || m_cacheAge.hasExpired(kInfoCacheTtlMs)) {
        m_cachedInfo = sysinfo::collect();
        m_cacheAge.start();
    }
    return m_cachedInfo;
}

void IntegrationServer::applyCors(const QHttpServerRequest& req,
                                  QHttpServerResponse& response)
{
    const QByteArray origin = req.headers().value("Origin").toByteArray();

    auto h = response.headers();

    if (isBrowserExtensionOrigin(origin)) {
        // Reflect the exact extension origin. This is the modern
        // best-practice replacement for "*" and works correctly with
        // browsers that distinguish credentialed/uncredentialed
        // requests, plus it makes intent explicit in the response.
        h.append("Access-Control-Allow-Origin", origin);
        h.append("Vary", "Origin");
    } else {
        // Non-browser caller (curl, tests) — no Origin to reflect.
        // "*" is harmless here: nothing on the page side is reading.
        h.append("Access-Control-Allow-Origin", "*");
    }

    h.append("Access-Control-Allow-Methods", "GET");
    h.append("Access-Control-Allow-Headers", "Content-Type, X-Sysinfo-Client");
    response.setHeaders(std::move(h));
}
