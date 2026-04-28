#ifndef APP_H
#define APP_H

#include <QObject>
#include <QString>

class IntegrationServer;
class SettingsManager;
class TrayController;
class WelcomeNotifier;

/**
 * @brief Top-level composition root.
 *
 * Owns and wires together the four runtime parts of SysInfo:
 *   - TrayController  : tray icon and context menu (UI),
 *   - WelcomeNotifier : delayed onboarding notifications,
 *   - IntegrationServer : local HTTP endpoint for the browser extension,
 *   - cached SystemInfo string used for the tray tooltip and clipboard.
 *
 * Holds no business logic of its own beyond
 *   "periodically refresh the tray tooltip with a fresh SystemInfo snapshot".
 *
 * The provided SettingsManager reference must outlive this App instance —
 * in practice both live on the stack of main(), with App declared after
 * SettingsManager so it is destroyed first.
 */
class App : public QObject
{
    Q_OBJECT

public:
    /**
     * @param settings Application-wide settings store. Not owned; must outlive App.
     * @param parent   Standard Qt parent.
     */
    explicit App(SettingsManager& settings, QObject* parent = nullptr);

    /**
     * @brief Bring the application up: build tray UI, schedule onboarding,
     *        start the integration server.
     * @return false if the system tray is unavailable on this session — a
     *         fatal configuration error; the caller (main) should exit.
     *         A failure of the integration server is non-fatal: the user
     *         is warned via QMessageBox and the app keeps running.
     */
    [[nodiscard]] bool start();

private slots:
    /// Refresh cached SystemInfo and copy it to the system clipboard.
    void onCopyRequested();
    /// Show the modal "About SysInfo" dialog.
    void onAboutRequested();
    /// Confirm with the user, log AppExit, and quit the QApplication.
    void onQuitRequested();
    /// Open the Windows-only tray-pinning guide dialog.
    void onTrayGuideRequested();

private:
    /// Periodically (every 30 s) refresh m_cachedInfo and update the tray tooltip.
    void startTrayUpdateTimer();

    SettingsManager&    m_settings;          ///< Injected, not owned.
    TrayController*     m_tray     = nullptr; ///< Parent-owned via QObject(this).
    WelcomeNotifier*    m_notifier = nullptr; ///< Parent-owned.
    IntegrationServer*  m_server   = nullptr; ///< Parent-owned.
    QString             m_cachedInfo;        ///< Last rendered tray tooltip text.
};

#endif // APP_H
