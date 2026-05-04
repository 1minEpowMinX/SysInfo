#include "tray_controller.h"
#include "core/logging/logger.h"

#include <QAction>
#include <QIcon>
#include <QMenu>

TrayController::TrayController(QObject* parent)
    : QObject(parent)
{}

// Out-of-line destructor: unique_ptr<QMenu> needs a complete type for its
// deleter, which pull in via the <QMenu> include above.
TrayController::~TrayController() = default;

bool TrayController::isSystemTrayAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

bool TrayController::init(const QString& iconPath)
{
    m_icon = new QSystemTrayIcon(QIcon(iconPath), this);
    if (m_icon->icon().isNull()) {
        Logger::log(Logger::EventId::TrayIconMissing,
                    "Tray icon failed to load from resources.");
    }

    buildMenu();

    connect(m_icon, &QSystemTrayIcon::messageClicked,
            this, &TrayController::notificationClicked);

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

    connect(copy,  &QAction::triggered, this, &TrayController::copyRequested);
    connect(about, &QAction::triggered, this, &TrayController::aboutRequested);
    connect(quit,  &QAction::triggered, this, &TrayController::quitRequested);

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
                                      QSystemTrayIcon::MessageIcon icon,
                                      int msecs)
{
    if (m_icon) {
        m_icon->showMessage(title, body, icon, msecs);
    }
}
