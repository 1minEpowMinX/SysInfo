#include "tray_controller.h"
#include "core/logging/logger.h"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QSize>

#include <utility>

namespace
{
    /// Size rendered once to tell a loadable icon from a missing file.
    constexpr QSize kProbeIconSize(16, 16);
}

const QString TrayController::kDefaultIconPath =
    QStringLiteral(":/resources/icons/sysinfo_app.svg");

TrayController::TrayController(QString iconPath, QObject* parent)
    : TrayView(parent)
    , m_iconPath(std::move(iconPath))
{}

// Out-of-line destructor: unique_ptr<QMenu> needs a complete type for its
// deleter, which the <QMenu> include above supplies.
TrayController::~TrayController() = default;

bool TrayController::isSystemTrayAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool TrayController::init()
{
    if (!isSystemTrayAvailable()) {
        return false;
    }

    // A second call would orphan the previous icon on this QObject and drop
    // the menu that the tray icon still points at.
    if (m_icon != nullptr) {
        return false;
    }

    const QIcon icon(m_iconPath);

    // QIcon stores the path without reading it, so isNull() answers false even
    // for a path that resolves to nothing. Rendering one pixmap is what tells
    // a real icon from a missing file.
    if (icon.pixmap(kProbeIconSize).isNull()) {
        Logger::log(Logger::EventId::TrayIconMissing,
                    "Tray icon failed to load from resources.");
    }

    m_icon = new QSystemTrayIcon(icon, this);

    buildMenu();

    connect(m_icon, &QSystemTrayIcon::messageClicked,
            this, &NotificationSink::notificationClicked);

    return true;
}

void TrayController::buildMenu()
{
    // QMenu is a QWidget, so it cannot be parented to QSystemTrayIcon (a
    // bare QObject). It dies with the TrayController.
    m_menu = std::make_unique<QMenu>();

    QAction* copy  = m_menu->addAction(tr("Copy to clipboard"));
    QAction* about = m_menu->addAction(tr("About"));
    QAction* quit  = m_menu->addAction(tr("Exit"));

    connect(copy,  &QAction::triggered, this, &TrayView::copyRequested);
    connect(about, &QAction::triggered, this, &TrayView::aboutRequested);
    connect(quit,  &QAction::triggered, this, &TrayView::quitRequested);

    m_icon->setContextMenu(m_menu.get());
}

void TrayController::show()
{
    if (m_icon) {
        m_icon->setVisible(true);
        m_icon->show();
    }
}

void TrayController::setTooltip(const QString& text)
{
    if (m_icon) {
        m_icon->setToolTip(text);
    }
}

void TrayController::showNotification(const QString& title,
                                      const QString& body,
                                      int msecs)
{
    if (m_icon) {
        m_icon->showMessage(title, body, QSystemTrayIcon::Information, msecs);
    }
}
