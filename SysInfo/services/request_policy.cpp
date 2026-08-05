#include "request_policy.h"

#include <QByteArrayList>
#include <QString>

#include <algorithm>

namespace integration {

// RequestContext rather than QHttpServerRequest, which carries no public way to
// set a header: a decision phrased over the request could only be exercised by
// standing up a real socket.
//
// Two limits are accepted rather than closed. Local non-browser clients bypass
// CORS entirely; that is left to IntegrationServer binding to loopback and to
// the data being low-sensitivity in this threat model. A sibling extension can
// read the public extension IDs off the Web Store / AMO listing and mimic
// X-Sysinfo-Client; turning that claim into a proof needs native messaging with
// a shared secret, which the portable distribution model does not carry.

namespace {

/// Generous bound on an extension identifier: a Chromium ID is 32 characters,
/// a Firefox per-installation UUID is 36.
constexpr qsizetype kMaxExtensionIdLength = 64;

/// Characters permitted in the identifier half of an extension origin: a
/// superset of the two real formats (a Chromium ID is 32 lowercase letters, a
/// Firefox UUID is hex digits and hyphens), admitting anything that is neither
/// a separator, whitespace nor a control byte.
bool isExtensionIdChar(char c)
{
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9')
        || c == '-';
}

} // namespace

const QStringList &defaultAllowedExtensionIds()
{
    static const QStringList ids = {
        QStringLiteral("mjdcgdoembmihkaajaabkffkejompofj"),  // Chrome / Edge prod
        QStringLiteral("sysinfo-addon@pivdenny.ua"),          // Firefox prod
    };
    return ids;
}

QStringList effectiveAllowedExtensionIds(const QStringList &configured)
{
    return configured.isEmpty() ? defaultAllowedExtensionIds() : configured;
}

bool isBrowserExtensionOrigin(const QByteArray &origin)
{
    static const QByteArrayList kSchemes = {
        QByteArrayLiteral("chrome-extension://"),
        QByteArrayLiteral("moz-extension://"),
        QByteArrayLiteral("edge-extension://"),
    };

    for (const QByteArray &scheme : kSchemes) {
        if (!origin.startsWith(scheme)) {
            continue;
        }

        // corsAllowOrigin() echoes this exact value back, so the checks below
        // are what bound the bytes that can reach a response header.
        const char *const idBegin = origin.constData() + scheme.size();
        const char *const idEnd   = origin.constData() + origin.size();
        if (idBegin == idEnd || idEnd - idBegin > kMaxExtensionIdLength) {
            return false;
        }
        return std::all_of(idBegin, idEnd, isExtensionIdChar);
    }
    return false;
}

bool isFromBrowser(const RequestContext &request)
{
    return !request.secFetchSite.isEmpty() || !request.origin.isEmpty();
}

bool isContextAllowed(const RequestContext &request)
{
    // Direct navigation — address bar, bookmark, link click. Refusing it keeps
    // the JSON out of the user's browser history.
    if (request.secFetchMode == "navigate") {
        return false;
    }

    if (request.origin.isEmpty()) {
        // No Origin — non-browser client (curl, tests). Sec-Fetch-Mode
        // already filtered above.
        return true;
    }

    // Origin present — must be a known extension scheme. A page Origin
    // (https://evil.com, etc.) falls through to false, which is what turns
    // away the whole class of cross-site JS callers.
    return isBrowserExtensionOrigin(request.origin);
}

bool isClientAllowed(const RequestContext &request, const QStringList &allowedIds)
{
    if (request.clientId.isEmpty()) {
        // Allow only non-browser clients to omit the identifier. This branch
        // is what stops a sibling browser extension that simply forgot (or
        // refused) to set X-Sysinfo-Client from sneaking in via the
        // curl-friendly fallback, and what turns away a stale SysInfo
        // extension build that predates the header.
        return !isFromBrowser(request);
    }

    return allowedIds.contains(QString::fromUtf8(request.clientId));
}

bool isRequestAllowed(const RequestContext &request, const QStringList &allowedIds)
{
    return isContextAllowed(request) && isClientAllowed(request, allowedIds);
}

QByteArray corsAllowOrigin(const RequestContext &request)
{
    // Reflecting the exact origin is the modern replacement for a bare "*": it
    // works with browsers that distinguish credentialed from uncredentialed
    // requests. The wildcard is harmless on the other branch — a caller with no
    // Origin has no page side reading the response.
    return isBrowserExtensionOrigin(request.origin) ? request.origin
                                                    : QByteArrayLiteral("*");
}

} // namespace integration
