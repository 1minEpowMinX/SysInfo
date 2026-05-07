#include "integration_server.h"

#include "core/settings/settings_manager.h"
#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QHostAddress>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonObject>

namespace {

/**
 * @brief Officially published SysInfo extension IDs.
 *
 * Used when SettingsManager::allowedExtensionIds() returns empty (the
 * common "out of the box" case). Administrators can override the list
 * via the Integration/AllowedExtensionIds key without rebuilding.
 */
const QStringList kDefaultAllowedExtensionIds = {
    QStringLiteral("mjdcgdoembmihkaajaabkffkejompofj"),  // Chrome / Edge prod
    QStringLiteral("sysinfo-addon@pivdenny.ua"),          // Firefox prod
};

/**
 * @brief True if @p origin looks like a browser-extension URL.
 *
 * Extension SWs in every supported browser stamp Origin as
 *   chrome-extension://<id>     (Chrome / Chromium)
 *   moz-extension://<uuid>      (Firefox; UUID is per-installation)
 *   edge-extension://<id>       (Edge — Chromium variant)
 *
 * A legitimate web page that tries to fetch us cross-origin will instead
 * stamp Origin as https://<host> — never matches these prefixes, so this
 * one check filters out the entire class of cross-site JS attacks.
 */
bool isBrowserExtensionOrigin(const QByteArray& origin)
{
    return origin.startsWith("chrome-extension://")
        || origin.startsWith("moz-extension://")
        || origin.startsWith("edge-extension://");
}

/**
 * @brief Detect whether the request is sent by a browser at all.
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
 * @brief Reject requests that come from contexts we never want to serve.
 *
 * Two filters, both based on browser-stamped, page-untouchable headers:
 *
 *   1. Sec-Fetch-Mode == "navigate"
 *      Direct navigation in the address bar / bookmark / clicked link.
 *      We don't want /systeminfo JSON to leak into browser history.
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
 *     "cross-site" here, Chrome sends "none", we no longer care);
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
 * @brief Whitelist check on the X-Sysinfo-Client claim.
 *
 * The header is a CLAIM, not a PROOF — see IntegrationServer's class
 * docstring. It works as a defence layer because (a) presence of a
 * non-simple header forces a CORS preflight that the browser filters by
 * Origin, and (b) we already reject hostile origins in isContextAllowed().
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
        const sysinfo::Info data = sysinfo::collect();
        const QJsonObject info = isFromBrowser(req)
                ? sysinfo::presenter::toJson(data)
                : sysinfo::presenter::toJsonWithLabels(data);

        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(info).toJson());
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

    // CORS preflight (OPTIONS). Preflight is a handshake — it carries no
    // X-Sysinfo-Client (that header is only on the actual GET) and no body.
    // We gate on context only: if Origin is hostile or it's a navigate
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
