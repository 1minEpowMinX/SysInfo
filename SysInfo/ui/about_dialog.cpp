#include "about_dialog.h"

#include <QApplication>
#include <QEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QResizeEvent>
#include <QStyleHints>
#include <QVBoxLayout>

#include <utility>

namespace
{

// Every figure below is in the pixels of the reference design, type sizes
// included. The design fixes each gap on its own, so they are listed rather
// than derived from a scale, and two figures that happen to be equal are not
// necessarily the same figure.

// --- The page.

/// Fixed content width.
constexpr int kDialogWidth = 650;

/// Page margins. The horizontal pair is one figure — the content column is
/// centred — while top and bottom differ.
constexpr int kMarginH      = 36;
constexpr int kMarginTop    = 34;
constexpr int kMarginBottom = 30;

// Vertical rhythm of the root column, in the order the blocks appear. The
// metadata grid stands off the rule above it and the rule below it by the same
// figure, which is the one relationship among these.
constexpr int kGapHeaderToCapabilities = 26;
constexpr int kGapCapabilitiesToNote   = 18;
constexpr int kGapNoteToSeparator      = 22;
constexpr int kGapMetadataToSeparator  = 18;
constexpr int kGapSeparatorToFootnote  = 14;
constexpr int kGapFootnoteToPanel      = 20;

/// Height of one hairline separator.
constexpr int kSeparatorThickness = 1;

// --- The header: the application icon and the name beside it.

/// Edge of the square the application icon is rendered into.
constexpr int kIconSize = 72;

constexpr int kIconToTitleGap = 18;

/// Letter spacing of the title, as a percentage of the font's normal spacing:
/// the design sets it a touch tighter than the font ships.
constexpr qreal kTitleLetterSpacing = 98.5;

// --- The capabilities block: a lead-in sentence over a bulleted list.

constexpr int kGapIntroToBullets = 11;
constexpr int kBulletToTextGap   = 10;
constexpr int kBulletRowGap      = 7;

// --- The metadata grid, between the two rules.

constexpr int kMetadataKeyToValueGap = 10;
constexpr int kMetadataRowGap        = 9;

/// Inset between the two key/value pairs, on top of the grid's own horizontal
/// spacing.
constexpr int kMetadataPairGap = 22;

// --- The two rounded panels: the tray note and the machine panel. Radius and
// horizontal padding are shared; the vertical padding differs by a pixel
// between them, as the design has it.

constexpr int kPanelRadius          = 7;
constexpr int kPanelPaddingH        = 14;
constexpr int kTrayNotePaddingV     = 12;
constexpr int kMachinePanelPaddingV = 13;

constexpr int kMachineKeyToValueGap = 14;
constexpr int kMachineRowGap        = 5;

// --- The type scale.
//
// The reference design was drawn against Segoe UI 9 pt — 12 px at 96 dpi — and
// carries these sizes in pixels. uiFont() and monoFont() rescale each by the
// ratio of the application font to that base, so the window reproduces the
// design while still tracking the system font-scaling setting.
constexpr qreal kDesignBasePx = 12.0;

constexpr qreal kTitlePx    = 30.0;
constexpr qreal kIntroPx    = 13.5;
constexpr qreal kBodyPx     = 13.2;
constexpr qreal kNotePx     = 13.0;
constexpr qreal kMetadataPx = 12.2;
constexpr qreal kPanelPx    = 11.5;
constexpr qreal kFootnotePx = 11.0;

// --- The colours.

// Weights the text colour is mixed into the surface at, giving the window its
// three levels of emphasis and its hairlines.
constexpr qreal kSecondaryWeight = 0.78;
constexpr qreal kMutedWeight     = 0.58;
constexpr qreal kFootnoteWeight  = 0.52;
constexpr qreal kSeparatorWeight = 0.14;

/// The window's colours, resolved for one colour scheme.
struct AboutPalette
{
    QColor surface;
    QColor panel;
    QColor textPrimary;
    QColor textSecondary;
    QColor textMuted;
    QColor accent;
    QColor link;
    QColor separator;
    QColor footnote;
};

/**
 * @brief Mixes @p over into @p under at @p weight.
 *
 * Produces the opaque colour a translucent @p over would resolve to. Qt's
 * style-sheet colour parser is fed opaque values only, which keeps the result
 * independent of how a platform style fills the alpha-bearing palette roles.
 */
QColor mixed(const QColor& over, const QColor& under, qreal weight)
{
    return QColor::fromRgbF(
        over.redF() * weight + under.redF() * (1.0 - weight),
        over.greenF() * weight + under.greenF() * (1.0 - weight),
        over.blueF() * weight + under.blueF() * (1.0 - weight));
}

/**
 * @brief Derives the window's colours from the style's own palette.
 *
 * Takes the surface, the raised panel, the text and the link from @p source, so
 * the window carries whatever the platform style is currently painting the rest
 * of the application with. Only the bullet accent is fixed: it is the orange of
 * the application icon, and belongs to the product rather than to the theme.
 *
 * @param source Palette of the widget being styled.
 * @param scheme Colour scheme in force, which selects the accent's tint.
 * @return Opaque colours ready for a style sheet.
 */
AboutPalette paletteFrom(const QPalette& source, Qt::ColorScheme scheme)
{
    const QColor surface = source.color(QPalette::Window);
    const QColor text = source.color(QPalette::WindowText);

    AboutPalette colours;
    colours.surface = surface;

    // Base sits a step away from Window in both schemes.
    colours.panel = source.color(QPalette::Base);

    colours.textPrimary = text;
    colours.textSecondary = mixed(text, surface, kSecondaryWeight);
    colours.textMuted = mixed(text, surface, kMutedWeight);
    colours.footnote = mixed(text, surface, kFootnoteWeight);
    colours.separator = mixed(text, surface, kSeparatorWeight);
    colours.link = source.color(QPalette::Link);

    colours.accent = scheme == Qt::ColorScheme::Dark ? QColor(0xF0, 0xA3, 0x44)
                                                     : QColor(0xC4, 0x68, 0x0A);

    return colours;
}

/// Returns the application font's size in points, resolving it through
/// QFontInfo when the font carries a pixel size instead.
qreal basePointSize()
{
    const QFont base = QApplication::font();
    const qreal points = base.pointSizeF();

    return points > 0 ? points : QFontInfo(base).pointSizeF();
}

/// Returns the application font carrying @p designPx, the size the reference
/// design gives the text, rescaled from kDesignBasePx to the application font.
QFont uiFont(qreal designPx, QFont::Weight weight = QFont::Normal)
{
    QFont font = QApplication::font();
    font.setPointSizeF(basePointSize() * designPx / kDesignBasePx);
    font.setWeight(weight);

    return font;
}

/// Returns a fixed-pitch font carrying @p designPx, scaled the same way as
/// uiFont().
QFont monoFont(qreal designPx)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    // The system fixed-pitch font stays as the last resort behind the two
    // families that ship with the platform.
    font.setFamilies({QStringLiteral("Cascadia Mono"),
                      QStringLiteral("Consolas"),
                      font.family()});
    font.setPointSizeF(basePointSize() * designPx / kDesignBasePx);

