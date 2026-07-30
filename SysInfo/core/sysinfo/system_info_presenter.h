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

/// Renders localised plain text for the tray tooltip and clipboard.
QString toText(const Info &s);

/// Builds the stable JSON shape — keys match the public /systeminfo API contract.
/// Used for the browser extension: it receives raw data and localises
/// field labels itself through browser.i18n / chrome.i18n.
QJsonObject toJson(const Info &s);

/// Builds JSON with a parallel "labels" object containing localised field
/// names. Used for non-browser callers (curl, support tooling, manual
/// inspection) where there is no client-side i18n stack and the labels
/// are useful for human reading.
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
