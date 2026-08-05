#include "welcome_notifier.h"

#include "core/ports/notification_sink.h"
#include "core/settings/onboarding_flags.h"

#include <QTimer>

WelcomeNotifier::WelcomeNotifier(NotificationSink& sink,
                                 OnboardingFlags& flags,
                                 Lifetimes lifetimes,
                                 QObject* parent)
    : QObject(parent)
    , m_sink(sink)
    , m_flags(flags)
    , m_lifetimes(lifetimes)
{}

void WelcomeNotifier::scheduleShow(int delayMs)
{
    QTimer::singleShot(delayMs, this, &WelcomeNotifier::onTimerFired);
}

// Nothing guards against a second entry: the singleShot above fires once per
// scheduleShow() call.
void WelcomeNotifier::onTimerFired()
{
    if (m_flags.showWelcome()) {
        m_sink.showNotification(
            tr("SysInfo runs in the background"),
            tr("The application collects system information and assists in diagnostics.\n"
               "For more details, see the \"About\" section."),
            m_lifetimes.welcomeMs);
        m_flags.setShowWelcome(false);

        // The hint follows once this message has expired, leaving exactly one
        // notification clickable at any moment.
        QTimer::singleShot(m_lifetimes.welcomeMs, this,
                           &WelcomeNotifier::showTrayGuideHint);
        return;
    }

    showTrayGuideHint();
}

void WelcomeNotifier::showTrayGuideHint()
{
#ifdef Q_OS_WINDOWS
    if (!m_flags.showTrayGuide()) {
        return;
    }

    // QSystemTrayIcon::messageClicked reports that a notification was clicked
    // but not which one, so the window the click arrives in is the only thing
    // identifying its source — hence a subscription bounded by this hint.
    const QMetaObject::Connection link =
        connect(&m_sink, &NotificationSink::notificationClicked,
                this, &WelcomeNotifier::trayGuideRequested,
                Qt::SingleShotConnection);

    m_sink.showNotification(
        tr("Make the icon visible in the tray"),
        tr("Drag the SysInfo icon to the notification area.\n"
           "Click here to open detailed instructions."),
        m_lifetimes.trayGuideMs);

    QTimer::singleShot(m_lifetimes.trayGuideMs, this,
                       [link] { QObject::disconnect(link); });
#endif
}