    return font;
}

/// Tags @p widget for the style-sheet selectors of AboutDialog::applyColours().
template <typename T>
T* tagged(T* widget, const QString& role)
{
    widget->setProperty("role", role);

    return widget;
}

/// Builds a label carrying @p text in @p font under the style-sheet role @p role.
QLabel* makeLabel(const QString& text, const QFont& font, const QString& role)
{
    auto* label = new QLabel(text);
    label->setFont(font);

    return tagged(label, role);
}

/**
 * @brief A label that shortens its text in the middle when it does not fit.
 *
 * Needs no Q_OBJECT: it declares no signals or slots and only overrides
 * painting-related virtuals.
 */
class ElidingLabel : public QLabel
{
public:
    explicit ElidingLabel(QString text)
        : m_fullText(std::move(text))
    {
        setToolTip(m_fullText);
        setText(m_fullText);
    }

    /// Reports no preferred width of its own, so the surrounding layout is free
    /// to make it narrower than its text.
    QSize minimumSizeHint() const override
    {
        return QSize(0, QLabel::minimumSizeHint().height());
    }

protected:
    void resizeEvent(QResizeEvent* event) override
    {
        QLabel::resizeEvent(event);
        QLabel::setText(fontMetrics().elidedText(m_fullText, Qt::ElideMiddle, width()));
    }

private:
    QString m_fullText;
};

} // namespace

