#ifndef SYSTEM_INFO_PRESENTER_H
#define SYSTEM_INFO_PRESENTER_H

#include "system_info.h"

#include <QJsonObject>
#include <QString>

/**
 * @brief Formats sysinfo::Info for display.
 *
 * Lives in a sub-namespace to keep i18n (QObject::tr) out of the data layer:
 * sysinfo itself only collects raw values, while sysinfo::presenter renders
 * them for human or API consumption.
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
 * @brief Renders the localised "system details" block of the About dialog.
 *
 * Reads the host name and the user name off @p s and asks QSysInfo for the OS
 * product name; the uptime and the IP address are not part of this block.
 *
 * @param s                Session snapshot; only hostname and username are read.
 * @param settingsFilePath Absolute path of the on-disk settings file.
 * @return A rich-text paragraph naming the OS, the user, the device and the
 *         settings file.
 */
QString toSystemDetailsHtml(const Info &s, const QString &settingsFilePath);

} // namespace sysinfo::presenter

#endif // SYSTEM_INFO_PRESENTER_H
