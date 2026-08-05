#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "extension_whitelist.h"
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
 * Serves two ports, and is the only implementation of either: OnboardingFlags
 * for the one-shot notices, ExtensionWhitelist for the administrator override.
 * Every consumer holds the port it needs rather than this class. Outside the
 * two sit the constructor and filePath().
 *
 * Not a singleton: instantiate once in main() and hand it to each consumer as
 * the port that consumer takes. Copy and move are deleted.
 */
class SettingsManager : public OnboardingFlags, public ExtensionWhitelist
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

    /**
     * @brief Names the file the settings live in.
     *
     * Fixed for the life of the store, and the single point of access for it —
     * callers must NOT instantiate their own QSettings to work it out.
     *
     * @return Absolute path of the on-disk settings file.
     */
    QString filePath() const;

    /**
     * @brief Reads the administrator override of the IntegrationServer client whitelist.
     *
     * Read fresh on each call, so an edit to the registry/ini file takes effect
     * without restarting SysInfo.
     *
     * @return Browser-extension IDs under the key Integration/AllowedExtensionIds.
     *         May be empty, in which case the access policy falls back to its
     *         compiled-in defaults.
     */
    QStringList allowedExtensionIds() const override;

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