AboutDialog::AboutDialog(const sysinfo::AboutFacts& facts, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About SysInfo"));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(kMarginH, kMarginTop, kMarginH, kMarginBottom);
    root->setSpacing(0);

    root->addWidget(buildHeader());
    root->addSpacing(kGapHeaderToCapabilities);
    root->addWidget(buildCapabilities());
    root->addSpacing(kGapCapabilitiesToNote);
    root->addWidget(buildTrayNote());
    root->addSpacing(kGapNoteToSeparator);
    root->addWidget(buildSeparator());
    root->addSpacing(kGapMetadataToSeparator);
    root->addWidget(buildMetadata());
    root->addSpacing(kGapMetadataToSeparator);
    root->addWidget(buildSeparator());
    root->addSpacing(kGapSeparatorToFootnote);
    root->addWidget(makeLabel(
        tr("© %1 Kyrylo Bitskyi for Pivdenny. All rights reserved.").arg(BUILD_YEAR),
        uiFont(kFootnotePx), QStringLiteral("footnote")));
    root->addSpacing(kGapFootnoteToPanel);
    root->addStretch(1);
    root->addWidget(buildMachinePanel(facts));

    applyColours();

    // Fixed width with the height left to the content: the Russian and
    // Ukrainian strings run longer than the English ones, and a fixed height
    // would cut the machine panel off.
    //
    // The height comes from totalHeightForWidth() rather than from
    // adjustSize(): sizeHint() answers for the width the layout would choose on
    // its own, which is narrower than kDialogWidth, so the wrapping labels
    // report more lines there than they occupy here and the window ends up
    // taller than its content.
    setFixedWidth(kDialogWidth);
    root->activate();
    setFixedHeight(root->totalHeightForWidth(kDialogWidth));
}

QWidget* AboutDialog::buildHeader()
{
    auto* header = new QWidget;

    auto* layout = new QHBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kIconToTitleGap);

    auto* icon = new QLabel;
    icon->setFixedSize(kIconSize, kIconSize);
    icon->setPixmap(QIcon(QStringLiteral(":/resources/icons/sysinfo_app.svg"))
                        .pixmap(QSize(kIconSize, kIconSize), devicePixelRatioF()));

    QFont titleFont = uiFont(kTitlePx, QFont::Bold);
    titleFont.setLetterSpacing(QFont::PercentageSpacing, kTitleLetterSpacing);

    layout->addWidget(icon);
    layout->addWidget(makeLabel(QStringLiteral("SysInfo"), titleFont,
                                QStringLiteral("title")),
                      0, Qt::AlignVCenter);
    layout->addStretch(1);

    return header;
}

