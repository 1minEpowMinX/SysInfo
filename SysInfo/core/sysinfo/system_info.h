#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QDateTime>
#include <QString>

/**
 * @brief Collects system information as pure data.
 *
 * The sysinfo namespace exposes only raw collection of OS-level facts. It
 * does not localise, format or serialise — that responsibility belongs to
 * sysinfo::presenter (see system_info_presenter.h). This split keeps the
 * data layer testable without bringing up a QTranslator.
 */
namespace sysinfo {

/**
 * @brief Holds the four facts SysInfo reports.
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
 * @brief Picks the best-guess "active" IPv4 address of this machine.
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
 * @brief Reports the build/revision of the running OS, below the granularity of a version.
 *
 * Complements QSysInfo::kernelVersion(), which stops short of the patch
 * level on some platforms — notably Windows, where it reports "10.0.26200"
 * and omits the update revision that changes with every cumulative update.
 *
 * Per platform:
 *   - Windows: "<CurrentBuildNumber>.<UBR>" ("26200.1234") — the pair winver
 *     shows, read from the CurrentVersion registry key;
 *   - macOS:   the kern.osversion build identifier ("23F79");
 *   - Linux:   the kernel build tag from /proc/sys/kernel/version
 *     ("45-Ubuntu"). Note the kernel *release* already carries a revision
 *     there and is reported separately as the kernel version.
 *
 * @return The build string, or empty when unobtainable or unsupported.
 */
QString osBuild();

/**
 * @brief Returns the last boot time as a QDateTime, in local time.
 *
 * The single place that performs the platform query (GetTickCount64 on
 * Windows, sysinfo() on Linux, sysctl(KERN_BOOTTIME) on macOS);
 * lastBootTime() and bootTimeSecs() are thin renderings of this value and
 * add no OS-specific logic of their own.
 *
 * @return A valid QDateTime on success, or an invalid one when the syscall
 *         fails or the platform is unsupported.
 */
QDateTime bootTime();

/**
 * @brief Formats the last boot time as a display string.
 *
 * @return "dd.MM.yyyy HH:mm" on success, or an empty QString when bootTime()
 *         is invalid. The presenter substitutes a localised fallback
 *         ("Unavailable") on empty.
 */
QString lastBootTime();

/**
 * @brief Returns the last boot time as seconds since the Unix epoch.
 *
 * The machine-readable counterpart of lastBootTime(), meant for the
 * diagnostic log event, emitted as a JSON number rather than a formatted
 * string. Because the value is in seconds, the Elasticsearch date field it
 * feeds must be mapped with "format": "epoch_second" — the default
 * epoch_millis would read the smaller number as a 1970 timestamp.
 *
 * @return Epoch seconds on success, or 0 when bootTime() is invalid.
 */
qint64 bootTimeSecs();

/**
 * @brief Collects a full Info snapshot.
 *
 * Convenience wrapper over the four getters above. Each call queries the
 * OS afresh — no caching at this layer.
 *
 * Hardware facts are a separate concern and live in hardware_info.h.
 *
 * @return Session snapshot; individual fields are empty when unobtainable.
 */
Info collect();

} // namespace sysinfo

#endif // SYSTEM_INFO_H
