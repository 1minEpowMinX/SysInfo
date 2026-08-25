#ifndef TRAY_VIEW_H
#define TRAY_VIEW_H

#include "notification_sink.h"

#include <QString>

/*
 * The interface App drives the tray through, so the composition layer names
 * no widget type. TrayController in ui/ is the implementation the application
 * runs with.
 *
 * Extends NotificationSink rather than restating it: WelcomeNotifier needs
 * only the notification half and takes that narrower interface, while App
 * needs the icon and the menu as well.
 *
 * Which image the icon carries is left to the implementation — a resource path
 * is a detail of the widget toolkit and has no place in the port.
 */

/**
 * @brief Presents the tray icon and reports what the user picked from it.
 */
class TrayView : public NotificationSink
{
    Q_OBJECT
public:
    /// @param parent Standard Qt parent.
    explicit TrayView(QObject* parent = nullptr) : NotificationSink(parent) {}

    /**
     * @brief Builds the icon and its context menu.
     * @return false if the system tray is unavailable on this session, or if
     *         this view is already initialised.
     */
    [[nodiscard]] virtual bool init() = 0;

    /// Makes the icon visible.
    virtual void show() = 0;

    /// Updates the hover tooltip.
    virtual void setTooltip(const QString& text) = 0;

signals:
    /// Fires when the user selects "Copy to clipboard" from the menu.
    void copyRequested();
    /// Fires when the user selects "About" from the menu.
    void aboutRequested();
    /// Fires when the user selects "Exit" from the menu.
    void quitRequested();
};

#endif // TRAY_VIEW_H
