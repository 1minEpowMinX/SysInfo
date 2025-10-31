#ifndef APP_H
#define APP_H

#include "../integration_server/integration_server.h"

#include <QMenu>
#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

class App : public QObject
{
    Q_OBJECT

public:
    static App& instance();

    void startApp();

private:
    explicit App(QObject *parent = nullptr);
    ~App();

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QString cachedInfo;

    IntegrationServer *integrationServer = nullptr;

    void systemTraySupportCheck();
    void createTrayIcon(const QString &iconPath);
    void loadTrayApp();
    void createContextMenu();
    void startTrayUpdateTimer();

private slots:
    void showTrayGuide();
    void copyToClipboard();
    void showAboutDialog();
    void quitApp();
};
#endif // APP_H
