#include "tray_guide.h"

#include "core/logging/logger.h"

#include <QMovie>
#include <QVBoxLayout>

TrayGuide::TrayGuide(QWidget* parent)
    : QDialog(parent)
{
    setupWindow();
    setupAnimation();
    setupTexts();
    setupControls();
    buildLayout();

    adjustSize();
}

void TrayGuide::setupWindow()
{
    setWindowTitle(tr("How to pin an icon to the tray"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(true);
}

void TrayGuide::setupAnimation()
{
    m_gifLabel = new QLabel(this);
    m_gifLabel->setAlignment(Qt::AlignCenter);

    QMovie* movie = new QMovie(":/resources/animations/tray_guide.gif", QByteArray(), this);
    if (!movie->isValid()) {
        Logger::log(Logger::EventId::UiResourceMissing,
                    "TrayGuide: tray_guide.gif not loaded");
    }

    m_gifLabel->setMovie(movie);
    movie->start();
}

void TrayGuide::setupTexts()
{
    m_textLabel = new QLabel(tr(
        "<p><b>To keep the app visible in the notification area:</b><br>"
        "1. Open hidden icons by clicking the up arrow next to the system tray.<br>"
        "2. Find the SysInfo icon and drag it to the visible area of the panel.</p>"
        ), this);
    m_textLabel->setWordWrap(true);
    m_textLabel->setAlignment(Qt::AlignCenter);
}

void TrayGuide::setupControls()
{
    m_dontShowAgain = new QCheckBox(tr("Don't show again"), this);
    m_closeButton   = new QPushButton(tr("Close"), this);

    connect(m_closeButton, &QPushButton::clicked, this, &TrayGuide::onCloseClicked);
}

void TrayGuide::buildLayout()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_gifLabel);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_dontShowAgain, 0, Qt::AlignCenter);
    layout->addWidget(m_closeButton,   0, Qt::AlignCenter);
    setLayout(layout);
}

void TrayGuide::onCloseClicked()
{
    if (m_dontShowAgain->isChecked()) {
        emit dismissedForGood();
    }

    close();
}
