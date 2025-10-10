#ifndef APP_H
#define APP_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QString>
#include <QMenu>

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(QObject *parent = nullptr);
    ~App();

private:
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QString cachedInfo;

    void SystemTraySupportCheck();
    void CreateTrayIcon(const QString &iconPath);
    void LoadTrayApp();
    void CreateContextMenu();
    void startTrayUpdateTimer();


private slots:
    void CopyToClipboard();
    void QuitApp();
};
#endif // APP_H
