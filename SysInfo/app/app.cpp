#include "app.h"

#include "welcome_notifier.h"
#include "../integration_server/integration_server.h"
#include "../logger/logger.h"
#include "../settings/settings_manager.h"
#include "../tray/about_dialog.h"
#include "../tray/tray_controller.h"
#include "../tray/tray_guide.h"
#include "../utils/utils.h"

#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QTimer>

namespace {
constexpr int kTrayUpdateIntervalMs = 30'000;
constexpr int kWelcomeDelayMs       = 60'000;
constexpr const char* kTrayIconPath = ":/resources/icons/sysinfo_icon.png";
} // namespace

App::App(SettingsManager& settings, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
{
    QApplication::setQuitOnLastWindowClosed(false);
}

bool App::start()
{
    if (!TrayController::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("The system tray is unavailable."));
        Logger::log(Logger::EventId::TrayUnavailable,
                    "The system tray is unavailable. The application will be terminated.");
        return false;
    }

    m_cachedInfo = Utils::toText(Utils::collectSystemInfo());

    m_tray = new TrayController(this);
    m_tray->init(kTrayIconPath);
    m_tray->setTooltip(m_cachedInfo);
    m_tray->show();

    connect(m_tray, &TrayController::copyRequested,  this, &App::onCopyRequested);
    connect(m_tray, &TrayController::aboutRequested, this, &App::onAboutRequested);
    connect(m_tray, &TrayController::quitRequested,  this, &App::onQuitRequested);

    startTrayUpdateTimer();

    m_notifier = new WelcomeNotifier(*m_tray, m_settings, this);
    connect(m_notifier, &WelcomeNotifier::trayGuideRequested,
            this, &App::onTrayGuideRequested);
    m_notifier->scheduleShow(kWelcomeDelayMs);

    m_server = new IntegrationServer(this);
    if (!m_server->start()) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("Failed to start the local server. "
                                 "Integration with Jira SM is unavailable."));
        Logger::log(Logger::EventId::ServerStartError,
                    "Failed to start the local server. "
                    "Integration with Jira SM is unavailable.");
    }

    return true;
}

void App::startTrayUpdateTimer()
{
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        const QString fresh = Utils::toText(Utils::collectSystemInfo());
        if (fresh != m_cachedInfo) {
            m_cachedInfo = fresh;
            m_tray->setTooltip(m_cachedInfo);
        }
    });
    timer->start(kTrayUpdateIntervalMs);
}

void App::onCopyRequested()
{
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard) {
        Logger::log(Logger::EventId::ClipboardUnavailable,
                    "Clipboard is unavailable.");
        return;
    }

    m_cachedInfo = Utils::toText(Utils::collectSystemInfo());
    clipboard->setText(m_cachedInfo);
    m_tray->setTooltip(m_cachedInfo);

    m_tray->showNotification(tr("System information"),
                             tr("Information copied to the clipboard."),
                             QSystemTrayIcon::Information,
                             5000);
}

void App::onAboutRequested()
{
    AboutDialog dlg;
    dlg.exec();
}

void App::onQuitRequested()
{
    const QString text = tr(
        "The application collects system information and assists in diagnostics."
        "<p><b>Do you still want to close the application?</b></p>");

    const auto reply = QMessageBox::question(nullptr, tr("Exit"), text,
                                             QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        Logger::log(Logger::EventId::AppExit, "Application terminated by user.");
        qApp->quit();
    }
}

void App::onTrayGuideRequested()
{
    auto* guide = new TrayGuide(m_settings);
    guide->setAttribute(Qt::WA_DeleteOnClose);
    guide->show();
}
