#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QDateTime>
#include <QString>

/**
 * @brief Collects system information as pure data.
 *
 * The sysinfo namespace exposes only raw collection of OS-level facts. It
 * does not localise, format or serialise — that responsibility belongs to
 * sysinfo::presenter (see system_info_presenter.h).
 */
namespace sysinfo {

/**
 * @brief Holds the four facts SysInfo reports.
 *
 * Plain value type with no invariants between fields — any combination of
 * empty/non-empty strings is valid.
 *
 * Empty fields signal "no value" rather than carrying a localised placeholder.
 *
 * These names are internal. What the /systeminfo endpoint puts on the wire is
 * fixed separately, in sysinfo::presenter::toJson(), which is where the public
 * key of each field lives.
 */
struct Info {
    QString hostname;      ///< Network host name; empty if unobtainable.
    QString username;      ///< Login name; empty if neither USERNAME nor USER is set.
    QString ip;            ///< Best-guess active IPv4 address; empty if none found.
    QString lastBootTime;  ///< As lastBootTime() renders it; empty if unsupported
                           ///< or the syscall failed.
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
 * Tailscale, OpenVPN…) over a plain LAN address.
 *
 * @return The IPv4 string, or empty QString if nothing usable was found.
 *         The presenter substitutes a localised fallback ("No IP") on empty.
 */
QString activeIpAddress();

/**
 * @brief Reports the build/revision of the running OS, below the granularity of a version.
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
 * The single place that performs the platform query; lastBootTime() and
 * bootTimeSecs() are thin renderings of this value and add no OS-specific
 * logic of their own.
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
 * The machine-readable counterpart of lastBootTime(); the DeviceInventory
 * payload carries this value as a JSON number.
 *
 * @return Epoch seconds on success, or 0 when bootTime() is invalid.
 */
qint64 bootTimeSecs();

/**
 * @brief Collects a full Info snapshot.
 *
 * Convenience wrapper over hostname(), username(), activeIpAddress() and
 * lastBootTime(). Each call queries the OS afresh — no caching at this layer.
 * osBuild() and bootTimeSecs() serve the diagnostic log event and are not
 * part of the snapshot.
 *
 * Hardware facts are a separate concern and live in hardware_info.h.
 *
 * @return Session snapshot; individual fields are empty when unobtainable.
 */
Info collect();

} // namespace sysinfo

#endif // SYSTEM_INFO_H
