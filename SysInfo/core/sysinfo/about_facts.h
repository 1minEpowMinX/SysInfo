#ifndef ABOUT_FACTS_H
#define ABOUT_FACTS_H

#include <QString>

namespace sysinfo {

/**
 * @brief Holds the machine-describing values the About window puts on screen.
 *
 * Carries text only: the window decides the wording of the labels, the order
 * of the rows and the colours. Built by sysinfo::presenter::toAboutFacts(),
 * which is where the values come from.
 */
struct AboutFacts
{
    QString operatingSystem;   ///< Product name of the running OS.
    QString username;          ///< Account the process runs as.
    QString hostname;          ///< Name of the device.
    QString settingsFilePath;  ///< Where QSettings keeps this user's settings.
};

} // namespace sysinfo

#endif // ABOUT_FACTS_H
