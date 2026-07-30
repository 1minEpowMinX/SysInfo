#ifndef TRAY_CONTROLLER_H
#define TRAY_CONTROLLER_H

#include <QObject>
#include <QString>
#include <QSystemTrayIcon>

#include <memory>

class QMenu;

/**
 * @brief Adapts QSystemTrayIcon and its context menu to the application's signals.
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
     *         init() refuses to build anything while it is false; SysInfo
     *         treats a missing tray as a fatal configuration error.
     */
    static bool isSystemTrayAvailable();

    /**
     * @brief Builds the tray icon and context menu.
     *
     * An icon that fails to load is logged as TrayIconMissing and does not
     * block creation — the tray entry appears with an empty icon.
     *
     * @param iconPath Resource or filesystem path to the tray icon image.
     * @return true once the icon and menu exist; false if the system tray is
     *         unavailable, or if this controller is already initialised.
     */
    [[nodiscard]] bool init(const QString& iconPath);

    /// Makes the tray icon visible.
    void show();

    /// Updates the tray-icon hover tooltip (typically the cached SystemInfo).
    void setTooltip(const QString& text);

    /**
     * @brief Displays a balloon/toast notification next to the tray icon.
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
    /// Fires when the user selects "Copy to clipboard" from the menu.
    void copyRequested();
    /// Fires when the user selects "About" from the menu.
    void aboutRequested();
    /// Fires when the user selects "Exit" from the menu.
    void quitRequested();
    /// Fires when the user clicks any tray notification (forwarded from QSystemTrayIcon::messageClicked).
    void notificationClicked();

private:
    /// Builds the context menu and wires its actions to the public signals.
    void buildMenu();

    QSystemTrayIcon*       m_icon = nullptr;   ///< Parented to this (QObject).
    std::unique_ptr<QMenu> m_menu;             ///< QMenu is QWidget — no QObject parent possible.
};

#endif // TRAY_CONTROLLER_H
