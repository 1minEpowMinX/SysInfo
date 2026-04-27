#include "welcome_notifier.h"

#include "../settings/settings_manager.h"
#include "../tray/tray_controller.h"

#include <QTimer>

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
            15'000);
        m_settings.setShowWelcome(false);
    }

#ifdef Q_OS_WINDOWS
    // Windows-only hint on how to pin the tray icon.
    if (m_settings.showTrayGuide()) {
        connect(&m_tray, &TrayController::notificationClicked,
                this, &WelcomeNotifier::trayGuideRequested);

        m_tray.showNotification(
            tr("Make the icon visible in the tray"),
            tr("Drag the SysInfo icon to the notification area.\n"
               "Click here to open detailed instructions."),
            QSystemTrayIcon::Information,
            25'000);
    }
#endif
}
