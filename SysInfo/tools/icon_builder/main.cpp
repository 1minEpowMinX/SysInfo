/**
 * @file main.cpp
 * @brief Builds the platform icon containers from the master SVG.
 *
 * Windows wants a multi-size .ico for the executable's resource, macOS wants a
 * .icns for the bundle, and freedesktop desktops want a hicolor theme tree of
 * fixed-size PNGs beside the scalable SVG. None of the three has a vector form
 * the platform reads directly, so all are derived artifacts that have to be
 * regenerated whenever the artwork changes. This tool is what regenerates them,
 * so the SVG stays the only file anyone edits.
 *
 * Usage:
 *   sysinfo_icon_builder <master.svg> <output-directory>
 *
 * Writes into the output directory:
 *   sysinfo_icon.ico                       Windows executable resource
 *   sysinfo_icon.icns                      macOS bundle
 *   hicolor/<size>x<size>/apps/sysinfo.png freedesktop icon theme
 *
 * The two containers are then read back and every entry decoded, so a file that
 * advertises images it cannot produce fails here rather than on the platform.
 */

#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QMap>
#include <QPainter>
#include <QSvgRenderer>
#include <QTextStream>

namespace
{

/// Sizes the Windows icon carries, smallest first.
constexpr int kIcoSizes[] = {16, 24, 32, 48, 64, 128, 256};

/**
 * @brief Sizes the freedesktop hicolor theme is populated with.
 *
 * The scalable SVG covers desktops that render vector icons, and these cover
 * the panels and launchers that look for a fixed-size PNG instead. 22 and 24
 * are panel sizes rather than a doubling of the sequence, and are the ones a
 * missing entry shows up in first.
 */
constexpr int kHicolorSizes[] = {16, 22, 24, 32, 48, 64, 128, 256};

/// Basename the freedesktop entry's Icon= key resolves to.
constexpr auto kThemeIconName = "sysinfo";

/**
 * @brief One entry of a macOS icon file.
 *
 * The four-character type encodes both the nominal size and the scale factor,
 * which is why several entries share a pixel size: ic11 is 16pt at @2x and
 * icp5 is 32pt at @1x, and both are 32 pixels.
 */
struct IcnsSlot
{
    const char* type;  ///< OSType naming the entry.
    int pixels;        ///< Edge of the image it holds.
};

/// The set `iconutil` produces from a complete .iconset directory.
constexpr IcnsSlot kIcnsSlots[] = {
    {"icp4", 16},    // icon_16x16
    {"ic11", 32},    // icon_16x16@2x
    {"icp5", 32},    // icon_32x32
    {"ic12", 64},    // icon_32x32@2x
    {"ic07", 128},   // icon_128x128
    {"ic13", 256},   // icon_128x128@2x
    {"ic08", 256},   // icon_256x256
    {"ic14", 512},   // icon_256x256@2x
    {"ic09", 512},   // icon_512x512
    {"ic10", 1024},  // icon_512x512@2x
};

/// Renders @p renderer into a square PNG of @p pixels a side.
QByteArray renderPng(QSvgRenderer& renderer, int pixels)
{
    QImage image(pixels, pixels, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    renderer.render(&painter, QRectF(0, 0, pixels, pixels));
    painter.end();

    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return png;
}

void appendLE16(QByteArray& out, quint16 value)
{
    out.append(char(value & 0xFF));
    out.append(char((value >> 8) & 0xFF));
}

void appendLE32(QByteArray& out, quint32 value)
{
    for (int shift = 0; shift < 32; shift += 8) {
        out.append(char((value >> shift) & 0xFF));
    }
}

void appendBE32(QByteArray& out, quint32 value)
{
    for (int shift = 24; shift >= 0; shift -= 8) {
        out.append(char((value >> shift) & 0xFF));
    }
}

/**
 * @brief Packs @p images into a Windows icon resource.
 *
 * Every entry is stored PNG-compressed, which Windows has read since Vista.
 * A size of 256 is written as 0 in the directory, the field being one byte.
 *
 * @param images PNG payloads keyed by their edge in pixels.
 * @return The bytes of an .ico file.
 */
QByteArray buildIco(const QMap<int, QByteArray>& images)
{
    const int count = int(images.size());

    QByteArray out;
    appendLE16(out, 0);              // reserved
    appendLE16(out, 1);              // type: icon
    appendLE16(out, quint16(count));

    quint32 offset = quint32(6 + 16 * count);
    for (auto it = images.cbegin(); it != images.cend(); ++it) {
        const int size = it.key();
        out.append(char(size >= 256 ? 0 : size));  // width
        out.append(char(size >= 256 ? 0 : size));  // height
        out.append(char(0));                       // palette size
        out.append(char(0));                       // reserved
        appendLE16(out, 1);                        // colour planes
        appendLE16(out, 32);                       // bits per pixel
        appendLE32(out, quint32(it.value().size()));
        appendLE32(out, offset);
        offset += quint32(it.value().size());
    }

    for (auto it = images.cbegin(); it != images.cend(); ++it) {
        out.append(it.value());
    }

    return out;
}

/**
 * @brief Packs @p images into a macOS icon file.
 *
 * @param images PNG payloads keyed by their edge in pixels; every size named by
 *               kIcnsSlots must be present.
 * @return The bytes of an .icns file.
 */
QByteArray buildIcns(const QMap<int, QByteArray>& images)
{
    QByteArray body;
    for (const IcnsSlot& slot : kIcnsSlots) {
        const QByteArray& png = images.value(slot.pixels);
        body.append(slot.type, 4);
        appendBE32(body, quint32(8 + png.size()));
        body.append(png);
    }

    QByteArray out;
    out.append("icns", 4);
    appendBE32(out, quint32(8 + body.size()));
    out.append(body);

    return out;
}

/// Writes @p bytes to @p path, reporting failure on @p err.
bool writeFile(const QString& path, const QByteArray& bytes, QTextStream& err)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        err << "cannot write " << path << ": " << file.errorString() << Qt::endl;
        return false;
    }
    file.write(bytes);

