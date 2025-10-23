#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include <QHttpServer>
#include <QObject>
#include <QTcpServer>

class IntegrationServer : public QObject
{
    Q_OBJECT
public:
    explicit IntegrationServer(QObject *parent = nullptr);

    bool start(const quint16 targetPort = 8734);

private:
    QHttpServer httpServer;
    QTcpServer tcpServer;
    quint16 targetPort = 0;
};

#endif // INTEGRATIONSERVER_H
