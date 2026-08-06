#ifndef SYSTEM_INFO_PRESENTER_H
#define SYSTEM_INFO_PRESENTER_H

#include "about_facts.h"
#include "system_info.h"

#include <QJsonObject>
#include <QString>

/**
 * @brief Formats sysinfo::Info for display.
 *
 * A sub-namespace of sysinfo: sysinfo collects raw values, sysinfo::presenter
 * renders them for human or API consumption, and QObject::tr appears in the
 * latter alone.
 *
 * Each rendering here serves one consumer — the tray tooltip, the HTTP
 * endpoint, the About window — and what varies between them is the shape, not
 * the data.
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
 * including the empty string that marks an unobtainable field; the placeholder
 * that stands in for it is the caller's to choose.
 *
 * @param s Session snapshot.
 * @return Object with the keys hostname, username, ip and uptime.
 */
QJsonObject toJson(const Info &s);

/**
 * @brief Builds toJson() plus a parallel "labels" object of localised field names.
 *
 * Serves non-browser callers (curl, support tooling, manual inspection).
 * Localisation reaches the labels alone; the data fields carry the same
 * verbatim values toJson() emits.
 *
 * @param s Session snapshot.
 * @return The toJson() object with an added "labels" member.
 */
QJsonObject toJsonWithLabels(const Info &s);

/**
 * @brief Collects the machine-describing values the About window shows.
 *
 * Reads the host name and the user name off @p s and asks QSysInfo for the OS
 * product name; the boot time and the IP address are not part of this block.
 * Carries every value through verbatim, empty ones included.
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
