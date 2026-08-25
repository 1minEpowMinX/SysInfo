#include "app.h"

#include "welcome_notifier.h"
#include "core/logging/logger.h"
#include "core/ports/dialog_presenter.h"
#include "core/ports/tray_view.h"
#include "core/ports/user_prompt.h"
#include "core/settings/onboarding_flags.h"
#include "core/sysinfo/info_source.h"
#include "core/sysinfo/system_info_presenter.h"
#include "services/integration_server.h"

#include <QApplication>
#include <QClipboard>
#include <QTimer>

#include <utility>

namespace {

/// Interval between tray tooltip re-collections.
constexpr int kTrayUpdateIntervalMs = 30'000;

/// How long the "copied to the clipboard" confirmation stays on screen.
constexpr int kCopyNoticeMs         = 5'000;

} // namespace

// The settings store enters as one port plus one string rather than whole, so
// App can name the store without being able to read it. The path is by value
// because it is fixed for the life of that store.
App::App(OnboardingFlags& flags,
         IntegrationServer& server,
         UserPrompt& prompt,
         TrayView& tray,
         DialogPresenter& dialogs,
         sysinfo::InfoSource& info,
         QString settingsFilePath,
         QObject* parent)
    : QObject(parent)
    , m_flags(flags)
    , m_server(server)
    , m_prompt(prompt)
    , m_tray(tray)
    , m_dialogs(dialogs)
    , m_info(info)
    , m_settingsFilePath(std::move(settingsFilePath))
{
    QApplication::setQuitOnLastWindowClosed(false);
}

bool App::start()
{
    // Gates on the tray before anything is collected, to avoid an idle load.
    if (!m_tray.init()) {
        m_prompt.showError(tr("Error"),
                           tr("The system tray is unavailable."));
        Logger::log(Logger::EventId::TrayUnavailable,
                    "The system tray is unavailable. The application will be terminated.");
        return false;
    }

    m_cachedInfo = sysinfo::presenter::toText(m_info.current());
    m_tray.setTooltip(m_cachedInfo);
    m_tray.show();

    connect(&m_tray, &TrayView::copyRequested,  this, &App::onCopyRequested);
    connect(&m_tray, &TrayView::aboutRequested, this, &App::onAboutRequested);
    connect(&m_tray, &TrayView::quitRequested,  this, &App::onQuitRequested);

    // The guide retires itself from inside its own window, and the flag it
    // clears is this layer's to persist. Its reader sits in WelcomeNotifier;
    // OnboardingFlags carries both halves so that the flag's lifetime has one
    // place to be read in.
    connect(&m_dialogs, &DialogPresenter::trayGuideDismissedForGood, this,
            [this] { m_flags.setShowTrayGuide(false); });

    startTrayUpdateTimer();

    m_notifier = new WelcomeNotifier(m_tray, m_flags,
                                     WelcomeNotifier::kDefaultLifetimes, this);
    connect(m_notifier, &WelcomeNotifier::trayGuideRequested,
            this, &App::onTrayGuideRequested);
    m_notifier->scheduleShow();

    if (!m_server.start()) {
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
        const QString fresh = sysinfo::presenter::toText(m_info.current());
        if (fresh != m_cachedInfo) {
            m_cachedInfo = fresh;
            m_tray.setTooltip(m_cachedInfo);
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

    // Answers a user action, so the re-collection window is bypassed.
    m_info.refresh();
    m_cachedInfo = sysinfo::presenter::toText(m_info.current());
    clipboard->setText(m_cachedInfo);
    m_tray.setTooltip(m_cachedInfo);

    m_tray.showNotification(tr("System information"),
                            tr("Information copied to the clipboard."),
                            kCopyNoticeMs);
}

void App::onAboutRequested()
{
    m_dialogs.showAbout(
        sysinfo::presenter::toAboutFacts(m_info.current(), m_settingsFilePath));
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
    m_dialogs.showTrayGuide();
}
