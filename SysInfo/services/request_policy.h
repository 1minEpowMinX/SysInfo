#ifndef REQUEST_POLICY_H
#define REQUEST_POLICY_H

#include <QByteArray>
#include <QStringList>

/**
 * @brief Decides which callers the integration endpoint serves.
 *
 * The whole access decision of IntegrationServer, expressed over the header
 * values alone and never over a live request.
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
 *        - Origin from a regular web page (https://evil.com, etc.) — the
 *          Origin prefix is not chrome-extension://, moz-extension:// or
 *          edge-extension://.
 *        - Origin carrying one of those schemes but a malformed identifier
 *          (empty, over-long, or containing anything outside the permitted
 *          character set).
 *        - Sec-Fetch-Mode == "navigate" — direct navigation in the address
 *          bar / bookmark / link click.
 *
 *      Accepted:
 *        - Origin is one of the three browser-extension URL schemes followed
 *          by a well-formed identifier — the same in Chrome, Firefox and Edge
 *          despite their differing Sec-Fetch-Site values for extension SW
 *          (Firefox sends "cross-site", Chromium "none"; that field does not
 *          enter the context decision).
 *        - No Origin and no Sec-Fetch-* — non-browser client (curl, tests,
 *          dev tooling).
 *
 *   2. Client whitelist (isClientAllowed)
 *      Checks the X-Sysinfo-Client claim against the effective whitelist.
 *
 *      Rejected:
 *        - Browser request without X-Sysinfo-Client.
 *        - X-Sysinfo-Client present but not on the whitelist.
 *
 *      Accepted:
 *        - Non-browser request without X-Sysinfo-Client (curl, tests).
 *        - Any request with a whitelisted X-Sysinfo-Client value.
 *
 * CORS preflight (OPTIONS) gates on layer 1 only — the X-Sysinfo-Client header
 * is by spec carried only on the actual GET, never on the preflight handshake.
 * The follow-up GET passes through both layers.
 *
 * The policy does NOT protect against:
 *   - local non-browser clients (curl, scripts, malware), which bypass CORS
 *     entirely;
 *   - sibling extensions installed in the same browser that know the public
 *     extension IDs and mimic the X-Sysinfo-Client header. Extension IDs are
 *     public (visible in the Web Store / AMO listing), so the header is a
 *     claim, not a proof.
 */
namespace integration {

/**
 * @brief Carries the request headers that bear on the access decision.
 *
 * Every member is the raw header value, empty when the header is absent.
 * Nothing else about a request enters the policy.
 */
struct RequestContext
{
    QByteArray origin;        ///< Origin.
    QByteArray secFetchSite;  ///< Sec-Fetch-Site.
    QByteArray secFetchMode;  ///< Sec-Fetch-Mode.
    QByteArray clientId;      ///< X-Sysinfo-Client.
};

/**
 * @brief Lists the officially published SysInfo extension IDs.
 *
 * Stands in whenever the administrator override is unset, which is the common
 * "out of the box" case.
 *
 * @return The compiled-in whitelist.
 */
const QStringList &defaultAllowedExtensionIds();

/**
 * @brief Resolves which whitelist applies.
 *
 * @param configured Administrator override; empty means "not configured".
 * @return @p configured when it carries anything, the compiled-in defaults
 *         otherwise.
 */
QStringList effectiveAllowedExtensionIds(const QStringList &configured);

/**
 * @brief Reports whether @p origin is a well-formed browser-extension URL.
 *
 * Extension SWs in every supported browser stamp Origin as
 *   chrome-extension://<id>     (Chrome / Chromium)
 *   moz-extension://<uuid>      (Firefox; UUID is per-installation)
 *   edge-extension://<id>       (Edge — Chromium variant)
 *
 * A legitimate web page fetching this server cross-origin instead stamps
 * Origin as https://<host>, which never matches these prefixes.
 *
 * The identifier after the scheme is checked against a character whitelist and
 * a length bound, not by scheme alone.
 *
 * @param origin Raw Origin header value.
 * @return true if the value names a browser extension.
 */
bool isBrowserExtensionOrigin(const QByteArray &origin);

/**
 * @brief Detects whether @p request was sent by a browser at all.
 *
 * Browsers stamp at least one of two header families: Sec-Fetch-* (Chrome 76+,
 * Firefox 90+, all Chromium-Edge) and Origin (for every cross-origin or
 * non-GET request). Both names are forbidden — page JS can neither suppress
 * nor fake them. curl / Postman / Qt tests / native clients send neither.
 *
 * @param request Headers under consideration.
 * @return true if the caller is a browser.
 */
bool isFromBrowser(const RequestContext &request);

/**
 * @brief Applies layer 1, the context filter.
 * @param request Headers under consideration.
 * @return true if the calling context is one the endpoint serves.
 */
bool isContextAllowed(const RequestContext &request);

/**
 * @brief Applies layer 2, the client whitelist.
 * @param request    Headers under consideration.
 * @param allowedIds Effective whitelist, as effectiveAllowedExtensionIds()
 *                   resolves it.
 * @return true if the client claim is acceptable.
 */
bool isClientAllowed(const RequestContext &request, const QStringList &allowedIds);

/**
 * @brief Applies both layers, in order.
 * @param request    Headers under consideration.
 * @param allowedIds Effective whitelist.
 * @return true if the request should be served.
 */
bool isRequestAllowed(const RequestContext &request, const QStringList &allowedIds);

/**
 * @brief Picks the value of the Access-Control-Allow-Origin response header.
 *
 * Reflects the exact extension origin back at a browser-extension caller, and
 * answers "*" to everyone else.
 *
 * @param request Headers under consideration.
 * @return The origin to reflect, or "*".
 */
QByteArray corsAllowOrigin(const RequestContext &request);

} // namespace integration

#endif // REQUEST_POLICY_H
