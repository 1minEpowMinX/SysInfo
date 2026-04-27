#include "system_info_presenter.h"

#include <QObject>

namespace Utils::Presenter {

QString toText(const SystemInfo &s)
{
    return QObject::tr("Device name: %1\nUser: %2\nIP address: %3\nUptime: %4")
        .arg(s.hostname, s.username, s.ip, s.uptime);
}

QJsonObject toJson(const SystemInfo &s)
{
    QJsonObject obj;
    obj["hostname"] = s.hostname;
    obj["username"] = s.username;
    obj["ip"]       = s.ip;
    obj["uptime"]   = s.uptime;
    return obj;
}

QJsonObject toJsonWithLabels(const SystemInfo &s)
{
    QJsonObject obj = toJson(s);

    QJsonObject labels;
    labels["hostname"] = QObject::tr("Device name");
    labels["username"] = QObject::tr("User");
    labels["ip"]       = QObject::tr("IP address");
    labels["uptime"]   = QObject::tr("Uptime");
    obj["labels"] = labels;

    return obj;
}

} // namespace Utils::Presenter
