#ifndef TRAYGUIDE_H
#define TRAYGUIDE_H

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class SettingsManager;

/**
 * @brief Windows-only dialog that explains how to pin the tray icon.
 *
 * Triggered when the user clicks the onboarding tray-guide notification
 * (see WelcomeNotifier). Shows a short animated screenshot demonstrating
 * the drag-from-overflow gesture, plus a "Don't show again" checkbox that
 * persists the choice via SettingsManager.
 *
 * The dialog is modal and self-disposing: App creates it with
 * Qt::WA_DeleteOnClose, so it deletes itself once the user closes it.
 */
class TrayGuide : public QDialog {
    Q_OBJECT
public:
    /**
     * @param settings Reference to the app-wide settings store; used to
     *                 persist the "Don't show again" choice. Must outlive
     *                 the dialog.
     * @param parent   Standard Qt parent.
     */
    explicit TrayGuide(SettingsManager& settings, QWidget* parent = nullptr);

private slots:
    /// Handle the Close button — saves the "don't show again" flag and closes.
    void onCloseClicked();

private:
    /// Configure window-level flags (title, modality, stays-on-top).
    void setupWindow();

    /// Build the looping screenshot animation; logs UiResourceMissing if the
    /// .gif resource is unavailable but does not abort construction.
    void setupAnimation();

    /// Build the explanatory text label.
    void setupTexts();

    /// Build the "Don't show again" checkbox and the Close button, plus the
    /// signal/slot connection that makes the button work.
    void setupControls();

    /// Lay out all four widgets vertically in this dialog.
    void buildLayout();

    SettingsManager& m_settings;     ///< Injected settings store (not owned).

    QLabel*      m_gifLabel;         ///< Holds the animated screenshot.
    QLabel*      m_textLabel;        ///< Multi-line explanatory text.
    QCheckBox*   m_dontShowAgain;    ///< Persists "Show this hint again?" preference.
    QPushButton* m_closeButton;      ///< Close action.
};

#endif // TRAYGUIDE_H
