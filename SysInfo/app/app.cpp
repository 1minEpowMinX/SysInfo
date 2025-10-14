#include "app.h"
#include "../settings/settingsmanager.h"
#include "../tray/aboutdialog.h"
#include "../tray/trayguide.h"
#include "../utils/utils.h"

#include <QObject>
#include <QApplication>
#include <QMessageBox>
#include <QAction>
#include <QClipboard>
#include <QTimer>
#include <QString>

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
            QObject::tr("SysInfo работает в фоне"),
            QObject::tr("Приложение собирает системную информацию и помогает в диагностике.\n"
                        "Подробнее см. раздел \"Справка\"."),
            QSystemTrayIcon::Information,
            8000
            );
        SettingsManager::instance().setShowWelcome(false);
    }

    // Instructions for pinning the icon to the taskbar — Windows only
#ifdef Q_OS_WINDOWS
    if (SettingsManager::instance().showTrayGuide()) {
        trayIcon->showMessage(
            QObject::tr("Сделайте значок видимым в трее"),
            QObject::tr("Перетащите значок SysInfo в область уведомлений.\n"
                        "Нажмите сюда, чтобы открыть подробную инструкцию."),
            QSystemTrayIcon::Information,
            15000
            );

        connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &App::showTrayGuide);
    }
#endif

    integrationServer = new IntegrationServer(this);
    if (!integrationServer->start()) {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"), QObject::tr("Не удалось запустить локальный сервер. Интеграция с Jira недоступна."));
    }
}

void App::systemTraySupportCheck() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"), QObject::tr("Системный трей недоступен."));
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
    QAction *actionShow = trayMenu->addAction(QObject::tr("Скопировать в буфер обмена"));
    QAction *actionHelp = trayMenu->addAction(tr("Справка"));
    QAction *aboutApp = trayMenu->addAction(QObject::tr("О программе"));
    QAction *actionQuit = trayMenu->addAction(QObject::tr("Выход"));

    QObject::connect(actionShow, &QAction::triggered, this, &App::copyToClipboard);
    QObject::connect(actionHelp, &QAction::triggered, this, &App::showHelp);
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

    trayIcon->showMessage(QObject::tr("Системная информация"), QObject::tr("Информация скопирована в буфер обмена."));
}

void App::showHelp() {
    QString text = QObject::tr(
        "<b>Приложение собирает системную информацию и помогает в диагностике:</b><br>"
        "• Отображает сведения о устройстве (имя, пользователь, IP, время работы).<br>"
        "• Работает в фоновом режиме через системный трей.<br>"
        "• Нажмите правой кнопкой на значке, чтобы открыть меню действий.<br>"
        "• Пункт «Скопировать в буфер» копирует текущие сведения.<br><br>"
        "Если значок в трее не виден – перетащите его в область уведомлений."
        );

    QMessageBox::information(nullptr, QObject::tr("Справка"), text);
}

void App::showAboutDialog() {
    AboutDialog dlg;
    dlg.exec();
}

void App::quitApp() {
    QString text = QObject::tr(
        "Приложение собирает системную информацию и помогает в диагностике.\nВсё равно закрыть приложение?");

    auto reply = QMessageBox::question(nullptr, QObject::tr("Выход"), text,
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}
