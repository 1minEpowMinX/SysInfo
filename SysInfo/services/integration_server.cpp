#include "integration_server.h"
#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QHostAddress>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QJsonObject>

IntegrationServer::IntegrationServer(QObject *parent)
    : QObject{parent}
{
    using Method = QHttpServerRequest::Method;

    httpServer.route("/systeminfo", Method::Get,
                     [](const QHttpServerRequest &req) {

        // Include labels for translation
        const QJsonObject info = sysinfo::presenter::toJsonWithLabels(sysinfo::collect());
        QHttpServerResponse response("application/json; charset=utf-8",
                                     QJsonDocument(info).toJson());
        applyCors(response);
        return response;
    });

    httpServer.route("/status", Method::Get,
                     [](const QHttpServerRequest &req) {

        QHttpServerResponse response("OK");
        applyCors(response);
        return response;
    });
}

IntegrationServer::~IntegrationServer()
{
    stop();
}

bool IntegrationServer::start(quint16 port) {
    if (!tcpServer.listen(QHostAddress::LocalHost, port)) { // Listen only localhost
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

void IntegrationServer::applyCors(QHttpServerResponse &response) {
    auto h = response.headers();
    h.append("Access-Control-Allow-Origin", "*");
    h.append("Access-Control-Allow-Methods", "GET");
    h.append("Access-Control-Allow-Headers", "Content-Type");
    response.setHeaders(std::move(h));
}
