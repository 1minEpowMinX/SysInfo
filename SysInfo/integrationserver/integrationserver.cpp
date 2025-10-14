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
        return QHttpServerResponse("application/json", QJsonDocument(info).toJson());
    });

    // Status check route
    IntegrationServer::httpServer.route("/status", [this]() {
        return QHttpServerResponse("OK", "Active");
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
