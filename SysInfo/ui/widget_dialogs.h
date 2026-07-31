#ifndef WIDGET_DIALOGS_H
#define WIDGET_DIALOGS_H

#include "core/ports/dialog_presenter.h"

/**
 * @brief Serves DialogPresenter through the QDialog subclasses in ui/.
 *
 * Holds no state: each call builds its window, and the two windows differ in
 * how they end. The About window is modal and runs its own event loop, so
 * showAbout() returns only once the user closes it. The tray guide is
 * self-disposing — it is created with Qt::WA_DeleteOnClose and showTrayGuide()
 * returns while it is still up, which is what lets the guide outlive the
 * notification click that asked for it.
 */
class WidgetDialogs : public DialogPresenter
{
    Q_OBJECT
public:
    /// @param parent Standard Qt parent.
    explicit WidgetDialogs(QObject* parent = nullptr) : DialogPresenter(parent) {}

    /// Opens the modal "About SysInfo" window, returning when it closes.
    void showAbout(const QString& systemDetailsHtml) override;

    /// Opens the tray-pinning guide, returning while it is still on screen.
    void showTrayGuide() override;
};

#endif // WIDGET_DIALOGS_H
