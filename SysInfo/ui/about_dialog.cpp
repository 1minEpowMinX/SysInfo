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

/// Fixed content width, in device-independent pixels.
constexpr int kDialogWidth = 650;

/// Edge of the square the application icon is rendered into.
constexpr int kIconSize = 72;

constexpr int kMarginLeft   = 36;
constexpr int kMarginTop    = 34;
constexpr int kMarginRight  = 36;
constexpr int kMarginBottom = 30;

/// Corner radius shared by the tray note and the machine panel.
constexpr int kPanelRadius = 7;

/// Inset between the two key/value pairs of the metadata grid, on top of the
/// grid's own horizontal spacing.
constexpr int kMetadataPairGap = 22;

// Font sizes, as multiples of the application font. The reference design was
// drawn against Segoe UI 9 pt — 12 px at 96 dpi — and every size in it is a
// near-exact multiple of that, so expressing them as ratios reproduces the
// design while still tracking the system font-scaling setting.
constexpr qreal kTitleRatio     = 2.5;
constexpr qreal kIntroRatio     = 1.125;
constexpr qreal kBodyRatio      = 1.1;
constexpr qreal kNoteRatio      = 1.083;
constexpr qreal kMetadataRatio  = 1.017;
constexpr qreal kPanelRatio     = 0.958;
constexpr qreal kFootnoteRatio  = 0.917;

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

    // Base sits a step away from Window in both schemes — lighter under a dark
    // theme, brighter under a light one — which is exactly the raised panel.
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

/// Returns the application font at @p ratio of its normal size.
QFont uiFont(qreal ratio, QFont::Weight weight = QFont::Normal)
{
    QFont font = QApplication::font();
    font.setPointSizeF(basePointSize() * ratio);
    font.setWeight(weight);

    return font;
}

/// Returns a fixed-pitch font at @p ratio of the application font's size.
QFont monoFont(qreal ratio)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    // The system fixed-pitch font stays as the last resort behind the two
    // families that ship with the platform.
    font.setFamilies({QStringLiteral("Cascadia Mono"),
                      QStringLiteral("Consolas"),
                      font.family()});
    font.setPointSizeF(basePointSize() * ratio);

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
    root->setContentsMargins(kMarginLeft, kMarginTop, kMarginRight, kMarginBottom);
    root->setSpacing(0);

    root->addWidget(buildHeader());
    root->addSpacing(26);
    root->addWidget(buildCapabilities());
    root->addSpacing(18);
    root->addWidget(buildTrayNote());
    root->addSpacing(22);
    root->addWidget(buildSeparator());
    root->addSpacing(18);
    root->addWidget(buildMetadata());
    root->addSpacing(18);
    root->addWidget(buildSeparator());
    root->addSpacing(14);
    root->addWidget(makeLabel(
        tr("© 2026 Kyrylo Bitskyi for Pivdenny. All rights reserved."),
        uiFont(kFootnoteRatio), QStringLiteral("footnote")));
    root->addSpacing(20);
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
    layout->setSpacing(18);

    auto* icon = new QLabel;
    icon->setFixedSize(kIconSize, kIconSize);
    icon->setPixmap(QIcon(QStringLiteral(":/resources/icons/sysinfo_app.svg"))
                        .pixmap(QSize(kIconSize, kIconSize), devicePixelRatioF()));

    QFont titleFont = uiFont(kTitleRatio, QFont::Bold);
    titleFont.setLetterSpacing(QFont::PercentageSpacing, 98.5);

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
        uiFont(kIntroRatio, QFont::Bold), QStringLiteral("intro"));
    intro->setWordWrap(true);
    layout->addWidget(intro);
    layout->addSpacing(11);

    auto* bullets = new QWidget;
    auto* grid = new QGridLayout(bullets);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(7);
    grid->setColumnStretch(1, 1);

    for (int row = 0; row < capabilities.size(); ++row) {
        grid->addWidget(makeLabel(QStringLiteral("•"), uiFont(kBodyRatio),
                                  QStringLiteral("bullet")),
                        row, 0, Qt::AlignTop);

        auto* text = makeLabel(capabilities.at(row), uiFont(kBodyRatio),
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
    layout->setContentsMargins(14, 12, 14, 12);

    auto* text = makeLabel(
        tr("If the icon is not visible in the tray, drag it to the notification area."),
        uiFont(kNoteRatio, QFont::DemiBold), QStringLiteral("note"));
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
        {tr("Core:"), tr("Qt %1, C++17").arg(QT_VERSION_STR), {}},
        {tr("Developer:"), QStringLiteral("Kyrylo Bitskyi"), {}},
        {tr("Source code:"), QStringLiteral("GitHub"), sourceUrl},
        {tr("Company:"), QStringLiteral("Pivdenny"), {}},
        {tr("License:"), QStringLiteral("GPL-3.0"), licenceUrl}};

    auto* block = new QWidget;

    auto* grid = new QGridLayout(block);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(9);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    for (int index = 0; index < entries.size(); ++index) {
        const int row = index / 2;
        const int column = (index % 2) * 2;

        const Entry& entry = entries.at(index);

        auto* key = makeLabel(entry.key, uiFont(kMetadataRatio),
                              QStringLiteral("key"));
        if (column == 2) {
            // QGridLayout spaces every column alike, so the wider gap between
            // the two pairs is carried by the second pair's key.
            key->setContentsMargins(kMetadataPairGap, 0, 0, 0);
        }
        grid->addWidget(key, row, column);

        auto* value = makeLabel(entry.value, uiFont(kMetadataRatio),
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
    grid->setContentsMargins(14, 13, 14, 13);
    grid->setHorizontalSpacing(14);
    grid->setVerticalSpacing(5);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(3, 1);

    const auto addPair = [&](int row, int column, const QString& key,
                             QLabel* value, int valueSpan) {
        grid->addWidget(makeLabel(key, uiFont(kPanelRatio),
                                  QStringLiteral("panelKey")),
                        row, column);
        value->setFont(monoFont(kPanelRatio));
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
    line->setFixedHeight(1);

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
