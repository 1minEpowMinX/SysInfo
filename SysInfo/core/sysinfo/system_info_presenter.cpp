#include "system_info_presenter.h"

#include <QObject>
#include <QSysInfo>

namespace sysinfo::presenter {

// The renderings sit here rather than beside the consumers that ask for them:
// ui/ is compiled into the executable alone, out of the test suite's reach.

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

QString ipOrFallback(const Info &s) { return withFallback(s.ip, QObject::tr("No IP")); }

QString bootTimeOrFallback(const Info &s)
{
    return withFallback(s.lastBootTime, QObject::tr("Unavailable"));
}

} // namespace

QString toText(const Info &s)
{
    return QObject::tr("Device name: %1\nUser: %2\nIP address: %3\nUptime: %4")
        .arg(s.hostname,
             s.username,
             ipOrFallback(s),
             bootTimeOrFallback(s));
}

QJsonObject toJson(const Info &s)
{
    // These keys are the published API — renaming one breaks every consumer
    // reading it. "uptime" carries the boot time and has done so since the
    // first release; the field behind it says what it holds, and the two are
    // reconciled here rather than by renaming the key.
    //
    // No placeholder stands in for an empty value: a translated one would make
    // a data field vary with SysInfo's UI language. The browser extension
    // supplies its own through browser.i18n / chrome.i18n.
    QJsonObject obj;
    obj["hostname"] = s.hostname;
    obj["username"] = s.username;
    obj["ip"]       = s.ip;
    obj["uptime"]   = s.lastBootTime;
    return obj;
}

QJsonObject toJsonWithLabels(const Info &s)
{
    QJsonObject obj = toJson(s);

    // For callers with no client-side i18n stack to name the fields with.
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
    // No withFallback() here: a placeholder has no business in a field a
    // support engineer reads literally, and the About window is the one place
    // that decides how to spell a missing value.
    return AboutFacts{QSysInfo::prettyProductName(),
                      s.username,
                      s.hostname,
                      settingsFilePath};
}

} // namespace sysinfo::presenter
