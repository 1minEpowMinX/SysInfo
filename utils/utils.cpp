#include "utils.h"

#include <QObject>
#include <QString>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDateTime>
#include <windows.h>


namespace Utils {

QString getHostname() {
    return QHostInfo::localHostName();
}

QString getUsername() {
    return qEnvironmentVariable("USERNAME");
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
    ULONGLONG uptimeMs = GetTickCount64();
    QDateTime bootTime = QDateTime::currentDateTime().addMSecs(-qint64(uptimeMs)); // curr time - boot time
    return bootTime.toString("dd.MM.yyyy HH:mm");
}

QString getSystemInfo()
{
    QString hostname = Utils::getHostname();
    QString username = Utils::getUsername();
    QString ip = Utils::getActiveIPAddress();
    QString uptime = Utils::getLastBootTime();

    return QObject::tr("Имя устройства: %1\nПользователь: %2\nIP-адрес: %3\nВремя включения: %4")
                     .arg(hostname, username, ip, uptime);
}
}
