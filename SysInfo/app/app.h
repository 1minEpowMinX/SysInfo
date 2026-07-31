#ifndef APP_H
#define APP_H

#include <QObject>
#include <QString>

class DialogPresenter;
class IntegrationServer;
class SettingsManager;
class TrayView;
class UserPrompt;
class WelcomeNotifier;

namespace sysinfo { class InfoSource; }

/**
 * @brief Wires the application's runtime parts together and coordinates them.
 *
 * Drives the three things SysInfo does once it is up:
 *   - keeps the tray tooltip carrying a fresh SystemInfo snapshot,
 *   - answers the tray menu (copy, about, exit),
 *   - schedules the onboarding notifications and opens what they ask for.
 *
 * Owns only what it creates: WelcomeNotifier and IntegrationServer. The tray,
 * the dialogs, the settings store and the prompt are injected and outlive it —
 * in practice all four live on the stack of main(), with App declared last so
 * it is destroyed first.
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
     * @param settings Application-wide settings store. Not owned; must outlive App.
     * @param prompt   Channel for errors and confirmations. Not owned; must
     *                 outlive App.
     * @param tray     Tray icon and menu. Not owned; must outlive App.
     * @param dialogs  Channel for the application's windows. Not owned; must
     *                 outlive App.
     * @param info     Session snapshot, shared with the integration server it
     *                 builds. Not owned; must outlive App.
     * @param parent   Standard Qt parent.
     */
    explicit App(SettingsManager& settings,
                 UserPrompt& prompt,
                 TrayView& tray,
                 DialogPresenter& dialogs,
                 sysinfo::InfoSource& info,
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
    /// Collects a fresh snapshot every 30 s, replacing m_cachedInfo and the
    /// tray tooltip only when the rendered text differs from the cached one.
    void startTrayUpdateTimer();

    SettingsManager&    m_settings;          ///< Injected, not owned.
    UserPrompt&         m_prompt;            ///< Injected, not owned.
    TrayView&           m_tray;              ///< Injected, not owned.
    DialogPresenter&    m_dialogs;           ///< Injected, not owned.
    sysinfo::InfoSource& m_info;             ///< Injected, not owned.
    WelcomeNotifier*    m_notifier = nullptr; ///< Parent-owned via QObject(this).
    IntegrationServer*  m_server   = nullptr; ///< Parent-owned.
    QString             m_cachedInfo;        ///< Last rendered tray tooltip text.
};

#endif // APP_H
