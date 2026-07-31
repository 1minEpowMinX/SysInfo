#ifndef DIALOG_PRESENTER_H
#define DIALOG_PRESENTER_H

#include <QObject>
#include <QString>

/**
 * @brief Opens the application's windows and reports what the user did in them.
 *
 * The interface App opens dialogs through, so the composition layer neither
 * names a widget type nor blocks on a modal loop of its own. WidgetDialogs in
 * ui/ is the implementation the application runs with.
 *
 * Whether a window is modal, and how long it stays up, is left to the
 * implementation; a caller gets no say and no return value.
 */
class DialogPresenter : public QObject
{
    Q_OBJECT
public:
    /// @param parent Standard Qt parent.
    explicit DialogPresenter(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Opens the "About SysInfo" window.
     * @param systemDetailsHtml Rich-text block describing the machine, as
     *                          sysinfo::presenter::toSystemDetailsHtml()
     *                          renders it. Shown as handed over and not
     *                          refreshed while the window is up.
     */
    virtual void showAbout(const QString& systemDetailsHtml) = 0;

    /**
     * @brief Opens the guide on pinning the tray icon.
     *
     * Emits trayGuideDismissedForGood() if the user asks not to see it again.
     */
    virtual void showTrayGuide() = 0;

signals:
    /// Fires when the user retires the tray guide from the guide itself.
    void trayGuideDismissedForGood();
};

#endif // DIALOG_PRESENTER_H
