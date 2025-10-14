#include "aboutdialog.h"
#include "../utils/utils.h"

#include <QDialog>
#include <QObject>
#include <QLabel>
#include <QPixmap>
#include <QSettings>
#include <QSysInfo>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent),
    backgroundLabel(new QLabel(this)),
    aboutLabel(new QLabel(this))
{
    setWindowTitle(QObject::tr("О программе"));
    resize(580, 465);

    QPixmap bg(":/resources/icons/SysInfo_Background.png");
    backgroundLabel->setPixmap(bg);
    backgroundLabel->setScaledContents(true);
    backgroundLabel->setGeometry(this->rect());
    backgroundLabel->lower(); // Send to background

    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText); // Enable HTML
    aboutLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    aboutLabel->setStyleSheet("font-size: 10pt; font-family: 'Segoe UI';");
    aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    aboutLabel->setOpenExternalLinks(true);

    QString aboutText = QObject::tr(
        "<h2><b>SysInfo</b></h2>"
        "<p><b>Версия:</b> %1 (build %2)</p>" // TODO: automate build creation
        "<p><b>Разработчик:</b> Kirill Bitskyi </p>"
        "<p><b>Компания:</b> Pivdenny </p>"
        "<p><b>Исходный код:</b> <a href='https://github.com/1minEpowMinX/SysInfo'>GitHub</a></p>"
        "<p><b>Ядро:</b> Qt %3, C++17</p>"
        "<p><b>Лицензия:</b> <a href='https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE'>LGPL v3.0</a></p>"
        "<hr>"
        "<p><small>© 2025 Kirill Bitskyi for Pivdenny. Все права защищены.</small></p>"
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

    return QObject::tr("<p span style='color: gray;'><b>ОС:</b> %1<br>"
              "<b>Пользователь:</b> %2<br>"
              "<b>Устройство:</b> %3<br>"
              "<b>Файл настроек:</b> %4</p>")
        .arg(osName, user, host, settingsPath);
}
