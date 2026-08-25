#include "hardware_info_platform.h"

#include "smbios_memory.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QStorageInfo>
#include <QString>
#include <QStringList>

#include <unistd.h>

namespace sysinfo::platform {

namespace {

/// @return Whole contents of @p path, or empty if it cannot be read.
QString readSysFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

/// @return Text after the first ':' in @p line, trimmed.
QString valueAfterColon(const QString &line)
{
    const int colon = line.indexOf(QLatin1Char(':'));
    return colon < 0 ? QString() : line.mid(colon + 1).trimmed();
}

/// @return Name of the whole disk backing @p name, which may be a partition.
QString baseBlockDevice(const QString &name)
{
    const QString sysPath = QStringLiteral("/sys/class/block/") + name;
    if (QFile::exists(sysPath + QStringLiteral("/queue"))) {
        return name; // Already a whole disk.
    }

    // A partition sits one directory below its disk in the device tree, so
    // resolving the symlink and taking the parent yields the disk.
    const QString resolved = QFileInfo(sysPath).canonicalFilePath();
    if (resolved.isEmpty()) {
        return name;
    }
    const QString parent = QFileInfo(resolved).dir().dirName();
    return parent.isEmpty() ? name : parent;
}

/// @return Block-device name of the root filesystem ("nvme0n1p2"), or empty.
QString rootBlockDevice()
{
    const QByteArray device = QStorageInfo::root().device();
    return device.startsWith("/dev/") ? QString::fromUtf8(device.mid(5)) : QString();
}

/// Fills vendor, model and capacity of @p disk from its /sys/class/block entry.
void storageIdentity(const QString &disk, Storage &storage)
{
    const QString base = QStringLiteral("/sys/class/block/") + disk;

    storage.vendor = readSysFile(base + QStringLiteral("/device/vendor"));
    storage.model = readSysFile(base + QStringLiteral("/device/model"));

    // "size" is expressed in 512-byte sectors regardless of the real sector size.
    bool parsed = false;
    const qint64 sectors = readSysFile(base + QStringLiteral("/size")).toLongLong(&parsed);
    if (parsed && sectors > 0) {
        storage.diskCapacityBytes = sectors * 512;
    }
}

/// @return "nvme", "ssd", "hdd", or empty if @p disk exposes no rotational flag.
QString diskType(const QString &disk)
{
    if (disk.startsWith(QLatin1String("nvme"))) {
        return QStringLiteral("nvme");
    }

    const QString rotational =
        readSysFile(QStringLiteral("/sys/class/block/") + disk +
                    QStringLiteral("/queue/rotational"));
    if (rotational.isEmpty()) {
        return {};
    }
    return rotational == QLatin1String("1") ? QStringLiteral("hdd")
                                            : QStringLiteral("ssd");
}

} // namespace

QString cpuModel()
{
    const QStringList lines =
        readSysFile(QStringLiteral("/proc/cpuinfo")).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("model name"))) {
            return valueAfterColon(line);
        }
    }
    return {};
}

int physicalCoreCount()
{
    // A physical core is identified by its (socket, core) pair; counting
    // distinct pairs collapses SMT siblings onto one core. Kernels that omit
    // these keys (common on ARM) yield 0.
    const QStringList lines =
        readSysFile(QStringLiteral("/proc/cpuinfo")).split(QLatin1Char('\n'));

    QSet<QString> cores;
    QString packageId;
    for (const QString &line : lines) {
        if (line.startsWith(QLatin1String("physical id"))) {
            packageId = valueAfterColon(line);
        } else if (line.startsWith(QLatin1String("core id"))) {
            cores.insert(packageId + QLatin1Char(':') + valueAfterColon(line));
        }
    }
    return static_cast<int>(cores.size());
}

qint64 totalMemoryBytes()
{
    // sysconf rather than sysinfo(2): <sys/sysinfo.h> declares both a struct and
    // a function named sysinfo in the global namespace, and this namespace
    // already holds that name there, so the header cannot be included anywhere
    // it is visible. No qualification helps — the clash is between two
    // declarations, not between two uses.
    const long pages = sysconf(_SC_PHYS_PAGES);
    const long pageSize = sysconf(_SC_PAGESIZE);
    if (pages <= 0 || pageSize <= 0) {
        return 0;
    }
    return qint64(pages) * qint64(pageSize);
}

void fillMemoryIdentity(Memory &memory)
{
    // Readable only by root; a normal user silently gets no identity fields.
    QFile dmi(QStringLiteral("/sys/firmware/dmi/tables/DMI"));
    if (dmi.open(QIODevice::ReadOnly)) {
        smbios::parseMemory(dmi.readAll(), memory);
    }
}

void fillStorage(Storage &storage)
{
    const QString rootDevice = rootBlockDevice();
    if (rootDevice.isEmpty()) {
        return;
    }

    const QString disk = baseBlockDevice(rootDevice);
    storage.systemDiskType = diskType(disk);
    storageIdentity(disk, storage);
}

} // namespace sysinfo::platform
