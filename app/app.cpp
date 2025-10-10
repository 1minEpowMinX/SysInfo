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

    SystemTraySupportCheck();
    LoadTrayIcon(":/assets/SysInfo-DALL.E.ico");
    CreateContextMenu();

    trayIcon->show();
}

App::~App() {
    delete trayMenu;
    delete trayIcon;
}

void App::SystemTraySupportCheck() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Ошибка"), QObject::tr("Системный трей недоступен."));
        qApp->quit();
    }
}

void App::getSystemInfo()
{
    QString hostname = Utils::getHostname();
    QString username = Utils::getUsername();
    QString ip = Utils::getActiveIPAddress();
    QString uptime = Utils::getLastBootTime();

    cachedInfo = QObject::tr("Имя устройства: %1\nПользователь: %2\nIP-адрес: %3\nВремя включения: %4")
    .arg(hostname, username, ip, uptime);
}

void App::LoadTrayIcon(const QString &iconPath) {
    trayIcon = new QSystemTrayIcon(QIcon(iconPath), this);
    trayIcon->setVisible(true);

    App::getSystemInfo();
    trayIcon->setToolTip(cachedInfo);

    // Timer for periodic information updates
    QTimer *updateTimer = new QTimer(this);

    connect(updateTimer, &QTimer::timeout, this, [this]() {
        QString oldInfo = cachedInfo;
        getSystemInfo();
        if (cachedInfo != oldInfo)
            trayIcon->setToolTip(cachedInfo);
    });
    updateTimer->start(30 * 1000);
}

void App::CreateContextMenu() {
    trayMenu = new QMenu();
    QAction *actionShow = trayMenu->addAction(QObject::tr("Скопировать в буфер обмена"));
    QAction *actionQuit = trayMenu->addAction(QObject::tr("Выход"));

    QObject::connect(actionShow, &QAction::triggered, this, &App::CopyToClipboard);
    QObject::connect(actionQuit, &QAction::triggered, this, &App::QuitApp);

    trayIcon->setContextMenu(trayMenu);
}

void App::CopyToClipboard() {
    QClipboard *clipBoard = QApplication::clipboard();
    if (!clipBoard) return;
    clipBoard->setText(cachedInfo);

    trayIcon->showMessage(QObject::tr("Системная информация"), QObject::tr("Информация скопирована в буфер обмена."));
}

void App::QuitApp() {
    auto reply = QMessageBox::question(nullptr, QObject::tr("Выход"),
                                       QObject::tr("Мониторинг собирает данные, которые помогут в устранении неполадок.\nВсё равно закрыть приложение?"),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}
