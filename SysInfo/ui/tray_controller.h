#ifndef TRAY_CONTROLLER_H
#define TRAY_CONTROLLER_H

#include "core/ports/tray_view.h"

#include <QString>
#include <QSystemTrayIcon>

#include <memory>

class QMenu;

/**
 * @brief Serves TrayView through QSystemTrayIcon and its context menu.
 *
 * Exposes the port's semantic surface (copyRequested / aboutRequested /
 * quitRequested / notificationClicked) instead of leaking QSystemTrayIcon to
 * the App layer. The QSystemTrayIcon::messageClicked signal is forwarded as
 * NotificationSink::notificationClicked, decoupling consumers from Qt's
 * vocabulary.
 *
 * Memory model:
 *   - The tray icon is parented to this QObject and dies with the controller.
 *   - The context menu is held by std::unique_ptr and dies with the controller
 *     as well. The destructor is declared out-of-line, which keeps QMenu
 *     forward-declared in this header.
 */
class TrayController : public TrayView
{
    Q_OBJECT
public:
    /// Resource path of the icon the application ships.
    static const QString kDefaultIconPath;

    /**
     * @param iconPath Resource or filesystem path to the tray icon image.
     * @param parent   Standard Qt parent.
     */
    explicit TrayController(QString iconPath = kDefaultIconPath,
                            QObject* parent = nullptr);
    ~TrayController() override;

    /**
     * @brief Builds the tray icon and context menu.
     *
     * An icon that fails to load is logged as TrayIconMissing and does not
     * block creation — the tray entry appears with an empty icon.
     *
     * @return true once the icon and menu exist; false if the system tray is
     *         unavailable, or if this controller is already initialised.
     */
    [[nodiscard]] bool init() override;

    /// Makes the tray icon visible.
    void show() override;

    /// Updates the tray-icon hover tooltip (typically the cached SystemInfo).
    void setTooltip(const QString& text) override;

    /// Displays a balloon/toast notification next to the tray icon. Does
    /// nothing before init() has built the icon.
    void showNotification(const QString& title,
                          const QString& body,
                          int msecs) override;

private:
    /**
     * @return true if the system tray is available on this platform/session.
     *         init() refuses to build anything while it is false.
     */
    static bool isSystemTrayAvailable();

    /// Builds the context menu and wires its actions to the port's signals.
    void buildMenu();

    QString                m_iconPath;        ///< Image init() loads the icon from.
    QSystemTrayIcon*       m_icon = nullptr;  ///< Parented to this (QObject).
    std::unique_ptr<QMenu> m_menu;            ///< QMenu is QWidget — no QObject parent possible.
};

#endif // TRAY_CONTROLLER_H
