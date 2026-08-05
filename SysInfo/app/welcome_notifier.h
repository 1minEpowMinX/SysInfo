#ifndef WELCOME_NOTIFIER_H
#define WELCOME_NOTIFIER_H

#include <QObject>

class NotificationSink;
class OnboardingFlags;

/**
 * @brief Shows the onboarding notifications after a delay.
 *
 * After a configurable delay, shows a welcome tray message and, on Windows
 * only, a hint on how to pin the tray icon. Both consult OnboardingFlags
 * before showing.
 *
 * The two flags are retired by different parties. The welcome flag is cleared
 * here as the message is shown, so that message appears once per user profile.
 * The tray-guide flag is cleared only when the user ticks "Don't show again"
 * in the window the hint opens, so the hint returns on every run until they
 * do.
 *
 * Does not own the NotificationSink or the OnboardingFlags — both are injected
 * by reference and must outlive the notifier (guaranteed by the owning App).
 *
 * onTimerFired() runs at most once per scheduleShow() call, via a
 * QTimer::singleShot.
 *
 * The two notifications are never on screen together, and the subscription to
 * notification clicks lasts only while the hint that owns it is displayed.
 */
class WelcomeNotifier : public QObject
{
    Q_OBJECT
public:
    /// Delay scheduleShow() applies unless the caller names another one.
    static constexpr int kDefaultDelayMs = 60'000;

    /**
     * @brief Groups how long each notification stays on screen.
     *
     * A notification's lifetime is also the span during which an incoming
     * click belongs to it, so these values govern the sequencing and not just
     * the display.
     */
    struct Lifetimes
    {
        int welcomeMs;    ///< Welcome message.
        int trayGuideMs;  ///< Windows tray-pinning hint.
    };

    /// Lifetimes the application runs with.
    static constexpr Lifetimes kDefaultLifetimes{15'000, 25'000};

    /**
     * @param sink      Destination for the notifications.
     * @param flags     One-shot flags consulted before each notification.
     * @param lifetimes How long each notification stays up.
     * @param parent    Standard Qt parent.
     */
    explicit WelcomeNotifier(NotificationSink& sink,
                             OnboardingFlags& flags,
                             Lifetimes lifetimes = kDefaultLifetimes,
                             QObject* parent = nullptr);

    /// Schedules the welcome/guide notifications to fire after @p delayMs.
    void scheduleShow(int delayMs = kDefaultDelayMs);

signals:
    /// Fires when the user clicks the Windows tray-guide hint notification.
    void trayGuideRequested();

private:
    /// Shows the welcome message and schedules the tray hint for the moment it
    /// expires; goes straight to the hint when the welcome flag is already clear.
    void onTimerFired();

    /**
     * @brief Shows the Windows-only hint on pinning the tray icon.
     *
     * While the hint is displayed, a notification click emits
     * trayGuideRequested(). Does nothing once the user has dismissed the hint
     * for good, and nothing at all on other platforms.
     */
    void showTrayGuideHint();

    NotificationSink& m_sink;      ///< Injected notification destination (not owned).
    OnboardingFlags&  m_flags;     ///< Injected one-shot flags (not owned).
    Lifetimes         m_lifetimes; ///< How long each notification stays up.
};

#endif // WELCOME_NOTIFIER_H
