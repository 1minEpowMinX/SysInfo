#ifndef TRAY_CONTROLLER_H
#define TRAY_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

#include <memory>

class QMenu;

class TrayController : public QObject
{
    Q_OBJECT
public:
    explicit TrayController(QObject* parent = nullptr);
    ~TrayController() override;

    static bool isSystemTrayAvailable();

    bool init(const QString& iconPath);

    void show();
    void setTooltip(const QString& text);

    void showNotification(const QString& title,
                          const QString& body,
                          QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                          int msecs = 5000);

signals:
    void copyRequested();
    void aboutRequested();
    void quitRequested();
    void notificationClicked();

private:
    void buildMenu();

    QSystemTrayIcon* m_icon = nullptr;       // parented to this (QObject)
    std::unique_ptr<QMenu> m_menu;           // QMenu is QWidget (no QObject parent)
};

#endif // TRAY_CONTROLLER_H
