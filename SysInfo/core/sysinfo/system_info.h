#ifndef UTILS_H
#define UTILS_H

#include <QJsonObject>
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
QJsonObject toJson(const SystemInfo &s, bool includeLabels);
QString toText(const SystemInfo &s);
}

#endif // UTILS_H
