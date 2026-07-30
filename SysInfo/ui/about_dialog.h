#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QString>

class SettingsManager;

/**
 * @brief Shows the modal "About SysInfo" window.
 *
 * Shows the program version, build date, project author, support contact
 * and a snapshot of system details (host, user, OS, Qt, settings path).
 * The system-details block is built once during construction — values
 * are not refreshed while the dialog is open.
 *
 * SettingsManager is injected by reference so the dialog does not have
 * to duplicate the org/app strings to look up the settings file path.
 *
 * Lifetime: created on the stack by App::onAboutRequested() and shown
 * via QDialog::exec(); destroyed when exec() returns.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param settings App-wide settings store; only the settings file path
     *                 is read for display. Must outlive this dialog.
     * @param parent   Standard Qt parent.
     */
    explicit AboutDialog(SettingsManager& settings, QWidget* parent = nullptr);

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

    /// Composes the localised "system details" block (OS, host, user,
    /// settings file path) appended to the About body.
    QString buildSystemDetailsHtml() const;

    SettingsManager& m_settings;     ///< Injected, not owned.
    QLabel* backgroundLabel;          ///< Scaled background image (sysinfo_background.png).
    QLabel* aboutLabel;               ///< Foreground text label with version + system details.
};

#endif // ABOUTDIALOG_H
