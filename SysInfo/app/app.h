#ifndef APP_H
#define APP_H

#include <QObject>
#include <QString>

class IntegrationServer;
class SettingsManager;
class TrayController;
class UserPrompt;
class WelcomeNotifier;

/**
 * @brief Composes and owns the application's top-level objects.
 *
 * Owns and wires together the three runtime parts of SysInfo:
 *   - TrayController    : tray icon and context menu (UI),
 *   - WelcomeNotifier   : delayed onboarding notifications,
 *   - IntegrationServer : local HTTP endpoint for the browser extension.
 *
 * Also holds the rendered SystemInfo string that feeds the tray tooltip and
 * the clipboard.
 *
 * Holds no business logic of its own beyond
 *   "periodically refresh the tray tooltip with a fresh SystemInfo snapshot".
 *
 * Everything user-facing goes through the injected UserPrompt, so this class
 * names no widget type and blocks on no dialog of its own.
 *
 * Both injected references must outlive this App instance — in practice all
 * three live on the stack of main(), with App declared last so it is destroyed
 * first.
 */
class App : public QObject
{
    Q_OBJECT

public:
    /**
     * @param settings Application-wide settings store. Not owned; must outlive App.
     * @param prompt   Channel for errors and confirmations. Not owned; must
     *                 outlive App.
     * @param parent   Standard Qt parent.
     */
    explicit App(SettingsManager& settings,
                 UserPrompt& prompt,
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
    /// Shows the modal "About SysInfo" dialog.
    void onAboutRequested();
    /// Confirms with the user, logs AppExit, and quits the QApplication.
    void onQuitRequested();
    /// Opens the Windows-only tray-pinning guide dialog.
    void onTrayGuideRequested();

private:
    /// Collects a fresh snapshot every 30 s, replacing m_cachedInfo and the
    /// tray tooltip only when the rendered text differs from the cached one.
    void startTrayUpdateTimer();

    SettingsManager&    m_settings;          ///< Injected, not owned.
    UserPrompt&         m_prompt;            ///< Injected, not owned.
    TrayController*     m_tray     = nullptr; ///< Parent-owned via QObject(this).
    WelcomeNotifier*    m_notifier = nullptr; ///< Parent-owned.
    IntegrationServer*  m_server   = nullptr; ///< Parent-owned.
    QString             m_cachedInfo;        ///< Last rendered tray tooltip text.
};

#endif // APP_H
