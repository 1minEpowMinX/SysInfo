#ifndef DIALOG_PRESENTER_H
#define DIALOG_PRESENTER_H

#include "core/sysinfo/about_facts.h"

#include <QObject>

/*
 * The interface App opens dialogs through, so the composition layer neither
 * names a widget type nor blocks on a modal loop of its own. WidgetDialogs in
 * ui/ is the implementation the application runs with.
 *
 * Whether a window is modal, and how long it stays up, is left to the
 * implementation; a caller gets no say and no return value.
 */

/**
 * @brief Opens the application's windows and reports what the user did in them.
 */
class DialogPresenter : public QObject
{
    Q_OBJECT
public:
    /// @param parent Standard Qt parent.
    explicit DialogPresenter(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief Opens the "About SysInfo" window.
     * @param facts Machine-describing values to display, as
     *              sysinfo::presenter::toAboutFacts() collects them. Read once
     *              when the window opens and not refreshed while it is up.
     */
    virtual void showAbout(const sysinfo::AboutFacts& facts) = 0;

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
