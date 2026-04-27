#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QString>

namespace Utils {

struct SystemInfo {
    QString hostname;
    QString username;
    QString ip;
    QString uptime;
};

QString getHostname();
QString getUsername();
QString getActiveIPAddress();
QString getLastBootTime();

SystemInfo collectSystemInfo();

} // namespace Utils

#endif // SYSTEM_INFO_H