    return true;
}

/**
 * @brief Decodes @p path through Qt's own handler and reports what came out.
 *
 * Decodes rather than reading the declared size: a container can advertise
 * entries whose payloads do not parse, and only decoding tells the two apart.
 *
 * @return false if any entry failed to decode.
 */
bool report(const QString& path, QTextStream& out, QTextStream& err)
{
    QImageReader reader(path);
    QStringList sizes;
    bool ok = true;

    do {
        const QImage image = reader.read();
        if (image.isNull()) {
            err << "  " << QFileInfo(path).fileName() << ": entry "
                << sizes.size() << " did not decode: " << reader.errorString()
                << Qt::endl;
            ok = false;
            continue;
        }
        sizes << QStringLiteral("%1").arg(image.width());
    } while (reader.jumpToNextImage());

    out << "  " << QFileInfo(path).fileName() << ": " << sizes.size()
        << " images (" << sizes.join(QStringLiteral(", ")) << ")" << Qt::endl;

    return ok;
}

} // namespace

int main(int argc, char* argv[])
{
    // QCoreApplication, not QGuiApplication: painting into a QImage goes
    // through the raster engine, which needs no platform plugin, and the tool
    // must run on build machines with no display and no offscreen plugin
    // deployed.
    QCoreApplication app(argc, argv);

    QTextStream out(stdout);
    QTextStream err(stderr);

    const QStringList args = QCoreApplication::arguments();
    if (args.size() != 3) {
        err << "usage: " << args.value(0) << " <master.svg> <output-directory>"
            << Qt::endl;
        return 2;
    }

    QSvgRenderer renderer(args.at(1));
    if (!renderer.isValid()) {
        err << "not a readable SVG: " << args.at(1) << Qt::endl;
        return 1;
    }

    // The three platforms ask for overlapping sizes, and several icns entries
    // share one, so each size is rasterised once and handed out from here.
    QMap<int, QByteArray> rendered;
    const auto pngFor = [&](int size) {
        auto it = rendered.find(size);
        if (it == rendered.end()) {
            it = rendered.insert(size, renderPng(renderer, size));
        }
        return it.value();
    };

    QMap<int, QByteArray> icoImages;
    for (int size : kIcoSizes) {
        icoImages.insert(size, pngFor(size));
    }

    QMap<int, QByteArray> icnsImages;
    for (const IcnsSlot& slot : kIcnsSlots) {
        icnsImages.insert(slot.pixels, pngFor(slot.pixels));
    }

    const QDir outputDir(args.at(2));
    const QString icoPath = outputDir.filePath(QStringLiteral("sysinfo_icon.ico"));
    const QString icnsPath = outputDir.filePath(QStringLiteral("sysinfo_icon.icns"));

    if (!writeFile(icoPath, buildIco(icoImages), err)
        || !writeFile(icnsPath, buildIcns(icnsImages), err)) {
        return 1;
    }

    // The freedesktop theme is a directory tree rather than a container, laid
    // out here exactly as it installs so that packaging copies it verbatim.
    QStringList hicolorWritten;
    for (int size : kHicolorSizes) {
        const QString relative = QStringLiteral("hicolor/%1x%1/apps").arg(size);
        if (!outputDir.mkpath(relative)) {
            err << "cannot create " << outputDir.filePath(relative) << Qt::endl;
            return 1;
        }

        const QString path = outputDir.filePath(
            QStringLiteral("%1/%2.png").arg(relative, QLatin1String(kThemeIconName)));
        if (!writeFile(path, pngFor(size), err)) {
            return 1;
        }
        hicolorWritten << QStringLiteral("%1").arg(size);
    }

    out << "wrote from " << args.at(1) << ":" << Qt::endl;
    const bool icoOk = report(icoPath, out, err);
    const bool icnsOk = report(icnsPath, out, err);
    out << "  hicolor/: " << hicolorWritten.size() << " images ("
        << hicolorWritten.join(QStringLiteral(", ")) << ")" << Qt::endl;

    return icoOk && icnsOk ? 0 : 1;
}
