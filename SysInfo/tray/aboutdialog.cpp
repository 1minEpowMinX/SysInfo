#include "aboutdialog.h"
#include "../utils/utils.h"

#include <QDialog>
#include <QLabel>
#include <QObject>
#include <QPixmap>
#include <QSettings>
#include <QSysInfo>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent),
    backgroundLabel(new QLabel(this)),
    aboutLabel(new QLabel(this))
{
    setWindowTitle(QObject::tr("About"));

    QPixmap bg(":/resources/icons/SysInfo_Background.png");
    backgroundLabel->setPixmap(bg);
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(this->rect());
    backgroundLabel->lower(); // Send to background

    aboutLabel->adjustSize();
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText); // Enable HTML
    aboutLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    aboutLabel->setStyleSheet("font-size: 10pt; font-family: sans-serif;"); // Use sans-serif family for better compatibility
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutLabel->setOpenExternalLinks(true);

    QString aboutText = QObject::tr(
        "<h2><b>SysInfo</b></h2>"
        "<p><b>Version:</b> %1 (build %2)</p>" // TODO: automate build creation
        "<p><b>Developer:</b> Kirill Bitskyi </p>"
        "<p><b>Company:</b> Pivdenny </p>"
        "<p><b>Source code:</b> <a href='https://github.com/1minEpowMinX/SysInfo'>GitHub</a></p>"
        "<p><b>Core:</b> Qt %3, C++17</p>"
        "<p><b>License:</b> <a href='https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE'>LGPL v3.0</a></p>"
        "<hr>"
        "<p><small>© 2025 Kirill Bitskyi for Pivdenny. All rights reserved.</small></p>"
        ).arg(PROJECT_VERSION, BUILD_DATETIME, QT_VERSION_STR);

    QString detailsLabel = getSystemDetails();

    aboutLabel->setText(aboutText + detailsLabel);
}

void AboutDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    backgroundLabel->setGeometry(this->rect());

    // Width of the text area (e.g., 50% of the width)
    int textWidth = width() * 0.50;
    int rightMargin = 30; // Small indent from the edge of the window

    // Align slightly to the left, not flush.
    aboutLabel->setGeometry(width() - textWidth - rightMargin, 0, textWidth, height());
}

QString AboutDialog::getSystemDetails() const
{
    QString osName = QSysInfo::prettyProductName();
    QString host = Utils::getHostname();
    QString user = Utils::getUsername();

    QString settingsPath;
    QSettings settings("Pivdenny", "SysInfo");
    settingsPath = settings.fileName();

    return QObject::tr(
               "<p span style='color: gray;'><b>OS:</b> %1<br>"
               "<b>User:</b> %2<br>"
               "<b>Device:</b> %3<br>"
               "<b>Settings file:</b> %4</p>")
        .arg(osName, user, host, settingsPath);
}
