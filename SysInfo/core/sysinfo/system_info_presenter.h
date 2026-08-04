#ifndef SYSTEM_INFO_PRESENTER_H
#define SYSTEM_INFO_PRESENTER_H

#include "about_facts.h"
#include "system_info.h"

#include <QJsonObject>
#include <QString>

/**
 * @brief Formats sysinfo::Info for display.
 *
 * Lives in a sub-namespace to keep i18n (QObject::tr) out of the data layer:
 * sysinfo itself only collects raw values, while sysinfo::presenter renders
 * them for human or API consumption.
 *
 * Each rendering here serves one consumer — the tray tooltip, the HTTP
 * endpoint, the About window — and they sit together because what varies
 * between them is the shape, not the data. Moving one out to sit beside its
 * consumer would put it past the reach of the test suite, ui/ being compiled
 * into the executable alone.
 */
namespace sysinfo::presenter {

/// Renders localised plain text for the tray tooltip and clipboard. Empty
/// fields are replaced with a localised placeholder — the only rendering
/// that does so.
QString toText(const Info &s);

/**
 * @brief Builds the stable JSON shape of the public /systeminfo API contract.
 *
 * Every key of Info is present whatever was obtainable, so the shape of the
 * document does not vary with the machine. Values are carried verbatim,
 * including the empty string that marks an unobtainable field: the placeholder
 * that stands in for it is a display decision belonging to the caller, and
 * baking a translated one in here would make a data field vary with SysInfo's
 * UI language. The browser extension supplies its own through
 * browser.i18n / chrome.i18n.
 *
 * @param s Session snapshot.
 * @return Object with the keys hostname, username, ip and uptime.
 */
QJsonObject toJson(const Info &s);

/**
 * @brief Builds toJson() plus a parallel "labels" object of localised field names.
 *
 * Serves non-browser callers (curl, support tooling, manual inspection), which
 * have no client-side i18n stack to name the fields with. Localisation reaches
 * the labels alone; the data fields carry the same verbatim values toJson()
 * emits.
 *
 * @param s Session snapshot.
 * @return The toJson() object with an added "labels" member.
 */
QJsonObject toJsonWithLabels(const Info &s);

/**
 * @brief Collects the machine-describing values the About window shows.
 *
 * Reads the host name and the user name off @p s and asks QSysInfo for the OS
 * product name; the uptime and the IP address are not part of this block.
 * Carries every value through verbatim, empty ones included — the About window
 * is the only place that decides how to spell a missing value, and no
 * placeholder belongs in a field a support engineer reads literally.
 *
 * @param s                Session snapshot; only hostname and username are read.
 * @param settingsFilePath Where QSettings keeps this user's settings, as
 *                         SettingsManager::filePath() reports it. A registry
 *                         key on Windows, a file elsewhere.
 * @return The four values, unformatted.
 */
AboutFacts toAboutFacts(const Info &s, const QString &settingsFilePath);

} // namespace sysinfo::presenter

#endif // SYSTEM_INFO_PRESENTER_H
