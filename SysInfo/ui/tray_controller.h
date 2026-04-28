#ifndef TRAY_CONTROLLER_H
#define TRAY_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

#include <memory>

class QMenu;

/**
 * @brief UI adapter around QSystemTrayIcon and its context menu.
 *
 * Exposes a small semantic surface (copyRequested / aboutRequested /
 * quitRequested / notificationClicked) instead of leaking the underlying
 * QSystemTrayIcon to the App layer. The QSystemTrayIcon::messageClicked
 * signal is forwarded as TrayController::notificationClicked, decoupling
 * consumers from Qt's vocabulary.
 *
 * Memory model:
 *   - The tray icon is parented to this QObject and dies with the controller.
 *   - The context menu is a QWidget and cannot be parented to the tray
 *     icon (which is a bare QObject), so it is owned via std::unique_ptr.
 *     The destructor is declared out-of-line so QMenu can stay
 *     forward-declared in this header.
 */
class TrayController : public QObject
{
    Q_OBJECT
public:
    /// @param parent Standard Qt parent; usually the App composition root.
    explicit TrayController(QObject* parent = nullptr);
    ~TrayController() override;

    /**
     * @return true if the system tray is available on this platform/session.
     *         Must be checked before calling init() — a missing tray is a
     *         fatal configuration error for SysInfo.
     */
    static bool isSystemTrayAvailable();

    /**
     * @brief Build the tray icon and context menu.
     * @param iconPath Resource or filesystem path to the tray icon image.
     * @return true on success; false only on hard failures (icon currently
     *         missing is logged but does not block tray creation).
     */
    bool init(const QString& iconPath);

    /// Make the tray icon visible.
    void show();

    /// Update the tray-icon hover tooltip (typically the cached SystemInfo).
    void setTooltip(const QString& text);

    /**
     * @brief Display a balloon/toast notification next to the tray icon.
     * @param title Short title shown bold by the OS.
     * @param body  Multi-line body (newline-separated lines work on all OS).
     * @param icon  Visual cue (info/warning/critical).
     * @param msecs How long the notification stays visible.
     */
    void showNotification(const QString& title,
                          const QString& body,
                          QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                          int msecs = 5000);

signals:
    /// User selected "Copy to clipboard" from the menu.
    void copyRequested();
    /// User selected "About" from the menu.
    void aboutRequested();
    /// User selected "Exit" from the menu.
    void quitRequested();
    /// User clicked any tray notification (forwarded from QSystemTrayIcon::messageClicked).
    void notificationClicked();

private:
    /// Build the context menu and wire its actions to the public signals.
    void buildMenu();

    QSystemTrayIcon*       m_icon = nullptr;   ///< Parented to this (QObject).
    std::unique_ptr<QMenu> m_menu;             ///< QMenu is QWidget — no QObject parent possible.
};

#endif // TRAY_CONTROLLER_H
