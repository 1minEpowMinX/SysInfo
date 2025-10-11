#include "app.h"
#include "../utils/utils.h"

#include <QObject>
#include <QApplication>
#include <QMessageBox>
#include <QAction>
#include <QClipboard>
#include <QTimer>
#include <QString>


App::App(QObject *parent)
    : QObject(parent)
{
    QApplication::setQuitOnLastWindowClosed(false);

    cachedInfo = Utils::getSystemInfo();

    App::systemTraySupportCheck();
    App::loadTrayApp();
    App::createContextMenu();

    trayIcon->show();
}

App::~App() {
    delete trayMenu;
    delete trayIcon;
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
        QString oldInfo = cachedInfo;
        Utils::getSystemInfo();
        if (cachedInfo != oldInfo)
            trayIcon->setToolTip(cachedInfo);
    });
    updateTimer->start(30 * 1000);
}

void App::createTrayIcon(const QString &iconPath) {
    trayIcon = new QSystemTrayIcon(QIcon(iconPath), this);
    trayIcon->setVisible(true);
}


void App::loadTrayApp() {
    App::createTrayIcon(":/assets/SysInfo_Icon.ico");
    trayIcon->setToolTip(cachedInfo);
    App::startTrayUpdateTimer();
}

void App::createContextMenu() {
    trayMenu = new QMenu();
    QAction *actionShow = trayMenu->addAction(QObject::tr("Скопировать в буфер обмена"));
    QAction *actionQuit = trayMenu->addAction(QObject::tr("Выход"));

    QObject::connect(actionShow, &QAction::triggered, this, &App::copyToClipboard);
    QObject::connect(actionQuit, &QAction::triggered, this, &App::quitApp);

    trayIcon->setContextMenu(trayMenu);
}

void App::copyToClipboard() {
    QClipboard *clipBoard = QApplication::clipboard();
    if (!clipBoard) return;
    clipBoard->setText(cachedInfo);

    trayIcon->showMessage(QObject::tr("Системная информация"), QObject::tr("Информация скопирована в буфер обмена."));
}

void App::quitApp() {
    auto reply = QMessageBox::question(nullptr, QObject::tr("Выход"),
                                       QObject::tr("Мониторинг собирает данные, которые помогут в устранении неполадок.\nВсё равно закрыть приложение?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}