QWidget* AboutDialog::buildCapabilities()
{
    const QStringList capabilities{
        tr("Displays device information (device name, user, IP address, uptime)."),
        tr("Runs in the background via the system tray."),
        tr("Right-click on the icon to open the action menu."),
        tr("The ”Copy to clipboard“ option copies the current information.")};

    auto* block = new QWidget;

    auto* layout = new QVBoxLayout(block);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* intro = makeLabel(
        tr("The application collects system information and assists in diagnostics:"),
        uiFont(kIntroPx, QFont::Bold), QStringLiteral("intro"));
    intro->setWordWrap(true);
    layout->addWidget(intro);
    layout->addSpacing(kGapIntroToBullets);

    auto* bullets = new QWidget;
    auto* grid = new QGridLayout(bullets);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(kBulletToTextGap);
    grid->setVerticalSpacing(kBulletRowGap);
    grid->setColumnStretch(1, 1);

    for (int row = 0; row < capabilities.size(); ++row) {
        grid->addWidget(makeLabel(QStringLiteral("•"), uiFont(kBodyPx),
                                  QStringLiteral("bullet")),
                        row, 0, Qt::AlignTop);

        auto* text = makeLabel(capabilities.at(row), uiFont(kBodyPx),
                               QStringLiteral("body"));
        text->setWordWrap(true);
        grid->addWidget(text, row, 1);
    }

    layout->addWidget(bullets);

    return block;
}

QFrame* AboutDialog::buildTrayNote()
{
    auto* note = tagged(new QFrame, QStringLiteral("panel"));

    auto* layout = new QVBoxLayout(note);
    layout->setContentsMargins(kPanelPaddingH, kTrayNotePaddingV,
                               kPanelPaddingH, kTrayNotePaddingV);

    auto* text = makeLabel(
        tr("If the icon is not visible in the tray, drag it to the notification area."),
        uiFont(kNotePx, QFont::DemiBold), QStringLiteral("note"));
    text->setWordWrap(true);
    layout->addWidget(text);

    return note;
}

QWidget* AboutDialog::buildMetadata()
{
    const QString sourceUrl = QStringLiteral("https://github.com/1minEpowMinX/SysInfo");
    const QString licenceUrl = sourceUrl + QStringLiteral("/blob/main/LICENSE");

    struct Entry
    {
        QString key;
        QString value;
        QString url;
    };

    const QList<Entry> entries{
        {tr("Version:"), tr("%1 (build %2)").arg(PROJECT_VERSION, BUILD_DATE), {}},
        {tr("Core:"), tr("Qt %1, C++%2").arg(QT_VERSION_STR, PROJECT_CXX_STANDARD), {}},
        {tr("Developer:"), QStringLiteral("Kyrylo Bitskyi"), {}},
        {tr("Source code:"), QStringLiteral("GitHub"), sourceUrl},
        {tr("Company:"), QStringLiteral("Pivdenny"), {}},
        {tr("License:"), QStringLiteral("GPL-3.0"), licenceUrl}};

    auto* block = new QWidget;

    auto* grid = new QGridLayout(block);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(kMetadataKeyToValueGap);
    grid->setVerticalSpacing(kMetadataRowGap);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    for (int index = 0; index < entries.size(); ++index) {
        const int row = index / 2;
        const int column = (index % 2) * 2;

        const Entry& entry = entries.at(index);

        auto* key = makeLabel(entry.key, uiFont(kMetadataPx),
                              QStringLiteral("key"));
        if (column == 2) {
            // QGridLayout spaces every column alike, so the wider gap between
            // the two pairs is carried by the second pair's key.
            key->setContentsMargins(kMetadataPairGap, 0, 0, 0);
        }
        grid->addWidget(key, row, column);

        auto* value = makeLabel(entry.value, uiFont(kMetadataPx),
                                QStringLiteral("value"));
        if (!entry.url.isEmpty()) {
            value->setTextFormat(Qt::RichText);
            value->setOpenExternalLinks(true);
            value->setTextInteractionFlags(Qt::TextBrowserInteraction);
            m_links.append(Link{value, entry.url, entry.value});
        }
        grid->addWidget(value, row, column + 1);
    }

    return block;
}

