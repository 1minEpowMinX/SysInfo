#ifndef NOTIFICATION_SINK_H
#define NOTIFICATION_SINK_H

#include <QObject>
#include <QString>

/*
 * The interface WelcomeNotifier depends on, which keeps it clear of the tray
 * implementation and of the widget types that come with it. TrayController in
 * ui/ is the implementation the application runs with.
 */

/**
 * @brief Displays tray notifications and reports clicks on them.
 *
 * Every notification carries informational severity: warnings and errors go to
 * the platform log through Logger, not into a toast.
 */
class NotificationSink : public QObject
{
    Q_OBJECT

public:
    /// @param parent Standard Qt parent.
    explicit NotificationSink(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Displays a notification next to the tray icon.
     * @param title Short title shown bold by the OS.
     * @param body  Multi-line body (newline-separated lines work on all OS).
     * @param msecs How long the notification stays visible.
     */
    virtual void showNotification(const QString& title,
                                  const QString& body,
                                  int msecs) = 0;

signals:
    /// Fires when the user clicks a notification, without naming which one.
    void notificationClicked();
};

#endif // NOTIFICATION_SINK_H
