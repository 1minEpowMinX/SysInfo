#include "trayguide.h"
#include "../settings/settingsmanager.h"

#include <QMovie>
#include <QObject>
#include <QVBoxLayout>

TrayGuide::TrayGuide(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QObject::tr("How to pin an icon to the tray"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(true);

    m_gifLabel = new QLabel(this);
    m_gifLabel->setAlignment(Qt::AlignCenter);
    QMovie* movie = new QMovie(":/resources/animations/Tray_Guide.gif");
    m_gifLabel->setMovie(movie);
    movie->start();

    m_textLabel = new QLabel(QObject::tr(
        "To keep the app visible in the notification area:\n"
        "1. Open hidden icons by clicking the up arrow next to the system tray.\n"
        "2️. Find the SysInfo icon and drag it to the visible area of the panel.\n"
        ));
    m_textLabel->setWordWrap(true);
    m_textLabel->setAlignment(Qt::AlignCenter);

    m_dontShowAgain = new QCheckBox(QObject::tr("Don't show again"));
    m_closeButton = new QPushButton(QObject::tr("Close"));

    connect(m_closeButton, &QPushButton::clicked, this, &TrayGuide::onCloseClicked);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_gifLabel);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_dontShowAgain, 0, Qt::AlignCenter);
    layout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    setLayout(layout);

    this->adjustSize();
}

void TrayGuide::onCloseClicked()
{
    if (m_dontShowAgain->isChecked())
        SettingsManager::instance().setShowTrayGuide(false);

    close();
}
