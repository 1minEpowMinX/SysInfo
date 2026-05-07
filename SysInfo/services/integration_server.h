#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include <QHttpServer>
#include <QHttpServerResponse>
#include <QObject>
#include <QStringList>
#include <QTcpServer>

class SettingsManager;

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
 * Security model
 * --------------
 * The server protects against:
 *   - cross-site JS attacks (rejected via Sec-Fetch-Site filter),
 *   - direct navigation in the address bar (rejected via Sec-Fetch-Mode),
 *   - browser callers without a valid X-Sysinfo-Client header (rejected
 *     with 403). This catches stale SysInfo extension builds that have
 *     not yet been updated to send the header, and any sibling browser
 *     extension that does not know to mimic it.
 *   - browser callers that send X-Sysinfo-Client but the value is not
 *     in the SettingsManager whitelist (rejected with 403).
 *
 * The server does NOT protect against:
 *   - local non-browser clients (curl, scripts, malware) — they bypass
 *     CORS entirely. Mitigated by binding to loopback and by the data
 *     being low-sensitivity in our threat model;
 *   - sibling extensions installed in the same browser — extension IDs
 *     are public, X-Sysinfo-Client is a claim and not a proof. Hardening
 *     to "proof" requires native messaging with a shared secret, which
 *     is out of scope for the current version.
 *
 * Lifecycle is explicit: construct → start(port) → stop() / destructor.
 * start() returns false if the port is busy or binding fails — the App
 * layer surfaces this to the user via QMessageBox.
 */
class IntegrationServer : public QObject
{
    Q_OBJECT
public:
    /**
     * @param settings  Application settings store; consulted on every
     *                  request for the AllowedExtensionIds whitelist.
     *                  Must outlive this server.
     * @param parent    Standard Qt parent.
     */
    explicit IntegrationServer(SettingsManager& settings,
                               QObject *parent = nullptr);
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
    /// @return The active whitelist: SettingsManager override if non-empty,
    ///         otherwise the compiled-in default list of officially
    ///         published SysInfo extension IDs.
    QStringList allowedExtensionIds() const;

    /// Run the layered Sec-Fetch-* and X-Sysinfo-Client checks.
    /// @return true if the request should be served, false if rejected.
    bool isRequestAllowed(const QHttpServerRequest& req) const;

    /// Attach CORS headers tailored to this request's Origin.
    /// Called only after isRequestAllowed() has approved the request
    /// (or, for OPTIONS preflight, after isContextAllowed()).
    static void applyCors(const QHttpServerRequest& req,
                          QHttpServerResponse& response);

    SettingsManager& m_settings;     ///< Injected, not owned.
    QHttpServer httpServer;
    QTcpServer  tcpServer;
};

#endif // INTEGRATIONSERVER_H
