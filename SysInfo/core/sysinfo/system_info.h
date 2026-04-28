#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QString>

/**
 * @brief Pure data layer for system information.
 *
 * The sysinfo namespace exposes only raw collection of OS-level facts. It
 * does not localise, format or serialise — that responsibility belongs to
 * sysinfo::presenter (see system_info_presenter.h). This split keeps the
 * data layer testable without bringing up a QTranslator.
 */
namespace sysinfo {

/**
 * @brief Snapshot of the four facts SysInfo reports.
 *
 * Plain value type with no invariants between fields — any combination of
 * empty/non-empty strings is valid. The field names form the public JSON
 * contract of the /systeminfo HTTP endpoint, so do not rename without
 * also updating the API consumers (browser extension).
 *
 * Empty fields signal "no value" rather than carrying a localised
 * placeholder — formatting is the presenter's job, not the model's.
 */
struct Info {
    QString hostname;   ///< Network host name; empty if unobtainable.
    QString username;   ///< Login name; empty if neither USERNAME nor USER is set.
    QString ip;         ///< Best-guess active IPv4 address; empty if none found.
    QString uptime;     ///< "dd.MM.yyyy HH:mm"; empty if unsupported or syscall failed.
};

/// @return The OS-reported host name (QHostInfo::localHostName), or empty.
QString hostname();

/// @return Login name from USERNAME (Windows) or USER (Unix); empty if unset.
QString username();

/**
 * @brief Pick the best-guess "active" IPv4 address of this machine.
 *
 * Walks all network interfaces, skips loopback/down ones and known virtual
 * bridges (Docker, VMware, Hyper-V, etc.). Prefers a VPN address (WireGuard,
 * Tailscale, OpenVPN…) over a plain LAN address — this is what the support
 * team actually wants to see on a developer's machine.
 *
 * @return The IPv4 string, or empty QString if nothing usable was found.
 *         The presenter substitutes a localised fallback ("No IP") on empty.
 */
QString activeIpAddress();

/**
 * @brief Last boot time as a display string.
 *
 * Implementation differs per OS (GetTickCount64 on Windows, sysinfo() on
 * Linux, sysctl(KERN_BOOTTIME) on macOS).
 *
 * @return "dd.MM.yyyy HH:mm" on success, or an empty QString when the
 *         syscall fails or the platform is unsupported. The presenter
 *         substitutes a localised fallback ("Unavailable") on empty.
 */
QString lastBootTime();

/**
 * @brief Collect a full Info snapshot.
 *
 * Convenience wrapper over the four getters above. Each call queries the
 * OS afresh — no caching at this layer.
 */
Info collect();

} // namespace sysinfo

#endif // SYSTEM_INFO_H
