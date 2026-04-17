#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include <QHttpServer>
#include <QHttpServerResponse>
#include <QObject>
#include <QTcpServer>

class IntegrationServer : public QObject
{
    Q_OBJECT
public:
    explicit IntegrationServer(QObject *parent = nullptr);
    ~IntegrationServer() override;

    bool start(quint16 port = 8734);
    void stop();
    quint16 boundPort() const;
    bool isListening() const;

private:
    static void applyCors(QHttpServerResponse &response);

    QHttpServer httpServer;
    QTcpServer tcpServer;
};

#endif // INTEGRATIONSERVER_H
