#include "utils.h"

#include <QObject>
#include <QString>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QDateTime>
#include <windows.h>


namespace Utils {

QString GetHostname() {
    return QHostInfo::localHostName();
}

QString GetUsername() {
    return qEnvironmentVariable("USERNAME");
}

QString GetActiveIPAddress() {
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

QString GetLastBootTime() {
    ULONGLONG uptimeMs = GetTickCount64();
    QDateTime bootTime = QDateTime::currentDateTime().addMSecs(-qint64(uptimeMs)); // curr time - boot time
    return bootTime.toString("dd.MM.yyyy HH:mm");
}

QString GetSystemInfo()
{
    QString hostname = Utils::GetHostname();
    QString username = Utils::GetUsername();
    QString ip = Utils::GetActiveIPAddress();
    QString uptime = Utils::GetLastBootTime();

    return QObject::tr("Имя устройства: %1\nПользователь: %2\nIP-адрес: %3\nВремя включения: %4")
                     .arg(hostname, username, ip, uptime);
}
}
