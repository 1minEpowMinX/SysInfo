#include "device_inventory.h"

#include "hardware_info.h"
#include "system_info.h"

#include <QString>
#include <QSysInfo>

#include <cmath>

namespace sysinfo::inventory {

// Every key below is the field contract of an Elasticsearch index: renaming one
// silently breaks existing dashboards and saved queries.

namespace {

constexpr qint64 kGib = 1024LL * 1024 * 1024;
constexpr qint64 kGb  = 1000LL * 1000 * 1000;

/// How far a drive may sit from a standard capacity and still be snapped onto
/// it, as a fraction of that capacity.
constexpr double kCapacityTolerance = 0.12;

/**
 * @brief Inserts @p value under @p key only if the data layer produced one.
 *
 * The collectors signal "no value" with an empty string or 0 (see the Cpu /
 * Memory / Storage docs). Forwarding those verbatim would make an unknown
 * CPU look like a machine with zero cores once aggregated, so unset fields
 * are dropped from the document instead.
 */
void insertIfSet(QJsonObject &obj, const QString &key, const QString &value)
{
    if (!value.isEmpty()) {
        obj.insert(key, value);
    }
}

void insertIfSet(QJsonObject &obj, const QString &key, qint64 value)
{
    if (value > 0) {
        obj.insert(key, value);
    }
}

/// Inserts a whole group, unless every field in it was unobtainable.
void insertGroupIfSet(QJsonObject &obj, const QString &key, const QJsonObject &group)
{
    if (!group.isEmpty()) {
        obj.insert(key, group);
    }
}

} // namespace

int nominalMemoryGib(qint64 bytes)
{
    if (bytes <= 0) {
        return 0;
    }
    // Firmware reserves a slice of physical memory, so the OS reports 31.71 GiB
    // on a 32 GiB machine. Sizes that are not multiples of 2 GiB do not exist
    // in practice, so rounding up loses nothing legitimate.
    const double exact = static_cast<double>(bytes) / kGib;
    return static_cast<int>(std::ceil(exact / 2.0) * 2);
}

int nominalDiskGb(qint64 bytes)
{
    if (bytes <= 0) {
        return 0;
    }

    const double exact = static_cast<double>(bytes) / kGb;

    // A drive reports a little less than the number on the box once the
    // controller's spare area is deducted, so the two never compare equal.

    // Specification decimal sizes only
    static const int kStandardSizes[] = {
        16, 32, 64, 120, 128, 240, 250, 256, 480, 500, 512,
        960, 1000, 2000, 4000, 8000, 16000};

    // Nearest, not first-within-tolerance size
    int best = 0;
    double bestDistance = 0.0;
    for (const int candidate : kStandardSizes) {
        const double distance = std::fabs(exact - candidate) / candidate;
        if (best == 0 || distance < bestDistance) {
            best = candidate;
            bestDistance = distance;
        }
    }

    // Too far from every known size: report the plain conversion rather
    // than forcing the drive into the wrong bucket.
    return bestDistance <= kCapacityTolerance ? best
                                              : static_cast<int>(std::llround(exact));
}

QJsonObject payload()
{
    // The document answers "what machine is this", which is what keeps usage
    // figures (free space, available memory) and partitioning (system volume
    // size) out of it: those describe a moment or an install, not the device.
    // The sizes that do appear are nominal, so identical machines aggregate
    // into one bucket.
    const Hardware hw = collectHardware();

    QJsonObject os;
    insertIfSet(os, QStringLiteral("product"), QSysInfo::prettyProductName());
    insertIfSet(os, QStringLiteral("kernel"),  QSysInfo::kernelVersion());
    insertIfSet(os, QStringLiteral("build"),   osBuild());

    QJsonObject cpu;
    insertIfSet(cpu, QStringLiteral("model"),          hw.cpu.model);
    insertIfSet(cpu, QStringLiteral("arch"),           hw.cpu.architecture);
    insertIfSet(cpu, QStringLiteral("cores_physical"), hw.cpu.physicalCores);
    insertIfSet(cpu, QStringLiteral("cores_logical"),  hw.cpu.logicalCores);

    QJsonObject memory;
    insertIfSet(memory, QStringLiteral("manufacturer"), hw.memory.manufacturer);
    insertIfSet(memory, QStringLiteral("model"),        hw.memory.model);
    insertIfSet(memory, QStringLiteral("type"),         hw.memory.type);
    insertIfSet(memory, QStringLiteral("total_gib"),    nominalMemoryGib(hw.memory.totalBytes));

    QJsonObject storage;
    insertIfSet(storage, QStringLiteral("system_disk_type"), hw.storage.systemDiskType);
    insertIfSet(storage, QStringLiteral("vendor"),           hw.storage.vendor);
    insertIfSet(storage, QStringLiteral("model"),            hw.storage.model);
    insertIfSet(storage, QStringLiteral("disk_capacity_gb"),
                nominalDiskGb(hw.storage.diskCapacityBytes));

    QJsonObject root;
    // An Elasticsearch date field reads this number directly; see
    // bootTimeSecs() for the mapping it needs.
    insertIfSet(root, QStringLiteral("boot_time"), bootTimeSecs());
    // Version is unconditional: it is what makes the document self-contained,
    // so a query can filter by build without joining against the AppStart event.
    root.insert(QStringLiteral("version"), QString::fromUtf8(PROJECT_VERSION));
    insertGroupIfSet(root, QStringLiteral("os"),      os);
    insertGroupIfSet(root, QStringLiteral("cpu"),     cpu);
    insertGroupIfSet(root, QStringLiteral("memory"),  memory);
    insertGroupIfSet(root, QStringLiteral("storage"), storage);

    return root;
}

} // namespace sysinfo::inventory
