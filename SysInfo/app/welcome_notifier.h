#ifndef WELCOME_NOTIFIER_H
#define WELCOME_NOTIFIER_H

#include <QObject>

class SettingsManager;
class TrayController;

/**
 * @brief Delayed onboarding notifications.
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

    /// Schedule the welcome/guide notifications to fire after @p delayMs.
    void scheduleShow(int delayMs = 60'000);

signals:
    /// Emitted when the user clicks the Windows tray-guide hint notification.
    void trayGuideRequested();

private:
    /// Timer callback: shows the welcome message and (on Windows) the guide hint.
    void onTimerFired();

    TrayController&  m_tray;       ///< Injected tray adapter (not owned).
    SettingsManager& m_settings;   ///< Injected settings store (not owned).
};

#endif // WELCOME_NOTIFIER_H
    /**
     * @brief Shows the Windows-only hint on pinning the tray icon.
     *
     * While the hint is displayed, a notification click emits
     * trayGuideRequested(). Does nothing once the user has dismissed the hint
     * for good, and nothing at all on other platforms.
     */
    void showTrayGuideHint();

