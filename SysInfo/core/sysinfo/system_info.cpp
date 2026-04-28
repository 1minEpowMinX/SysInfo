#include "system_info.h"

#include <QDateTime>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#elif defined(Q_OS_MAC)
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/clock.h>
#include <mach/mach_host.h>
#endif

namespace Utils {

QString getHostname() {
    return QHostInfo::localHostName();
}

QString getUsername() {
#ifdef Q_OS_WIN
    return qEnvironmentVariable("USERNAME");
#else
    return qEnvironmentVariable("USER");
#endif
}

namespace {

bool looksLikeVirtualBridge(const QString &nameLower) {
    static const char *const kBridgeKeywords[] = {
        "docker", "vethernet", "vmware", "vmnet",
        "virtualbox", "vboxnet", "hyper-v", "bluetooth"
    };
    for (const char *kw : kBridgeKeywords) {
        if (nameLower.contains(QLatin1String(kw)))
            return true;
    }
    return false;
}

bool looksLikeVpn(const QNetworkInterface &iface) {
    if (iface.type() == QNetworkInterface::Virtual)
        return true;

    const QString nameLower = iface.humanReadableName().toLower();
    static const char *const kVpnKeywords[] = {
        "vpn", "wireguard", "tailscale", "openvpn",
        "anyconnect", "cisco", "zerotier", "tun", "tap"
    };
    for (const char *kw : kVpnKeywords) {
        if (nameLower.contains(QLatin1String(kw)))
            return true;
    }
    return false;
}

} // namespace

QString getActiveIPAddress() {
    QString vpnIp, lanIp;

    const auto &interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        const auto flags = iface.flags();
        // Skip interfaces that are down, loopback or not running
        if (!flags.testFlag(QNetworkInterface::IsUp) ||
            !flags.testFlag(QNetworkInterface::IsRunning) ||
            flags.testFlag(QNetworkInterface::IsLoopBack))
            continue;

        const QString nameLower = iface.humanReadableName().toLower();
        if (looksLikeVirtualBridge(nameLower))
            continue;

        const bool isVpn = looksLikeVpn(iface);

        const auto &entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            const QString ip = entry.ip().toString();
            if (ip.contains(QLatin1Char(':'))) continue; // Skip IPv6

            if (isVpn) {
                if (vpnIp.isEmpty()) vpnIp = ip;
            } else if (lanIp.isEmpty()) {
                lanIp = ip;
            }
        }
    }

    if (!vpnIp.isEmpty()) return vpnIp;
    if (!lanIp.isEmpty()) return lanIp;
    return {}; // empty = "no usable IPv4 found"; presenter localises the fallback
}

QString getLastBootTime() {
    // Returns the boot time formatted as "dd.MM.yyyy HH:mm" on supported
    // platforms, or an empty QString when the syscall fails or the platform
    // is not supported. The presenter is responsible for substituting a
    // localised fallback string ("Unavailable" / "Not supported") on empty.
#ifdef Q_OS_WIN
    ULONGLONG uptimeMs = GetTickCount64();
    QDateTime bootTime = QDateTime::currentDateTime().addMSecs(-qint64(uptimeMs));
    return bootTime.toString("dd.MM.yyyy HH:mm");

#elif defined(Q_OS_LINUX)
    struct sysinfo s_info;
    if (sysinfo(&s_info) == 0) {
        QDateTime bootTime = QDateTime::currentDateTime().addSecs(-s_info.uptime);
        return bootTime.toString("dd.MM.yyyy HH:mm");
    }
    return {};

#elif defined(Q_OS_MAC)
    // macOS does not have sysinfo, so we use sysctl
    struct timeval boottime;
    size_t len = sizeof(boottime); // Buffer size
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0) {
        QDateTime bootTime = QDateTime::fromSecsSinceEpoch(boottime.tv_sec);
        return bootTime.toString("dd.MM.yyyy HH:mm");
    }
    return {};

#else
    return {};
#endif
}

SystemInfo collectSystemInfo()
{
    SystemInfo s;
    s.hostname = Utils::getHostname();
    s.username = Utils::getUsername();
    s.ip       = Utils::getActiveIPAddress();
    s.uptime   = Utils::getLastBootTime();
    return s;
}

} // namespace Utils
