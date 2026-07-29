#ifndef WELCOME_NOTIFIER_H
#define WELCOME_NOTIFIER_H

#include <QObject>

class SettingsManager;
class TrayController;

/**
 * @brief Shows the onboarding notifications after a delay.
 *
 * After a configurable delay, shows a one-time welcome tray message and,
 * on Windows only, a one-time hint on how to pin the tray icon. Both
 * notifications are guarded by SettingsManager flags so they fire at most
 * once per user profile.
 *
 * Does not own TrayController or SettingsManager — both are injected by
 * reference and must outlive the notifier (guaranteed by the owning App).
 *
 * onTimerFired() runs at most once per scheduleShow() call via a
 * QTimer::singleShot, so no internal de-duplication is needed.
 *
 * The two notifications are never on screen together, and the subscription to
 * notification clicks lasts only while the hint that owns it is displayed.
 * QSystemTrayIcon::messageClicked reports that a notification was clicked but
 * not which one, so arrival time is what identifies the source.
 */
class WelcomeNotifier : public QObject
{
    Q_OBJECT
public:
    /**
     * @param tray     Tray adapter used to display notifications.
     * @param settings Settings store consulted for the one-shot flags.
     * @param parent   Standard Qt parent.
     */
    explicit WelcomeNotifier(TrayController& tray,
                             SettingsManager& settings,
                             QObject* parent = nullptr);

    /// Schedules the welcome/guide notifications to fire after @p delayMs.
    void scheduleShow(int delayMs = 60'000);

signals:
    /// Fires when the user clicks the Windows tray-guide hint notification.
    void trayGuideRequested();

private:
    /// Timer callback: shows the welcome message, then schedules the tray hint.
    void onTimerFired();

    /**
     * @brief Shows the Windows-only hint on pinning the tray icon.
     *
     * While the hint is displayed, a notification click emits
     * trayGuideRequested(). Does nothing once the user has dismissed the hint
     * for good, and nothing at all on other platforms.
     */
    void showTrayGuideHint();

    TrayController&  m_tray;       ///< Injected tray adapter (not owned).
    SettingsManager& m_settings;   ///< Injected settings store (not owned).
};

#endif // WELCOME_NOTIFIER_H
