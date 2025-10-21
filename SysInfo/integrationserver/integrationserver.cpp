#include "integrationserver.h"
#include "../utils/utils.h"

#include <QJsonObject>
#include <QHttpServerResponse>
#include <QHostAddress>

IntegrationServer::IntegrationServer(QObject *parent)
    : QObject{parent}
{
    // Route: /systeminfo
    IntegrationServer::httpServer.route("/systeminfo", [this]() {
        const QJsonObject info = Utils::toJson(Utils::collectSystemInfo(), true); // Include labels for translation

        QHttpServerResponse response("application/json; charset=utf-8", QJsonDocument(info).toJson());

        auto h = response.headers();
        h.append("Access-Control-Allow-Origin", "*");
        h.append("Access-Control-Allow-Methods", "GET, OPTIONS");
        h.append("Access-Control-Allow-Headers", "Content-Type");
        response.setHeaders(std::move(h));
        return response;
    });

    // Status check route
    IntegrationServer::httpServer.route("/status", [this]() {
        QHttpServerResponse response("OK");

        auto h = response.headers();
        h.append("Access-Control-Allow-Origin", "*");
        h.append("Access-Control-Allow-Methods", "GET, OPTIONS");
        h.append("Access-Control-Allow-Headers", "Content-Type");
        response.setHeaders(std::move(h));
        return response;
    });
}

bool IntegrationServer::start(const quint16 targetPort) {
    if (!tcpServer.listen(QHostAddress::LocalHost, targetPort)) { // Listen only localhost
        return false;
    }

    if (!httpServer.bind(&tcpServer)) {
        return false;
    }

    return true;
}
