#include "integration_server.h"

#include "request_policy.h"
#include "core/settings/extension_whitelist.h"
#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QHostAddress>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

/// Lifts the headers the access decision reads out of a live request.
integration::RequestContext contextOf(const QHttpServerRequest& req)
{
    const auto& headers = req.headers();
    return {
        headers.value("Origin").toByteArray(),
        headers.value("Sec-Fetch-Site").toByteArray(),
        headers.value("Sec-Fetch-Mode").toByteArray(),
        headers.value("X-Sysinfo-Client").toByteArray(),
    };
}

QHttpServerResponse forbidden()
{
    return QHttpServerResponse(QHttpServerResponse::StatusCode::Forbidden);
}

} // namespace

using Method = QHttpServerRequest::Method;

IntegrationServer::IntegrationServer(ExtensionWhitelist& whitelist,
                                     sysinfo::InfoSource& info,
                                     QObject *parent)
    : QObject{parent}
    , m_whitelist(whitelist)
    , m_info(info)
{
    httpServer.route("/systeminfo", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        const integration::RequestContext context = contextOf(req);
        if (!isRequestAllowed(context)) {
            return forbidden();
        }

        // Labels are emitted only for non-browser callers (curl, support
        // tooling, manual inspection). The browser extension localises
        // field names client-side via browser.i18n / chrome.i18n APIs,
        // so it receives the bare data payload — smaller wire format,
        // less duplicated translation logic.
        // Shared with the rest of the application, so a request arriving just
        // after the tray refreshed pays for no collection of its own.
        const sysinfo::Info& data = m_info.current();
        const QJsonObject info = integration::isFromBrowser(context)
                ? sysinfo::presenter::toJson(data)
                : sysinfo::presenter::toJsonWithLabels(data);

        // Compact: the only readers are the extension and support tooling,
        // both of which parse rather than eyeball the response.
        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(info).toJson(QJsonDocument::Compact));
        applyCors(context, response);
        return response;
    });

    httpServer.route("/status", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        const integration::RequestContext context = contextOf(req);
        if (!isRequestAllowed(context)) {
            return forbidden();
        }

        QHttpServerResponse response("OK");
        applyCors(context, response);
        return response;
    });

    httpServer.route("/version", Method::Get,
                     [this](const QHttpServerRequest &req) -> QHttpServerResponse {
        const integration::RequestContext context = contextOf(req);
        if (!isRequestAllowed(context)) {
            return forbidden();
        }

        // The extension reads this to warn the user when SysInfo is too old or
        // too new for the build it has.
        QJsonObject body;
        body["version"] = QString::fromUtf8(PROJECT_VERSION);
        body["build"]   = QString::fromUtf8(BUILD_DATE);

        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(body).toJson(QJsonDocument::Compact));
        applyCors(context, response);
        return response;
    });

    // CORS preflight (OPTIONS). Preflight is a handshake — it carries no
    // X-Sysinfo-Client (that header is only on the actual GET) and no body.
    // Gating is on context only: if Origin is hostile or it is a navigate
    // request, reject. The actual client-id whitelist is enforced on the
    // GET that follows.
    auto preflight = [](const QHttpServerRequest &req) -> QHttpServerResponse {
        const integration::RequestContext context = contextOf(req);
        if (!integration::isContextAllowed(context)) {
            return forbidden();
        }

        QHttpServerResponse response(QHttpServerResponse::StatusCode::NoContent);
        applyCors(context, response);
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
    return integration::effectiveAllowedExtensionIds(m_whitelist.allowedExtensionIds());
}

bool IntegrationServer::isRequestAllowed(const integration::RequestContext& context) const
{
    return integration::isRequestAllowed(context, allowedExtensionIds());
}

void IntegrationServer::applyCors(const integration::RequestContext& context,
                                  QHttpServerResponse& response)
{
    const QByteArray allowOrigin = integration::corsAllowOrigin(context);

    auto h = response.headers();
    h.append("Access-Control-Allow-Origin", allowOrigin);

    if (allowOrigin != "*") {
        // Announced only when an Origin was reflected: with the wildcard the
        // response is the same whoever asked for it.
        h.append("Vary", "Origin");
    }

    h.append("Access-Control-Allow-Methods", "GET");
    h.append("Access-Control-Allow-Headers", "Content-Type, X-Sysinfo-Client");
    response.setHeaders(std::move(h));
}
