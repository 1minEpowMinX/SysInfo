#ifndef INTEGRATIONSERVER_H
#define INTEGRATIONSERVER_H

#include "core/sysinfo/system_info.h"

#include <QElapsedTimer>
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
 *                   The extension uses this to detect API-contract
 *                   compatibility and to warn the user if SysInfo is
 *                   too old or too new for the installed extension.
 *   - /systeminfo : JSON document produced by sysinfo::presenter. The
 *                   payload shape varies by caller:
 *                     - browser caller (extension SW)      : bare data
 *                       (hostname / username / ip / uptime). The
 *                       extension localises field names client-side
 *                       through browser.i18n / chrome.i18n.
 *                     - non-browser caller (curl, support) : same data
 *                       PLUS a "labels" object with localised field
 *                       names. Convenient for human inspection in
 *                       support tickets and dev tooling.
 *
 * Security model
 * --------------
 * Two independent layers gate every request, applied in order:
 *
 *   1. Context filter (isContextAllowed)
 *      Origin-based, complemented by Sec-Fetch-Mode. Both headers are
 *      forbidden header names — page JS cannot fake or strip them.
 *
 *      Rejected:
 *        - Origin from a regular web page (https://evil.com, etc.) —
 *          the Origin prefix is not chrome-extension://, moz-extension://
 *          or edge-extension://.
 *        - Origin carrying one of those schemes but a malformed identifier
 *          (empty, over-long, or containing anything outside the permitted
 *          character set). The value is echoed back in the CORS header, so
 *          it is validated in full rather than by scheme alone.
 *        - Sec-Fetch-Mode == "navigate" — direct navigation in the
 *          address bar / bookmark / link click. Hides the JSON from
 *          browser history.
 *
 *      Accepted:
 *        - Origin is one of the three browser-extension URL schemes
 *          followed by a well-formed identifier — the same in Chrome,
 *          Firefox and Edge despite their differing Sec-Fetch-Site values
 *          for extension SW (Firefox sends "cross-site", Chromium "none";
 *          that field does not enter the context decision).
 *        - No Origin and no Sec-Fetch-* — non-browser client (curl,
 *          tests, dev tooling).
 *
 *   2. Client whitelist (isClientAllowed)
 *      Checks the X-Sysinfo-Client header against the compiled-in list of
 *      officially published SysInfo extension IDs, or against the
 *      administrator override when the ExtensionWhitelist supplies one.
 *
 *      Rejected:
 *        - Browser request without X-Sysinfo-Client — catches stale
 *          SysInfo extension builds that have not yet been updated to
 *          send the header, and sibling extensions that do not mimic it.
 *        - X-Sysinfo-Client present but not on the whitelist.
 *
 *      Accepted:
 *        - Non-browser request without X-Sysinfo-Client (curl, tests).
 *        - Any request with a whitelisted X-Sysinfo-Client value.
 *
 * CORS preflight (OPTIONS) gates on layer 1 only — the X-Sysinfo-Client
 * header is by spec carried only on the actual GET, never on the
 * preflight handshake. The follow-up GET still passes through both
 * layers, so this is not a bypass.
 *
 * applyCors() reflects the request Origin into Access-Control-Allow-
 * Origin (with Vary: Origin) for browser-extension callers, and falls
 * back to "*" for non-browser callers that have no Origin to reflect.
 *
 * The server does NOT protect against:
 *   - local non-browser clients (curl, scripts, malware) — they bypass
 *     CORS entirely. Mitigated by binding to loopback and by the data
 *     being low-sensitivity in this threat model;
 *   - sibling extensions installed in the same browser that know the
 *     public extension IDs and decide to mimic the X-Sysinfo-Client
 *     header. Extension IDs are public (visible in the Web Store /
 *     AMO listing), so the header is a claim, not a proof. Hardening
 *     to "proof" requires native messaging with a shared secret, which
 *     the portable distribution model does not carry.
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
     * @param parent    Standard Qt parent.
     */
    explicit IntegrationServer(ExtensionWhitelist& whitelist,
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
    /// @return The active whitelist: the injected override if non-empty,
    ///         otherwise the compiled-in default list of officially
    ///         published SysInfo extension IDs.
    QStringList allowedExtensionIds() const;

    /// Runs both gates: Origin/Sec-Fetch-Mode context filter, then
    /// X-Sysinfo-Client whitelist. See class docstring for the policy.
    /// @return true if the request should be served, false if rejected.
    bool isRequestAllowed(const QHttpServerRequest& req) const;

    /// Attaches CORS headers tailored to this request's Origin.
    /// Called only after isRequestAllowed() has approved the request
    /// (or, for OPTIONS preflight, after isContextAllowed()).
    static void applyCors(const QHttpServerRequest& req,
                          QHttpServerResponse& response);

    /**
     * @brief Returns the session snapshot, recollected at most once per TTL window.
     *
     * sysinfo::collect() enumerates every network interface — on Windows a
     * GetAdaptersAddresses call costing tens of milliseconds — and route
     * handlers run on the thread the server lives in, the GUI thread. Requests
     * arriving within one window share a single collection.
     *
     * @return Cached snapshot, which may trail the true state by up to the TTL.
     */
    const sysinfo::Info& cachedInfo() const;

    ExtensionWhitelist& m_whitelist; ///< Injected, not owned.
    QHttpServer httpServer;          ///< Route table; bound to tcpServer by start().
    QTcpServer  tcpServer;           ///< Listening socket, and the only one bound.

    mutable sysinfo::Info m_cachedInfo;  ///< Valid only while m_cacheAge has not expired.
    mutable QElapsedTimer m_cacheAge;    ///< Invalid until the first collection.
};

#endif // INTEGRATIONSERVER_H
