#include "trayguide.h"
#include "../settings/settingsmanager.h"

#include <QMovie>
#include <QVBoxLayout>

TrayGuide::TrayGuide(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Как закрепить значок в трее"));
    setWindowFlag(Qt::WindowStaysOnTopHint);
    setModal(true);
    resize(400, 300);

    m_gifLabel = new QLabel(this);
    m_gifLabel->setAlignment(Qt::AlignCenter);
    QMovie* movie = new QMovie(":/resources/animations/Tray_Guide.gif");
    m_gifLabel->setMovie(movie);
    movie->start();

    m_textLabel = new QLabel(tr(
        "Чтобы приложение всегда было видно в области уведомлений:\n"
        "1. Откройте скрытые значки, нажав стрелку вверх рядом с системным треем.\n"
        "2️. Найдите значок SysInfo и перетащите его в видимую область панели.\n"
        ));
    m_textLabel->setWordWrap(true);
    m_textLabel->setAlignment(Qt::AlignCenter);

    m_dontShowAgain = new QCheckBox(tr("Больше не показывать"));
    m_closeButton = new QPushButton(tr("Закрыть"));

    connect(m_closeButton, &QPushButton::clicked, this, &TrayGuide::onCloseClicked);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_gifLabel);
    layout->addWidget(m_textLabel);
    layout->addWidget(m_dontShowAgain, 0, Qt::AlignCenter);
    layout->addWidget(m_closeButton, 0, Qt::AlignCenter);
    setLayout(layout);
}

void TrayGuide::onCloseClicked()
{
    if (m_dontShowAgain->isChecked())
        SettingsManager::instance().setShowTrayGuide(false);

    close();
}
