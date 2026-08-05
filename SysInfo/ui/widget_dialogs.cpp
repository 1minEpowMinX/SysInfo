#include "widget_dialogs.h"

#include "about_dialog.h"
#include "tray_guide.h"

void WidgetDialogs::showAbout(const sysinfo::AboutFacts& facts)
{
    AboutDialog dialog(facts);
    dialog.exec();
}

void WidgetDialogs::showTrayGuide()
{
    // Heap-allocated and self-deleting rather than run with exec(): the guide
    // has to outlive the notification click that asked for it.
    auto* guide = new TrayGuide;
    guide->setAttribute(Qt::WA_DeleteOnClose);
    connect(guide, &TrayGuide::dismissedForGood,
            this, &DialogPresenter::trayGuideDismissedForGood);
    guide->show();
}
