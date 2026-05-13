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

    /// @return Absolute path of the on-disk settings file. Useful for the
    ///         About dialog, support tickets and manual cleanup. Provides
    ///         a single point of access — callers must NOT instantiate
    ///         their own QSettings("Pivdenny", "SysInfo") to read this.
    QString filePath() const;

    /**
     * @return Browser-extension IDs administrators have whitelisted for
     *         the IntegrationServer (key Integration/AllowedExtensionIds).
     *         May be empty — IntegrationServer falls back to its compiled-in
     *         defaults in that case.
     *
     * Read fresh on each call so an admin can edit the registry/ini file
     * without restarting SysInfo.
     */
    QStringList allowedExtensionIds() const;

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
