#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "onboarding_flags.h"

#include <QSettings>

/**
 * @brief Stores SysInfo's persistent user preferences.
 *
 * Thin wrapper over QSettings ("Pivdenny", "SysInfo") that exposes only the
 * values actually used by the application — no string keys leak into the
 * rest of the codebase. Covers the two one-shot onboarding flags (welcome
 * message and Windows tray-guide hint), the administrator override of the
 * IntegrationServer whitelist, and the path of the store itself.
 *
 * Not a singleton: instantiate once in main() and inject by reference into
 * App / TrayGuide. Copy and move are deleted because QSettings holds OS
 * resources that should not be duplicated.
 */
class SettingsManager : public OnboardingFlags
{
public:
    /// Opens the store for organisation "Pivdenny", application "SysInfo".
    SettingsManager();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    SettingsManager(SettingsManager&&) = delete;
    SettingsManager& operator=(SettingsManager&&) = delete;

    /// @return true if the welcome tray notification should still be shown.
    bool showWelcome() const override;
    /// Persists the welcome flag. Logs SettingsWriteFailed on QSettings I/O errors.
    void setShowWelcome(bool value) override;

    /// @return true if the Windows tray-guide hint should still be shown.
    bool showTrayGuide() const override;
    /// Persists the tray-guide flag. Logs SettingsWriteFailed on I/O errors.
    void setShowTrayGuide(bool value) override;

    /// @return Absolute path of the on-disk settings file. Useful for the
    ///         About dialog, support tickets and manual cleanup. Provides
    ///         a single point of access — callers must NOT instantiate
    ///         their own QSettings("Pivdenny", "SysInfo") to read this.
    QString filePath() const;

    /**
     * @brief Reads the administrator override of the IntegrationServer client whitelist.
     *
     * Read fresh on each call, so an edit to the registry/ini file takes effect
     * without restarting SysInfo.
     *
     * @return Browser-extension IDs under the key Integration/AllowedExtensionIds.
     *         May be empty, in which case IntegrationServer falls back to its
     *         compiled-in defaults.
     */
    QStringList allowedExtensionIds() const;

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
