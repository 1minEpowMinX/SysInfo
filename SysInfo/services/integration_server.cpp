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
 * @brief Reject requests originating from page contexts.
 *
 * Sec-Fetch-* are forbidden header names — the browser writes them
 * itself, page JS cannot spoof them. We treat:
 *   - Sec-Fetch-Site: cross-site / same-site → JS fetch initiated by a
 *     (potentially malicious) web page;
 *   - Sec-Fetch-Mode: navigate              → direct navigation
 *     (address bar, bookmark, link click). Hides /systeminfo JSON from
 *     the browser history.
 *
 * Allowed contexts: extension service worker (Site:none, Mode:cors) and
 * non-browser clients with no Sec-Fetch-* at all (curl, tests).
 */
bool isContextAllowed(const QHttpServerRequest& req)
{
    const QByteArray site = req.headers().value("Sec-Fetch-Site").toByteArray();
    if (site == "cross-site" || site == "same-site")
        return false;

    const QByteArray mode = req.headers().value("Sec-Fetch-Mode").toByteArray();
    if (mode == "navigate")
        return false;

    return true;
}

/**
 * @brief Whitelist check on the X-Sysinfo-Client claim.
 *
 * The header is a CLAIM, not a PROOF — see IntegrationServer's class
 * docstring. It works as a defence layer because (a) presence of a
 * non-simple header forces a CORS preflight that the browser filters by
 * Origin / Sec-Fetch-Site, and (b) we already reject dangerous contexts
 * in isContextAllowed().
 *
 * Empty header  ⇒ allowed (non-browser client: curl, tests, dev tools).
 * Non-empty     ⇒ must match the whitelist exactly.
 */
bool isClientAllowed(const QHttpServerRequest& req,
                     const QStringList& allowedIds)
{
    const QByteArray header = req.headers().value("X-Sysinfo-Client").toByteArray();
    if (header.isEmpty())
        return true;

    return allowedIds.contains(QString::fromUtf8(header));
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
        if (!isRequestAllowed(req))
            return forbidden();

        const QJsonObject info =
            sysinfo::presenter::toJsonWithLabels(sysinfo::collect());
        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(info).toJson());
        applyCors(response);
        return response;
    });

    httpServer.route("/status", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isRequestAllowed(req))
            return forbidden();

        QHttpServerResponse response("OK");
        applyCors(response);
        return response;
    });

    // Preflight (OPTIONS) for both routes — same gate as GET, no body.
    auto preflight = [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        if (!isRequestAllowed(req))
            return forbidden();

        QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
        applyCors(response);
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

void IntegrationServer::applyCors(QHttpServerResponse &response)
{
    auto h = response.headers();
    h.append("Access-Control-Allow-Origin",  "*");
    h.append("Access-Control-Allow-Methods", "GET");
    h.append("Access-Control-Allow-Headers", "Content-Type, X-Sysinfo-Client");
    response.setHeaders(std::move(h));
}
