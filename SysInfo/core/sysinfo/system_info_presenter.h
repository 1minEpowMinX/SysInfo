#ifndef SYSTEM_INFO_PRESENTER_H
#define SYSTEM_INFO_PRESENTER_H

#include "system_info.h"

#include <QJsonObject>
#include <QString>

/**
 * @brief Presentation helpers for Utils::SystemInfo.
 *
 * Lives in a sub-namespace to keep i18n (QObject::tr) out of the data layer:
 * Utils itself only collects raw values, while Utils::Presenter renders them
 * for human or API consumption. Thanks to this split, tests of the data
 * layer (tst_system_info) can verify field shapes without spinning up a
 * QTranslator.
 */
namespace Utils::Presenter {

/// Localised plain-text rendering, used for tray tooltip and clipboard.
QString toText(const SystemInfo &s);

/// Stable JSON shape — keys match the public /systeminfo API contract.
QJsonObject toJson(const SystemInfo &s);

/// JSON with a parallel "labels" object containing localised field names,
/// served to the browser extension so it can render its UI without
/// hard-coding translations.
QJsonObject toJsonWithLabels(const SystemInfo &s);

} // namespace Utils::Presenter

#endif // SYSTEM_INFO_PRESENTER_H
