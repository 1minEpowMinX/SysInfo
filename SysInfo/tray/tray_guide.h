#ifndef TRAYGUIDE_H
#define TRAYGUIDE_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class SettingsManager;

class TrayGuide : public QDialog {
    Q_OBJECT
public:
    explicit TrayGuide(SettingsManager& settings, QWidget* parent = nullptr);

private slots:
    void onCloseClicked();

private:
    SettingsManager& m_settings;
    QLabel* m_gifLabel;
    QLabel* m_textLabel;
    QCheckBox* m_dontShowAgain;
    QPushButton* m_closeButton;
};

#endif // TRAYGUIDE_H
