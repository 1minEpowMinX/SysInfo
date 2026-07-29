#include "welcome_notifier.h"

#include "core/settings/settings_manager.h"
#include "ui/tray_controller.h"

#include <QTimer>

namespace {

/// How long each notification stays on screen, and therefore the span during
/// which an incoming click belongs to it.
constexpr int kWelcomeLifetimeMs   = 15'000;
constexpr int kTrayGuideLifetimeMs = 25'000;

} // namespace

WelcomeNotifier::WelcomeNotifier(TrayController& tray,
                                 SettingsManager& settings,
                                 QObject* parent)
    : QObject(parent)
    , m_tray(tray)
    , m_settings(settings)
{}

void WelcomeNotifier::scheduleShow(int delayMs)
{
    QTimer::singleShot(delayMs, this, &WelcomeNotifier::onTimerFired);
}

void WelcomeNotifier::onTimerFired()
{
    // General welcome — shown once on any supported OS.
    if (m_settings.showWelcome()) {
        m_tray.showNotification(
            tr("SysInfo runs in the background"),
            tr("The application collects system information and assists in diagnostics.\n"
               "For more details, see the \"About\" section."),
            QSystemTrayIcon::Information,
            kWelcomeLifetimeMs);
        m_settings.setShowWelcome(false);

        // The hint follows once this message has expired, leaving exactly one
        // notification clickable at any moment.
        QTimer::singleShot(kWelcomeLifetimeMs, this,
                           &WelcomeNotifier::showTrayGuideHint);
        return;
    }

    showTrayGuideHint();
}

void WelcomeNotifier::showTrayGuideHint()
{
#ifdef Q_OS_WINDOWS
    if (!m_settings.showTrayGuide()) {
        return;
    }

    // The subscription spans this notification and nothing beyond it:
    // SingleShotConnection ends it on the first click, the timer below ends it
    // when the notification expires unclicked.
    const QMetaObject::Connection link =
        connect(&m_tray, &TrayController::notificationClicked,
                this, &WelcomeNotifier::trayGuideRequested,
                Qt::SingleShotConnection);

    m_tray.showNotification(
        tr("Make the icon visible in the tray"),
        tr("Drag the SysInfo icon to the notification area.\n"
           "Click here to open detailed instructions."),
        QSystemTrayIcon::Information,
        kTrayGuideLifetimeMs);

    QTimer::singleShot(kTrayGuideLifetimeMs, this,
                       [link] { QObject::disconnect(link); });
#endif
}
