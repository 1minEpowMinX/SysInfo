#ifndef TRAYGUIDE_H
#define TRAYGUIDE_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>

class TrayGuide : public QDialog {
    Q_OBJECT
public:
    explicit TrayGuide(QWidget* parent = nullptr);

private slots:
    void onCloseClicked();

private:
    QLabel* m_gifLabel;
    QLabel* m_textLabel;
    QCheckBox* m_dontShowAgain;
    QPushButton* m_closeButton;
};

#endif // TRAYGUIDE_H
