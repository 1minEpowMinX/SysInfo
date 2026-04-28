#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QString>

/**
 * @brief Pure data layer for system information.
 *
 * The Utils namespace exposes only raw collection of OS-level facts. It does
 * not localise, format or serialise — that responsibility belongs to
 * Utils::Presenter (see system_info_presenter.h). This split keeps the data
 * layer testable without bringing up a QTranslator.
 */
namespace Utils {

/**
 * @brief Snapshot of the four facts SysInfo reports.
 *
 * Plain value type with no invariants between fields — any combination of
 * empty/non-empty strings is valid. The field names form the public JSON
 * contract of the /systeminfo HTTP endpoint, so do not rename without
 * also updating the API consumers (browser extension).
 */
struct SystemInfo {
    QString hostname;   ///< Network host name as reported by the OS.
    QString username;   ///< Login name of the current desktop session.
    QString ip;         ///< Best-guess active IPv4 address (see getActiveIPAddress).
    QString uptime;     ///< Last boot time formatted as "dd.MM.yyyy HH:mm".
};

/// @return The OS-reported host name (QHostInfo::localHostName).
QString getHostname();

/// @return The login name from the USERNAME (Windows) or USER (Unix) env var.
QString getUsername();

/**
 * @brief Pick the best-guess "active" IPv4 address of this machine.
 *
 * Walks all network interfaces, skips loopback/down ones and known virtual
 * bridges (Docker, VMware, Hyper-V, etc.). Prefers a VPN address (WireGuard,
 * Tailscale, OpenVPN…) over a plain LAN address — this is what the support
 * team actually wants to see on a developer's machine.
 *
 * @return The IPv4 string, or the localised "No IP" fallback if nothing matches.
 */
QString getActiveIPAddress();

/**
 * @brief Last boot time as a localised display string.
 *
 * Implementation differs per OS (GetTickCount64 on Windows, sysinfo() on
 * Linux, sysctl(KERN_BOOTTIME) on macOS). On unsupported platforms or when
 * the syscall fails, returns a localised fallback.
 *
 * @return "dd.MM.yyyy HH:mm", or one of "Unavailable" / "Not supported".
 */
QString getLastBootTime();

/**
 * @brief Collect a full SystemInfo snapshot.
 *
 * Convenience wrapper over the four getters above. Each call queries the
 * OS afresh — no caching at this layer.
 */
SystemInfo collectSystemInfo();

} // namespace Utils

#endif // SYSTEM_INFO_H
