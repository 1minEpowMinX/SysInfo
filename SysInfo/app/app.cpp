#include "app.h"
#include "../settings/settingsmanager.h"
#include "../tray/aboutdialog.h"
#include "../tray/trayguide.h"
#include "../utils/utils.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QTimer>

App& App::instance() {
    static App inst;
    return inst;
}

App::App(QObject *parent)
    : QObject(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);
}

App::~App() {
    delete trayMenu;
    delete trayIcon;
}

void App::startApp()
{
    cachedInfo = Utils::toText(Utils::collectSystemInfo());

    systemTraySupportCheck();
    loadTrayApp();
    createContextMenu();

    trayIcon->show();

    // General information for all operating systems
    if (SettingsManager::instance().showWelcome()) {
        trayIcon->showMessage(
            QObject::tr("SysInfo runs in the background"),
            QObject::tr("The application collects system information and assists in diagnostics.\n"
                        "For more details, see the \"Help\" section."),
            QSystemTrayIcon::Information,
            8000
            );
        SettingsManager::instance().setShowWelcome(false);
    }

    // Instructions for pinning the icon to the taskbar — Windows only
#ifdef Q_OS_WINDOWS
    if (SettingsManager::instance().showTrayGuide()) {
        trayIcon->showMessage(
            QObject::tr("Make the icon visible in the tray"),
            QObject::tr("Drag the SysInfo icon to the notification area.\n"
                        "Click here to open detailed instructions."),
            QSystemTrayIcon::Information,
            15000
            );

        connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &App::showTrayGuide);
    }
#endif

    integrationServer = new IntegrationServer(this);
    if (!integrationServer->start()) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Failed to start the local server. "
                                                                         "Integration with Jira is unavailable."));
    }
}

void App::systemTraySupportCheck() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("The system tray is unavailable."));
        qApp->quit();
    }
}

void App::startTrayUpdateTimer() {
    // Timer for periodic information updates
    QTimer *updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, [this]() {
        QString newInfo = Utils::toText(Utils::collectSystemInfo());
        if (newInfo != cachedInfo) {
            cachedInfo = newInfo;
            trayIcon->setToolTip(cachedInfo);
        }
    });
    updateTimer->start(30 * 1000);
}

void App::createTrayIcon(const QString &iconPath) {
    trayIcon = new QSystemTrayIcon(QIcon(iconPath), this);
    trayIcon->setVisible(true);
}


void App::loadTrayApp() {
    App::createTrayIcon(":/resources/icons/SysInfo_Icon.ico");
    trayIcon->setToolTip(cachedInfo);
    App::startTrayUpdateTimer();
}

void App::createContextMenu() {
    trayMenu = new QMenu();
    QAction *actionShow = trayMenu->addAction(QObject::tr("Copy to clipboard"));
    QAction *aboutApp = trayMenu->addAction(QObject::tr("About"));
    QAction *actionQuit = trayMenu->addAction(QObject::tr("Exit"));

    QObject::connect(actionShow, &QAction::triggered, this, &App::copyToClipboard);
    QObject::connect(aboutApp, &QAction::triggered, this, &App::showAboutDialog);
    QObject::connect(actionQuit, &QAction::triggered, this, &App::quitApp);

    trayIcon->setContextMenu(trayMenu);
}

void App::showTrayGuide() {
    auto* guide = new TrayGuide();
    guide->show();
}

void App::copyToClipboard() {
    QClipboard *clipBoard = QApplication::clipboard();
    if (!clipBoard) return;
    clipBoard->setText(cachedInfo);

    trayIcon->showMessage(QObject::tr("System information"), QObject::tr("Information copied to the clipboard."));
}

void App::showAboutDialog() {
    AboutDialog dlg;
    dlg.exec();
}

void App::quitApp() {
    QString text = tr(
        "The application collects system information and assists in diagnostics."
        "<p><b>Do you still want to close the application?</b></p>");

    auto reply = QMessageBox::question(nullptr, QObject::tr("Exit"), text,
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}
