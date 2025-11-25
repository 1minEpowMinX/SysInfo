#include "utils.h"

#include <QDateTime>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTextStream>

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

QString getActiveIPAddress() {
    QString vpnIp, lanIp;

    const auto &interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface &iface : interfaces) {
        // Skip the interface if its unavailable or loopback
        if (!iface.flags().testFlag(QNetworkInterface::IsUp) ||
            (iface.flags().testFlag(QNetworkInterface::IsLoopBack)))
            continue;

        const auto &entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            QString ip = entry.ip().toString();
            if (ip.contains(":")) continue; // Skip IPv6

            if (iface.humanReadableName().contains("VPN", Qt::CaseInsensitive))
                vpnIp = ip;
            else if (lanIp.isEmpty())
                lanIp = ip;
        }
    }

    return !vpnIp.isEmpty() ? vpnIp : (!lanIp.isEmpty() ? lanIp : QObject::tr("Нет IP"));
}

QString getLastBootTime() {
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
    return QObject::tr("Unavailable");

#elif defined(Q_OS_MAC)
    // macOS does not have sysinfo, so we use sysctl
    struct timeval boottime;
    size_t len = sizeof(boottime); // Buffer size
    int mib[2] = {CTL_KERN, KERN_BOOTTIME};
    if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0) {
        QDateTime bootTime = QDateTime::fromSecsSinceEpoch(boottime.tv_sec);
        return bootTime.toString("dd.MM.yyyy HH:mm");
    }
    return QObject::tr("Unavailable");

#else
    return QObject::tr("Not supported");
#endif
}

SystemInfo collectSystemInfo()
{
    SystemInfo s;
    s.hostname = Utils::getHostname();
    s.username = Utils::getUsername();
    s.ip = Utils::getActiveIPAddress();
    s.uptime = Utils::getLastBootTime();
    return s;
}

QJsonObject toJson(const SystemInfo &s, bool includeLabels = false)
{
    QJsonObject obj;
    obj["hostname"] = s.hostname;
    obj["username"] = s.username;
    obj["ip"] = s.ip;
    obj["uptime"] = s.uptime;

    // Optionally add labels for localization
    if (includeLabels) {
        QJsonObject labels;
        labels["hostname"] = QObject::tr("Device name");
        labels["username"] = QObject::tr("User");
        labels["ip"]       = QObject::tr("IP address");
        labels["uptime"]   = QObject::tr("Uptime");
        obj["labels"] = labels;
    }

    return obj;
}


QString toText(const SystemInfo &s)
{
    return QObject::tr("Device name: %1\nUser: %2\nIP address: %3\nUptime: %4")
        .arg(s.hostname, s.username, s.ip, s.uptime);
}
}
