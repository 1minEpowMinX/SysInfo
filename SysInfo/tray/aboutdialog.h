#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QString>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
    QString getSystemDetails() const;

private:
    QLabel *backgroundLabel;
    QLabel *aboutLabel;
};
#endif // ABOUTDIALOG_H
