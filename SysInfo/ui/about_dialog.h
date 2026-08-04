#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "core/sysinfo/about_facts.h"

#include <QDialog>
#include <QList>

class QFrame;
class QLabel;
class QWidget;

/**
 * @brief Shows the modal "About SysInfo" window.
 *
 * Lays out, top to bottom: the application icon and name, what the program
 * does, the note on pinning the tray icon, the build and licence metadata, the
 * copyright line, and the machine-describing panel built from the AboutFacts
 * handed to the constructor. Collects nothing itself, so the values are those
 * of the moment the window opened and do not refresh while it is up.
 *
 * Colours follow QStyleHints::colorScheme() and are re-applied when the system
 * switches between light and dark. Font sizes are multiples of
 * QApplication::font(), so the window tracks the system font-scaling setting;
 * its width is fixed and its height follows the content, which keeps longer
 * translations from being clipped.
 *
 * Lifetime: created on the stack by WidgetDialogs::showAbout() and shown via
 * QDialog::exec(); destroyed when exec() returns.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @param facts  Machine-describing values for the bottom panel, as
     *               sysinfo::presenter::toAboutFacts() collects them.
     * @param parent Standard Qt parent.
     */
    explicit AboutDialog(const sysinfo::AboutFacts& facts,
                         QWidget* parent = nullptr);

protected:
    /// Re-applies the colours when the system colour scheme changes.
    void changeEvent(QEvent* event) override;

private:
    /// Builds the icon-and-name row.
    QWidget* buildHeader();

    /// Builds the lead-in sentence and the bulleted capability list.
    QWidget* buildCapabilities();

    /// Builds the rounded note about dragging the icon into the tray.
    QFrame* buildTrayNote();

    /// Builds the two-column build, authorship and licence grid.
    QWidget* buildMetadata();

    /// Builds the rounded panel describing the machine.
    QFrame* buildMachinePanel(const sysinfo::AboutFacts& facts);

    /// Builds one hairline separator.
    QFrame* buildSeparator();

    /// Writes the current scheme's colours into the window's style sheet and
    /// rebuilds the anchors.
    void applyColours();

    /// An anchor and the label showing it.
    struct Link
    {
        QLabel* label;  ///< Owned by the layout, not by this list.
        QString url;    ///< Target the anchor opens.
        QString text;   ///< Visible caption.
    };

    /// Anchors are re-rendered on every colour change: a style sheet does not
    /// reach the colour of an <a> element, which the rich-text engine takes
    /// from the markup instead.
    QList<Link> m_links;
};

#endif // ABOUTDIALOG_H
