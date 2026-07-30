#include "about_dialog.h"

#include "core/logging/logger.h"

#include <QPixmap>
#include <QResizeEvent>

AboutDialog::AboutDialog(const QString& systemDetailsHtml, QWidget* parent)
    : QDialog(parent)
    , backgroundLabel(new QLabel(this))
    , aboutLabel(new QLabel(this))
{
    setWindowTitle(tr("About SysInfo"));
    resize(650, 650);

    setupBackground();
    setupAboutLabel();

    aboutLabel->setText(buildAboutHtml() + systemDetailsHtml);
    aboutLabel->adjustSize();
}

void AboutDialog::setupBackground()
{
    QPixmap bg(":/resources/icons/sysinfo_background.png");
    if (bg.isNull()) {
        Logger::log(Logger::EventId::UiResourceMissing,
                    "AboutDialog: background image not loaded.");
    }

    backgroundLabel->setPixmap(bg);
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(this->rect());
    backgroundLabel->lower(); // Send to background
}

void AboutDialog::setupAboutLabel()
{
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText);                                          // Enable HTML
    aboutLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    aboutLabel->setStyleSheet("font-size: 10pt; font-family: 'Segoe UI', sans-serif"); // Use sans-serif family for better compatibility
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutLabel->setOpenExternalLinks(true);
}

QString AboutDialog::buildAboutHtml() const
{
    return tr(
               "<h2><b>SysInfo</b></h2>"
               "<p><b>The application collects system information and assists in diagnostics:</b><br>"
               "• Displays device information (device name, user, IP address, uptime).<br>"
               "• Runs in the background via the system tray.<br>"
               "• Right-click on the icon to open the action menu.<br>"
               "• The ”Copy to clipboard“ option copies the current information.<br><br>"
               "<b>If the icon is not visible in the tray, drag it to the notification area.</b></p>"
               "<hr>"
               "<p><b>Version:</b> %1 (build %2)</p>"
               "<p><b>Developer:</b> Kyrylo Bitskyi </p>"
               "<p><b>Company:</b> Pivdenny </p>"
               "<p><b>Source code:</b> <a href='https://github.com/1minEpowMinX/SysInfo'>GitHub</a></p>"
               "<p><b>Core:</b> Qt %3, C++17</p>"
               "<p><b>License:</b> <a href='https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE'>GPL-3.0</a></p>"
               "<hr>"
               "<p><small>© 2026 Kyrylo Bitskyi for Pivdenny. All rights reserved.</small></p>"
               ).arg(PROJECT_VERSION, BUILD_DATE, QT_VERSION_STR);
}

void AboutDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    backgroundLabel->setGeometry(this->rect());

    // Width of the text area (e.g., 50% of the width)
    const int textWidth   = width() * 0.50;
    const int rightMargin = 30;       // Small indent from the edge of the window

    // Align slightly to the left, not flush.
    aboutLabel->setGeometry(width() - textWidth - rightMargin, 0, textWidth, height());
}
