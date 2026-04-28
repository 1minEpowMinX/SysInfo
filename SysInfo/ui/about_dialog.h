#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QString>

/**
 * @brief Modal "About SysInfo" window.
 *
 * Shows the program version, build date, project author, support contact
 * and a snapshot of system details (host, user, OS, Qt, settings path).
 * The system-details block is built once in getSystemDetails() during
 * construction — values are not refreshed while the dialog is open.
 *
 * Lifetime: created on the stack by App::onAboutRequested() and shown
 * via QDialog::exec(); destroyed when exec() returns.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

protected:
    /// Repaint the background label scaled to the new dialog size.
    void resizeEvent(QResizeEvent *event) override;

    /// Build the localised, multi-line "system details" block displayed
    /// in the dialog body. Pulls fresh values from Utils + QSysInfo.
    QString getSystemDetails() const;

private:
    QLabel *backgroundLabel;   ///< Scaled background image (sysinfo_background.png).
    QLabel *aboutLabel;        ///< Foreground text label with version + system details.
};

#endif // ABOUTDIALOG_H
