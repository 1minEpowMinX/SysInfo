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

void WelcomeNotifier::onTimerFired()
{
    // General welcome — shown once on any supported OS.
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

    // The subscription spans this notification and nothing beyond it:
    // SingleShotConnection ends it on the first click, the timer below ends it
    // when the notification expires unclicked.
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
