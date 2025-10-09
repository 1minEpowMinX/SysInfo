#ifndef APP_H
#define APP_H

#include <QObject>
#include <QSystemTrayIcon>
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

    void SystemTraySupportCheck();
    void LoadTrayIcon(const QString &iconPath);
    void CreateContextMenu();

private slots:
    void CopyToClipboard();
    void QuitApp();
};
#endif // APP_H