QFrame* AboutDialog::buildMachinePanel(const sysinfo::AboutFacts& facts)
{
    auto* panel = tagged(new QFrame, QStringLiteral("panel"));

    auto* grid = new QGridLayout(panel);
    grid->setContentsMargins(kPanelPaddingH, kMachinePanelPaddingV,
                             kPanelPaddingH, kMachinePanelPaddingV);
    grid->setHorizontalSpacing(kMachineKeyToValueGap);
    grid->setVerticalSpacing(kMachineRowGap);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    const auto addPair = [&](int row, int column, const QString& key,
                             QLabel* value, int valueSpan) {
        grid->addWidget(makeLabel(key, uiFont(kPanelPx),
                                  QStringLiteral("panelKey")),
                        row, column);
        value->setFont(monoFont(kPanelPx));
        grid->addWidget(tagged(value, QStringLiteral("panelValue")), row,
                        column + 1, 1, valueSpan);
    };

    addPair(0, 0, tr("OS:"), new QLabel(facts.operatingSystem), 1);
    addPair(0, 2, tr("User:"), new QLabel(facts.username), 1);
    addPair(1, 0, tr("Device:"), new QLabel(facts.hostname), 1);

    // The settings location is the one value with no bound on its length — a
    // registry key on Windows, a path elsewhere. Sharing a row would leave it a
    // quarter of the panel, which is not enough to read a registry key in, so
    // it takes a row of its own and spans the remaining three columns.
    addPair(2, 0, tr("Settings:"), new ElidingLabel(facts.settingsFilePath), 3);

    return panel;
}

QFrame* AboutDialog::buildSeparator()
{
    auto* line = tagged(new QFrame, QStringLiteral("separator"));
    line->setFixedHeight(kSeparatorThickness);

    return line;
}

void AboutDialog::applyColours()
{
    // Read from the application palette rather than this widget's: a style
    // sheet set here feeds back into the widget's own palette, so a second
    // pass would derive its colours from the first pass's output.
    const AboutPalette colours =
        paletteFrom(qApp->palette(), qApp->styleHints()->colorScheme());

    setStyleSheet(QStringLiteral(
                      "QDialog { background-color: %1; }"
                      "QLabel { background: transparent; }"
                      "QLabel[role=\"title\"] { color: %3; }"
                      "QLabel[role=\"intro\"] { color: %3; }"
                      "QLabel[role=\"note\"] { color: %3; }"
                      "QLabel[role=\"value\"] { color: %3; }"
                      "QLabel[role=\"body\"] { color: %4; }"
                      "QLabel[role=\"panelValue\"] { color: %4; }"
                      "QLabel[role=\"key\"] { color: %5; }"
                      "QLabel[role=\"panelKey\"] { color: %5; }"
                      "QLabel[role=\"bullet\"] { color: %6; }"
                      "QLabel[role=\"footnote\"] { color: %7; }"
                      "QFrame[role=\"separator\"] { background-color: %8; }"
                      "QFrame[role=\"panel\"] { background-color: %2;"
                      " border-radius: %9px; }")
                      .arg(colours.surface.name(), colours.panel.name(),
                           colours.textPrimary.name(),
                           colours.textSecondary.name(),
                           colours.textMuted.name(), colours.accent.name(),
                           colours.footnote.name(), colours.separator.name())
                      .arg(kPanelRadius));

    // The anchors are rewritten rather than styled: a style sheet does not
    // reach the colour of an <a> element, which the rich-text engine takes
    // from the markup instead.
    for (const Link& link : std::as_const(m_links)) {
        link.label->setText(QStringLiteral("<a href=\"%1\" style=\"color:%2;\">%3</a>")
                                .arg(link.url, colours.link.name(), link.text));
    }
}

void AboutDialog::changeEvent(QEvent* event)
{
    QDialog::changeEvent(event);

    if (event->type() == QEvent::ThemeChange
        || event->type() == QEvent::ApplicationPaletteChange) {
        applyColours();
    }
}
