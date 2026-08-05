#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include "request_policy.h"
#include "core/sysinfo/info_source.h"
#include "core/sysinfo/system_info.h"

#include <QHttpServer>
#include <QHttpServerResponse>
#include <QObject>
#include <QStringList>
#include <QTcpServer>

class ExtensionWhitelist;

/**
 * @brief Serves system info to the browser extension over local HTTP.
 *
 * Binds to 127.0.0.1 only — never to a public interface — so the
 * integration is reachable only from this machine. Three GET routes:
 *
 *   - /status     : plain "OK", used by the extension as a health probe.
 *   - /version    : JSON {"version": "X.Y.Z", "build": "YYYYMMDD"}.
 *   - /systeminfo : JSON document produced by sysinfo::presenter. The
 *                   payload shape varies by caller:
 *                     - browser caller (extension SW)      : bare data
 *                       (hostname / username / ip / uptime);
 *                     - non-browser caller (curl, support) : same data
 *                       PLUS a "labels" object with localised field
 *                       names.
 *
 * Who gets served is decided by integration::isRequestAllowed() in
 * request_policy.h, which also documents the threat model and what it does
 * not cover. This class only lifts the headers out of each request and
 * applies the verdict; the preflight route consults layer 1 alone.
 *
 * Lifecycle is explicit: construct → start(port) → stop() / destructor.
 * start() returns false if the port is busy or binding fails — the App
 * layer decides how to surface that to the user.
 */
class IntegrationServer : public QObject
{
    Q_OBJECT
public:
    /**
     * @param whitelist Source of the administrator override, consulted on
     *                  every request. Not owned; must outlive this server.
     * @param info      Session snapshot the /systeminfo route serves, shared
     *                  with the rest of the application. Not owned; must
     *                  outlive this server.
     * @param parent    Standard Qt parent.
     */
    explicit IntegrationServer(ExtensionWhitelist& whitelist,
                               sysinfo::InfoSource& info,
                               QObject *parent = nullptr);
    ~IntegrationServer() override;

    /**
     * @brief Binds to 127.0.0.1:@p port and starts serving routes.
     * @param port TCP port; pass 0 to let the OS pick an ephemeral one, then
     *             read its choice back via boundPort().
     * @return true on success; false if the listen() or HTTP bind failed.
     */
    bool start(quint16 port = 8734);

    /// Closes the listening socket. Safe to call multiple times.
    void stop();

    /// @return The actual TCP port the server is bound to (0 if not listening).
    quint16 boundPort() const;

    /// @return true if the underlying TCP server is currently accepting connections.
    bool isListening() const;

private:
    /// @return The whitelist in force: the injected override if it carries
    ///         anything, the compiled-in defaults otherwise.
    QStringList allowedExtensionIds() const;

    /// Applies integration::isRequestAllowed() against the whitelist in force.
    /// @param context Headers lifted from the request.
    /// @return true if the request should be served, false if rejected.
    bool isRequestAllowed(const integration::RequestContext& context) const;

    /// Attaches CORS headers tailored to @p context, adding Vary: Origin only
    /// when an Origin was reflected. Called once the request has been approved.
    static void applyCors(const integration::RequestContext& context,
                          QHttpServerResponse& response);

    ExtensionWhitelist&  m_whitelist; ///< Injected, not owned.
    sysinfo::InfoSource& m_info;      ///< Injected, not owned.
    QHttpServer httpServer;           ///< Route table; bound to tcpServer by start().
    QTcpServer  tcpServer;            ///< Listening socket, and the only one bound.
};

#endif // INTEGRATIONSERVER_H
