#ifndef APP_H
#define APP_H

#include <QObject>
#include <QString>

class DialogPresenter;
class IntegrationServer;
class OnboardingFlags;
class TrayView;
class UserPrompt;
class WelcomeNotifier;

namespace sysinfo { class InfoSource; }

/**
 * @brief Coordinates the application's runtime parts.
 *
 * Drives the three things SysInfo does once it is up:
 *   - keeps the tray tooltip carrying a fresh SystemInfo snapshot,
 *   - answers the tray menu (copy, about, exit),
 *   - schedules the onboarding notifications and opens what they ask for.
 *
 * Creates and owns one collaborator, WelcomeNotifier, whose sequencing is its
 * own concern. Everything else is injected and outlives it — in practice all
 * of it lives on the stack of main(), with App declared last so it is
 * destroyed first.
 *
 * Of the settings store it takes the one port whose flags it clears, plus the
 * file path as a value.
 *
 * Every collaborator is reached through a port, so this class names no widget
 * type and blocks on no dialog of its own; which of them are modal is decided
 * behind DialogPresenter.
 */
class App : public QObject
{
    Q_OBJECT

public:
    /**
     * @param flags   One-shot onboarding flags: passed on to the notifier, and
     *                cleared here when the guide retires itself. Not owned;
     *                must outlive App.
     * @param server  Integration endpoint, started by start(). Not owned; must
     *                outlive App.
     * @param prompt  Channel for errors and confirmations. Not owned; must
     *                outlive App.
     * @param tray    Tray icon and menu. Not owned; must outlive App.
     * @param dialogs Channel for the application's windows. Not owned; must
     *                outlive App.
     * @param info    Session snapshot, shared with @p server. Not owned; must
     *                outlive App.
     * @param settingsFilePath Absolute path of the settings file, named in the
     *                About dialog.
     * @param parent  Standard Qt parent.
     */
    explicit App(OnboardingFlags& flags,
                 IntegrationServer& server,
                 UserPrompt& prompt,
                 TrayView& tray,
                 DialogPresenter& dialogs,
                 sysinfo::InfoSource& info,
                 QString settingsFilePath,
                 QObject* parent = nullptr);

    /**
     * @brief Brings the application up: builds the tray UI, schedules
     *        onboarding, starts the integration server.
     * @return false if the system tray is unavailable on this session — a
     *         fatal configuration error; the caller (main) should exit.
     *         A failure of the integration server is non-fatal: the user is
     *         warned through the UserPrompt and the app keeps running.
     */
    [[nodiscard]] bool start();

private slots:
    /// Refreshes cached SystemInfo and copies it to the system clipboard.
    void onCopyRequested();
    /// Opens the "About SysInfo" window with a fresh snapshot rendered into it.
    void onAboutRequested();
    /// Confirms with the user, logs AppExit, and quits the QApplication.
    void onQuitRequested();
    /// Opens the tray-pinning guide.
    void onTrayGuideRequested();

private:
    /// Starts the timer that re-collects the snapshot, replacing m_cachedInfo
    /// and the tray tooltip only when the rendered text differs from the
    /// cached one. Runs at kTrayUpdateIntervalMs, defined in the .cpp.
    void startTrayUpdateTimer();

    OnboardingFlags&     m_flags;    ///< Injected, not owned.
    IntegrationServer&   m_server;   ///< Injected, not owned.
    UserPrompt&          m_prompt;   ///< Injected, not owned.
    TrayView&            m_tray;     ///< Injected, not owned.
    DialogPresenter&     m_dialogs;  ///< Injected, not owned.
    sysinfo::InfoSource& m_info;     ///< Injected, not owned.

    QString          m_settingsFilePath;   ///< Named in the About dialog.
    WelcomeNotifier* m_notifier = nullptr; ///< Parent-owned via QObject(this).
    QString          m_cachedInfo;         ///< Last rendered tray tooltip text.
};

#endif // APP_H
