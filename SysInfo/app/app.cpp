#include "app.h"

#include "user_prompt.h"
#include "welcome_notifier.h"
#include "core/logging/logger.h"
#include "core/settings/settings_manager.h"
#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"
#include "services/integration_server.h"
#include "ui/about_dialog.h"
#include "ui/tray_controller.h"
#include "ui/tray_guide.h"

#include <QApplication>
#include <QClipboard>
#include <QTimer>

namespace {

/// Interval between tray tooltip re-collections, and therefore the longest a
/// changed IP address or boot time can stay absent from the tooltip.
constexpr int kTrayUpdateIntervalMs = 30'000;

/// How long the "copied to the clipboard" confirmation stays on screen.
constexpr int kCopyNoticeMs         = 5'000;

constexpr const char* kTrayIconPath = ":/resources/icons/sysinfo_icon.png";

} // namespace

App::App(SettingsManager& settings, UserPrompt& prompt, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_prompt(prompt)
{
    QApplication::setQuitOnLastWindowClosed(false);
}

bool App::start()
{
    // Gates on the tray before anything is collected: a failure here ends the
    // run, and sysinfo::collect() walks every network interface.
    m_tray = new TrayController(this);
    if (!m_tray->init(kTrayIconPath)) {
        m_prompt.showError(tr("Error"),
                           tr("The system tray is unavailable."));
        Logger::log(Logger::EventId::TrayUnavailable,
                    "The system tray is unavailable. The application will be terminated.");
        return false;
    }

    m_cachedInfo = sysinfo::presenter::toText(sysinfo::collect());
    m_tray->setTooltip(m_cachedInfo);
    m_tray->show();

    connect(m_tray, &TrayController::copyRequested,  this, &App::onCopyRequested);
    connect(m_tray, &TrayController::aboutRequested, this, &App::onAboutRequested);
    connect(m_tray, &TrayController::quitRequested,  this, &App::onQuitRequested);

    startTrayUpdateTimer();

    m_notifier = new WelcomeNotifier(*m_tray, m_settings,
                                     WelcomeNotifier::kDefaultLifetimes, this);
    connect(m_notifier, &WelcomeNotifier::trayGuideRequested,
            this, &App::onTrayGuideRequested);
    m_notifier->scheduleShow();

    m_server = new IntegrationServer(m_settings, this);
    if (!m_server->start()) {
        m_prompt.showError(tr("Error"),
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
        const QString fresh = sysinfo::presenter::toText(sysinfo::collect());
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

    m_cachedInfo = sysinfo::presenter::toText(sysinfo::collect());
    clipboard->setText(m_cachedInfo);
    m_tray->setTooltip(m_cachedInfo);

    m_tray->showNotification(tr("System information"),
                             tr("Information copied to the clipboard."),
                             kCopyNoticeMs);
}

void App::onAboutRequested()
{
    const QString details =
        sysinfo::presenter::toSystemDetailsHtml(sysinfo::collect(),
                                                m_settings.filePath());
    AboutDialog dlg(details);
    dlg.exec();
}

void App::onQuitRequested()
{
    const QString text = tr(
        "The application collects system information and assists in diagnostics."
        "<p><b>Do you still want to close the application?</b></p>");

    if (m_prompt.confirm(tr("Exit"), text)) {
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
