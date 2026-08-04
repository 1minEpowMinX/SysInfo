#include "system_info_presenter.h"

#include <QObject>
#include <QSysInfo>

namespace sysinfo::presenter {

namespace {

/**
 * @brief Substitutes a localised placeholder for an empty raw field.
 *
 * The data layer returns empty QStrings for "no value" cases (no usable
 * IPv4, unsupported platform, syscall failure). The presenter is the only
 * place that knows how to spell that for a human.
 */
QString withFallback(const QString &raw, const QString &fallback)
{
    return raw.isEmpty() ? fallback : raw;
}

QString ipOrFallback(const Info &s)     { return withFallback(s.ip,     QObject::tr("No IP")); }
QString uptimeOrFallback(const Info &s) { return withFallback(s.uptime, QObject::tr("Unavailable")); }

} // namespace

QString toText(const Info &s)
{
    return QObject::tr("Device name: %1\nUser: %2\nIP address: %3\nUptime: %4")
        .arg(s.hostname,
             s.username,
             ipOrFallback(s),
             uptimeOrFallback(s));
}

QJsonObject toJson(const Info &s)
{
    QJsonObject obj;
    obj["hostname"] = s.hostname;
    obj["username"] = s.username;
    obj["ip"]       = s.ip;
    obj["uptime"]   = s.uptime;
    return obj;
}

QJsonObject toJsonWithLabels(const Info &s)
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

AboutFacts toAboutFacts(const Info &s, const QString &settingsFilePath)
{
    return AboutFacts{QSysInfo::prettyProductName(),
                      s.username,
                      s.hostname,
                      settingsFilePath};
}

} // namespace sysinfo::presenter
