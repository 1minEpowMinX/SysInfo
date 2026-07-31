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
 * Serves two ports, and is the only implementation of either: OnboardingFlags,
 * through which WelcomeNotifier reads both one-shot flags and clears the
 * welcome one, and ExtensionWhitelist, through which IntegrationServer reads
 * the administrator override. Those consumers hold the port they need rather
 * than this class, which is what lets a test substitute the flags or the
 * whitelist without a QSettings behind them. The constructor, filePath() and
 * setShowTrayGuide() sit outside both ports and are reachable only through the
 * concrete type.
 *
 * Not a singleton: instantiate once in main() and inject by reference into
 * App, which passes the ports on to the objects it builds. Copy and move are
 * deleted because QSettings holds OS resources that should not be duplicated.
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
    /// Outside OnboardingFlags: App retires this hint, not the notifier.
    void setShowTrayGuide(bool value);

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
     *         May be empty, in which case the access policy falls back to its
     *         compiled-in defaults.
     */
    QStringList allowedExtensionIds() const override;

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
