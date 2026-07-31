#include "widget_dialogs.h"

#include "about_dialog.h"
#include "tray_guide.h"

void WidgetDialogs::showAbout(const QString& systemDetailsHtml)
{
    AboutDialog dialog(systemDetailsHtml);
    dialog.exec();
}

void WidgetDialogs::showTrayGuide()
{
    auto* guide = new TrayGuide;
    guide->setAttribute(Qt::WA_DeleteOnClose);
    connect(guide, &TrayGuide::dismissedForGood,
            this, &DialogPresenter::trayGuideDismissedForGood);
    guide->show();
}
