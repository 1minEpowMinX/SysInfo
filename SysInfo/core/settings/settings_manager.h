#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>

/**
 * @brief Persistent user preferences for SysInfo.
 *
 * Thin wrapper over QSettings ("Pivdenny", "SysInfo") that exposes only the
 * flags actually used by the application — no string keys leak into the
 * rest of the codebase. Currently tracks two one-shot onboarding flags
 * (welcome message and Windows tray-guide hint).
 *
 * Not a singleton: instantiate once in main() and inject by reference into
 * App / TrayGuide. Copy and move are deleted because QSettings holds OS
 * resources that should not be duplicated.
 */
class SettingsManager
{
public:
    SettingsManager();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    SettingsManager(SettingsManager&&) = delete;
    SettingsManager& operator=(SettingsManager&&) = delete;

    /// @return true if the welcome tray notification should still be shown.
    bool showWelcome() const;
    /// Persist the welcome flag. Logs SettingsWriteFailed on QSettings I/O errors.
    void setShowWelcome(bool value);

    /// @return true if the Windows tray-guide hint should still be shown.
    bool showTrayGuide() const;
    /// Persist the tray-guide flag. Logs SettingsWriteFailed on I/O errors.
    void setShowTrayGuide(bool value);

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
