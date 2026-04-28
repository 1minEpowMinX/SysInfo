#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include <QHttpServer>
#include <QHttpServerResponse>
#include <QObject>
#include <QTcpServer>

/**
 * @brief Local HTTP endpoint exposing system info to the browser extension.
 *
 * Binds to 127.0.0.1 only — never to a public interface — so the
 * integration is reachable only from this machine. Two GET routes:
 *
 *   - /status     : plain "OK", used by the extension as a health probe.
 *   - /systeminfo : JSON document produced by sysinfo::presenter, including
 *                   localised field labels for the extension UI.
 *
 * CORS headers are wide open (`*`) because the perimeter is closed: the
 * server only accepts loopback connections, and the extension's origin
 * varies per browser. See applyCors().
 *
 * Lifecycle is explicit: construct → start(port) → stop() / destructor.
 * start() returns false if the port is busy or binding fails — the App
 * layer surfaces this to the user via QMessageBox.
 */
class IntegrationServer : public QObject
{
    Q_OBJECT
public:
    explicit IntegrationServer(QObject *parent = nullptr);
    ~IntegrationServer() override;

    /**
     * @brief Bind to 127.0.0.1:@p port and start serving routes.
     * @param port TCP port; pass 0 to let the OS pick an ephemeral one
     *             (useful in tests — read it back via boundPort()).
     * @return true on success; false if the listen() or HTTP bind failed.
     */
    bool start(quint16 port = 8734);

    /// Close the listening socket. Safe to call multiple times.
    void stop();

    /// @return The actual TCP port the server is bound to (0 if not listening).
    quint16 boundPort() const;

    /// @return true if the underlying TCP server is currently accepting connections.
    bool isListening() const;

private:
    /// Attach the same permissive CORS headers to every outgoing response.
    static void applyCors(QHttpServerResponse &response);

    QHttpServer httpServer;
    QTcpServer  tcpServer;
};

#endif // INTEGRATIONSERVER_H
