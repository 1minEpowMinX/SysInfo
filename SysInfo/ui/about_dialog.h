#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QString>

/**
 * @brief Shows the modal "About SysInfo" window.
 *
 * Shows the program version, build date, project author, support contact and
 * the system-details block handed to the constructor. Collects nothing itself:
 * the caller renders the details through
 * sysinfo::presenter::toSystemDetailsHtml(), so the values are those of the
 * moment the dialog opened and do not refresh while it is up.
 *
 * Lifetime: created on the stack by WidgetDialogs::showAbout() and shown via
 * QDialog::exec(); destroyed when exec() returns.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param systemDetailsHtml Rich-text block appended below the static body,
     *                          as produced by
     *                          sysinfo::presenter::toSystemDetailsHtml().
     * @param parent            Standard Qt parent.
     */
    explicit AboutDialog(const QString& systemDetailsHtml,
                         QWidget* parent = nullptr);

protected:
    /// Stretches the background label over the new dialog size and re-lays the
    /// text column against its right edge.
    void resizeEvent(QResizeEvent *event) override;

private:
    /// Configures the full-window scaled background image.
    void setupBackground();

    /// Configures the foreground RichText label (style, flags, alignment).
    void setupAboutLabel();

    /// Composes the static "About" body with version / author / license info.
    QString buildAboutHtml() const;

    QLabel* backgroundLabel;          ///< Scaled background image (sysinfo_background.png).
    QLabel* aboutLabel;               ///< Foreground text label with version + system details.
};

#endif // ABOUTDIALOG_H
