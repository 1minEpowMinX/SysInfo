#ifndef SYSTEM_INFO_PRESENTER_H
#define SYSTEM_INFO_PRESENTER_H

#include "system_info.h"

#include <QJsonObject>
#include <QString>

/**
 * @brief Presentation helpers for sysinfo::Info.
 *
 * Lives in a sub-namespace to keep i18n (QObject::tr) out of the data layer:
 * sysinfo itself only collects raw values, while sysinfo::presenter renders
 * them for human or API consumption. Thanks to this split, tests of the
 * data layer (tst_system_info) can verify field shapes without spinning up
 * a QTranslator.
 */
namespace sysinfo::presenter {

/// Localised plain-text rendering, used for tray tooltip and clipboard.
QString toText(const Info &s);

/// Stable JSON shape — keys match the public /systeminfo API contract.
/// Used for the browser extension: it receives raw data and localises
/// field labels itself through browser.i18n / chrome.i18n.
QJsonObject toJson(const Info &s);

/// JSON with a parallel "labels" object containing localised field
/// names. Used for non-browser callers (curl, support tooling, manual
/// inspection) where there is no client-side i18n stack and the labels
/// are useful for human reading.
QJsonObject toJsonWithLabels(const Info &s);

} // namespace sysinfo::presenter

#endif // SYSTEM_INFO_PRESENTER_H
