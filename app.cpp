#include "app.h"

#include <QApplication>
#include <QMessageBox>
#include <QAction>

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
        QMessageBox::critical(nullptr, "Ошибка", "Системный трей недоступен!");
        qApp->quit();
    }
}

void App::LoadTrayIcon(const QString &iconPath) {
    trayIcon = new QSystemTrayIcon(QIcon(iconPath), this);
    trayIcon->setVisible(true);
    trayIcon->setToolTip("Имя компьютера:\nЛогин:\nIP-адрес:\nВремя включения компьютера:");
}

void App::CreateContextMenu() {
    trayMenu = new QMenu();
    QAction *actionShow = trayMenu->addAction("Скопировать в буфер обмена");
    QAction *actionQuit = trayMenu->addAction("Выход");

    QObject::connect(actionShow, &QAction::triggered, this, &App::CopyToClipboard);
    QObject::connect(actionQuit, &QAction::triggered, this, &App::QuitApp);

    trayIcon->setContextMenu(trayMenu);
}

void App::CopyToClipboard() {
    // TODO: реализовать функцию
    trayIcon->showMessage("Системная информация", "Информация скопирована в буфер обмена.");
}

void App::QuitApp() {
    auto reply = QMessageBox::question(nullptr, "Выход",
                                       "Мониторинг собирает данные, которые помогут в устранении неполадок.\nВсё равно закрыть приложение?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
        qApp->quit();
}
