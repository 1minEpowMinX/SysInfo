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
    static App& instance();

    void startApp();

private:
    explicit App(QObject *parent = nullptr);
    ~App();

    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QString cachedInfo;

    void systemTraySupportCheck();
    void createTrayIcon(const QString &iconPath);
    void loadTrayApp();
    void createContextMenu();
    void startTrayUpdateTimer();


private slots:
    void showTrayGuide();
    void copyToClipboard();
    void showHelp();
    void showAboutDialog();
    void quitApp();
};
#endif // APP_H
