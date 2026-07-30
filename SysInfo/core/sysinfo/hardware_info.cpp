#include "hardware_info.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QSettings>
#include <QStringList>
#include <QSysInfo>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winioctl.h>
#elif defined(Q_OS_LINUX)
#include <QStorageInfo>
#include <sys/sysinfo.h>
#elif defined(Q_OS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#endif

namespace sysinfo {

namespace {

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)

// --- SMBIOS / DMI ---
// Windows and Linux both hand out the same raw SMBIOS table, so the parser
// below is shared; only the way the bytes are obtained differs.

/**
 * @brief Reads string number @p index (1-based) from a structure's string set.
 *
 * SMBIOS stores strings in a NUL-separated block that follows the fixed part
 * of each structure. Index 0 is the spec's way of saying "no string".
 *
 * @param strings Start of the set.
 * @param end     One past its last byte — the position of the terminating
 *                double NUL, not the end of the whole table, so a structure
 *                can never hand out bytes belonging to the next one.
 * @param index   1-based position within the set.
 * @return        The string, or empty if @p index is 0 or out of range.
 */
QString smbiosString(const char *strings, const char *end, quint8 index)
{
    if (index == 0) {
        return {};
    }

    const char *cursor = strings;
    for (int i = 1; cursor < end; ++i) {
        const qsizetype length = qstrnlen(cursor, end - cursor);
        if (i == index) {
            return QString::fromLatin1(cursor, length).trimmed();
        }
        if (length == 0) {
            break; // Empty string here means the set has ended.
        }
        cursor += length + 1;
    }
    return {};
}

/// Translates the SMBIOS "Memory Type" enum into the name people use.
QString smbiosMemoryType(quint8 code)
{
    switch (code) {
    case 0x12: return QStringLiteral("DDR");
    case 0x13: return QStringLiteral("DDR2");
    case 0x14: return QStringLiteral("DDR2 FB-DIMM");
    case 0x18: return QStringLiteral("DDR3");
    case 0x1A: return QStringLiteral("DDR4");
    case 0x1B: return QStringLiteral("LPDDR");
    case 0x1C: return QStringLiteral("LPDDR2");
    case 0x1D: return QStringLiteral("LPDDR3");
    case 0x1E: return QStringLiteral("LPDDR4");
    case 0x20: return QStringLiteral("HBM");
    case 0x21: return QStringLiteral("HBM2");
    case 0x22: return QStringLiteral("DDR5");
    case 0x23: return QStringLiteral("LPDDR5");
    default:   return {}; // Includes "Other" and "Unknown" — nothing worth logging.
    }
}

/**
 * @brief Fills the identity fields of @p memory from a raw SMBIOS table.
 *
 * Walks the table looking for the first populated Memory Device (type 17)
 * structure. A slot with size 0 is an empty socket and is skipped, so the
 * answer describes a module that is actually installed.
 */
void parseSmbiosMemory(const QByteArray &table, Memory &memory)
{
    const char *const base = table.constData();
    const char *const end = base + table.size();
    const char *entry = base;

    // Field offsets within a type 17 structure, per the SMBIOS spec.
    constexpr int kSizeOffset         = 0x0C;
    constexpr int kMemoryTypeOffset   = 0x12;
    constexpr int kManufacturerOffset = 0x17;
    constexpr int kPartNumberOffset   = 0x1A;
    constexpr int kMinLength          = 0x1B;

    // Every bound below is expressed as a subtraction of two pointers into the table.
    while (end - entry >= 4) {
        const auto *fields = reinterpret_cast<const quint8 *>(entry);
        const quint8 type = fields[0];
        const quint8 length = fields[1];

        if (length < 4 || end - entry < length) {
            break; // Malformed header — stop rather than walk off the table.
        }
        if (type == 127) {
            break; // End-of-table marker.
        }

        // The string set runs from the end of the fixed part to a double NUL.
        const char *const strings = entry + length;
        const char *cursor = strings;
        while (end - cursor >= 2 && !(cursor[0] == '\0' && cursor[1] == '\0')) {
            ++cursor;
        }
        if (end - cursor < 2) {
            break; // Set never terminates — the table is truncated.
        }

        if (type == 17 && length >= kMinLength) {
            const quint16 size =
                quint16(fields[kSizeOffset] | (fields[kSizeOffset + 1] << 8));
            if (size != 0) { // Populated slot.
                memory.type = smbiosMemoryType(fields[kMemoryTypeOffset]);
                memory.manufacturer =
                    smbiosString(strings, cursor, fields[kManufacturerOffset]);
                memory.model = smbiosString(strings, cursor, fields[kPartNumberOffset]);
                return;
            }
        }

        entry = cursor + 2;
    }
}

#endif // Q_OS_WIN || Q_OS_LINUX

#ifdef Q_OS_WIN

/**
 * @brief Opens a device path ("\\.\C:", "\\.\PhysicalDrive0") for metadata queries.
 *
 * Requests neither read nor write access: the storage IOCTLs used below only
 * need the handle to exist, and asking for no access is what keeps them
 * working in a non-elevated process.
 *
 * @return Open handle, or INVALID_HANDLE_VALUE if the device cannot be opened.
 */
HANDLE openDevice(const QString &path)
{
    return CreateFileW(reinterpret_cast<const wchar_t *>(path.utf16()),
                       0,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       nullptr,
                       OPEN_EXISTING,
                       0,
                       nullptr);
}

/// @return Physical disk number backing the system volume, or -1 on failure.
int systemPhysicalDriveNumber()
{
    wchar_t windowsDir[MAX_PATH] = {};
    if (GetWindowsDirectoryW(windowsDir, MAX_PATH) == 0) {
        return -1;
    }

    // "C:\Windows" -> "\\.\C:"
    const QString volumePath =
        QStringLiteral("\\\\.\\") + QString::fromWCharArray(windowsDir).left(2);

    const HANDLE volume = openDevice(volumePath);
    if (volume == INVALID_HANDLE_VALUE) {
        return -1;
    }

    // Room for a few extents: a spanned volume reports more than one and the
    // IOCTL fails outright if the buffer cannot hold them all.
    QByteArray buffer(sizeof(VOLUME_DISK_EXTENTS) + 3 * sizeof(DISK_EXTENT), '\0');
    DWORD returned = 0;
    const bool ok = DeviceIoControl(volume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                    nullptr, 0,
                                    buffer.data(), static_cast<DWORD>(buffer.size()),
                                    &returned, nullptr);
    CloseHandle(volume);

    if (!ok) {
        return -1;
    }

    const auto *extents =
        reinterpret_cast<const VOLUME_DISK_EXTENTS *>(buffer.constData());
    if (extents->NumberOfDiskExtents == 0) {
        return -1;
    }
    return static_cast<int>(extents->Extents[0].DiskNumber);
}

/// @return "nvme", "ssd", "hdd", or empty if the media type is undetermined.
QString windowsDiskType(HANDLE disk)
{
    DWORD returned = 0;

    // NVMe is reported at the adapter level and is more specific than the
    // seek-penalty answer below, so it wins when both are available.
    STORAGE_PROPERTY_QUERY adapterQuery{};
    adapterQuery.PropertyId = StorageAdapterProperty;
    adapterQuery.QueryType = PropertyStandardQuery;
    STORAGE_ADAPTER_DESCRIPTOR adapter{};
    if (DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY,
                        &adapterQuery, sizeof(adapterQuery),
                        &adapter, sizeof(adapter), &returned, nullptr) &&
        adapter.BusType == BusTypeNvme) {
        return QStringLiteral("nvme");
    }

    // Rotating media incurs a seek penalty; solid-state media does not.
    STORAGE_PROPERTY_QUERY seekQuery{};
    seekQuery.PropertyId = StorageDeviceSeekPenaltyProperty;
    seekQuery.QueryType = PropertyStandardQuery;
    DEVICE_SEEK_PENALTY_DESCRIPTOR seek{};
    if (DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY,
                        &seekQuery, sizeof(seekQuery),
                        &seek, sizeof(seek), &returned, nullptr)) {
        return seek.IncursSeekPenalty ? QStringLiteral("hdd")
                                      : QStringLiteral("ssd");
    }
    return {};
}

/// Fills vendor and model from the drive's device descriptor.
void windowsDiskIdentity(HANDLE disk, Storage &storage)
{
    STORAGE_PROPERTY_QUERY query{};
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;

    // The descriptor is variable-length: ask for the header first to learn
    // how much room the vendor/product strings actually need.
    STORAGE_DESCRIPTOR_HEADER header{};
    DWORD returned = 0;
    if (!DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY,
                         &query, sizeof(query),
                         &header, sizeof(header), &returned, nullptr) ||
        header.Size < sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        return;
    }

    // Size comes from the storage driver. Cap it so that a bogus value cannot
    // turn a metadata query into a multi-gigabyte allocation; a real
    // descriptor is a few hundred bytes.
    constexpr DWORD kMaxDescriptorSize = 64 * 1024;
    if (header.Size > kMaxDescriptorSize) {
        return;
    }

    QByteArray buffer(static_cast<qsizetype>(header.Size), '\0');
    returned = 0;
    if (!DeviceIoControl(disk, IOCTL_STORAGE_QUERY_PROPERTY,
                         &query, sizeof(query),
                         buffer.data(), header.Size, &returned, nullptr) ||
        returned < sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        return;
    }

    const auto *descriptor =
        reinterpret_cast<const STORAGE_DEVICE_DESCRIPTOR *>(buffer.constData());

    // Offsets are relative to the start of the buffer, and 0 means absent. The
    // length is bounded by what the driver wrote: a string running to the very
    // end of the descriptor need not carry a terminating NUL.
    const auto stringAt = [&buffer, returned](DWORD offset) -> QString {
        if (offset == 0 || offset >= returned) {
            return {};
        }
        const char *const start = buffer.constData() + offset;
        const qsizetype length = qstrnlen(start, returned - offset);
        return QString::fromLatin1(start, length).trimmed();
    };

    storage.vendor = stringAt(descriptor->VendorIdOffset);
    storage.model = stringAt(descriptor->ProductIdOffset);
}

/// @return Size of the whole physical drive in bytes, or 0 on failure.
qint64 windowsDiskCapacity(HANDLE disk)
{
    // GET_DRIVE_GEOMETRY_EX rather than IOCTL_DISK_GET_LENGTH_INFO: the latter
    // is declared FILE_READ_ACCESS and fails on the zero-access handle held
    // here, so it costs the whole collector its freedom from elevation. Both
    // report the same size.
    DISK_GEOMETRY_EX geometry{};
    DWORD returned = 0;
    if (!DeviceIoControl(disk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0,
                         &geometry, sizeof(geometry), &returned, nullptr)) {
        return 0;
    }
    return static_cast<qint64>(geometry.DiskSize.QuadPart);
}

/// @return Physical (non-SMT) core count, or 0 on failure.
int windowsPhysicalCores()
{
    DWORD length = 0;
    // The first call is expected to fail; it only reports the size needed.
    GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length);
    if (length == 0) {
        return 0;
    }

    QByteArray buffer(static_cast<qsizetype>(length), '\0');
    if (!GetLogicalProcessorInformationEx(
            RelationProcessorCore,
            reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()),
            &length)) {
        return 0;
    }

    // Records are variable-length: walk by each entry's own Size field. Both
    // the header read and the advance are bounded by what the call actually
    // wrote, so neither a truncated tail nor a bogus Size leaves the buffer.
    constexpr DWORD kRecordHeader =
        sizeof(LOGICAL_PROCESSOR_RELATIONSHIP) + sizeof(DWORD);

    int cores = 0;
    DWORD offset = 0;
    while (length - offset >= kRecordHeader) {
        const auto *entry =
            reinterpret_cast<const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *>(
                buffer.constData() + offset);
        if (entry->Size < kRecordHeader || entry->Size > length - offset) {
            break; // Malformed record — stop rather than loop or overrun.
        }
        ++cores;
        offset += entry->Size;
    }
    return cores;
}

/// @return The raw SMBIOS table, or empty if the firmware would not hand it over.
QByteArray windowsSmbiosTable()
{
    // 'RSMB' as a big-endian provider signature; spelled numerically because
    // the value of a multi-character literal is implementation-defined.
    constexpr DWORD kRawSmbiosProvider = 0x52534D42;

    const DWORD size = GetSystemFirmwareTable(kRawSmbiosProvider, 0, nullptr, 0);
    if (size == 0) {
        return {};
    }

    QByteArray raw(static_cast<qsizetype>(size), '\0');
    if (GetSystemFirmwareTable(kRawSmbiosProvider, 0, raw.data(), size) != size) {
        return {};
    }

    // Skip the RawSMBIOSData header that precedes the table itself:
    // calling method, version major/minor, DMI revision, then a length DWORD.
    constexpr qsizetype kHeaderSize = 8;
    if (raw.size() <= kHeaderSize) {
        return {};
    }

    // The length DWORD is the firmware's own statement of where the table ends,
    // clamped to the bytes actually returned.
    const auto *fields = reinterpret_cast<const quint8 *>(raw.constData());
    const quint32 declared = quint32(fields[4]) | (quint32(fields[5]) << 8)
                           | (quint32(fields[6]) << 16) | (quint32(fields[7]) << 24);

    const qsizetype available = raw.size() - kHeaderSize;
    const qsizetype length = declared == 0
            ? available
            : qMin(static_cast<qsizetype>(declared), available);

    return raw.mid(kHeaderSize, length);
}

#elif defined(Q_OS_LINUX)

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

/// @return The "model name" field of /proc/cpuinfo, or empty if it is absent.
QString linuxCpuModel()
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

/// @return Physical (non-SMT) core count, or 0 when /proc/cpuinfo carries no
///         topology keys.
int linuxPhysicalCores()
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

/// @return Name of the whole disk backing @p name, which may be a partition.
QString linuxBaseBlockDevice(const QString &name)
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
QString linuxRootBlockDevice()
{
    const QByteArray device = QStorageInfo::root().device();
    return device.startsWith("/dev/") ? QString::fromUtf8(device.mid(5)) : QString();
}

/// Fills vendor, model and capacity of @p disk from its /sys/class/block entry.
void linuxStorageIdentity(const QString &disk, Storage &storage)
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
QString linuxDiskType(const QString &disk)
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

#elif defined(Q_OS_MAC)

/// @return The string value of sysctl @p name, or empty if the query fails or
///         the value has zero length.
QString sysctlString(const char *name)
{
    size_t len = 0;
    if (sysctlbyname(name, nullptr, &len, nullptr, 0) != 0 || len == 0) {
        return {};
    }

    QByteArray buffer(static_cast<qsizetype>(len), '\0');
    if (sysctlbyname(name, buffer.data(), &len, nullptr, 0) != 0) {
        return {};
    }
    return QString::fromUtf8(buffer.constData());
}

/// @return The integer value of sysctl @p name, or 0 if the query fails.
qint64 sysctlNumber(const char *name)
{
    qint64 value = 0; // Zero-initialised so a 32-bit answer reads back cleanly.
    size_t len = sizeof(value);
    return sysctlbyname(name, &value, &len, nullptr, 0) == 0 ? value : 0;
}

/// @return @p value transcoded from UTF-8 and trimmed, or empty if it is null
///         or will not transcode.
QString cfStringToQString(CFStringRef value)
{
    if (value == nullptr) {
        return {};
    }

    const CFIndex length = CFStringGetLength(value);
    const CFIndex capacity = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    QByteArray buffer(static_cast<qsizetype>(capacity), '\0');
    if (!CFStringGetCString(value, buffer.data(), capacity, kCFStringEncodingUTF8)) {
        return {};
    }
    return QString::fromUtf8(buffer.constData()).trimmed();
}

/**
 * @brief Fills media type, vendor and model from the IOKit device tree.
 *
 * Walks up from the IOMedia node of the root filesystem to the
 * IOBlockStorageDevice that owns it, whose "Device Characteristics" dictionary
 * carries the human-facing description of the drive.
 */
void macStorageIdentity(Storage &storage)
{
    struct statfs fs {};
    if (statfs("/", &fs) != 0) {
        return;
    }

    QString bsdName = QString::fromUtf8(fs.f_mntfromname); // "/dev/disk1s5s1"
    if (!bsdName.startsWith(QLatin1String("/dev/"))) {
        return;
    }
    bsdName = bsdName.mid(5);

    // kIOMainPortDefault is the macOS 12+ spelling of kIOMasterPortDefault.
#if defined(MAC_OS_VERSION_12_0) && \
    MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_VERSION_12_0
    const mach_port_t port = kIOMainPortDefault;
#else
    const mach_port_t port = kIOMasterPortDefault;
#endif

    const io_service_t media = IOServiceGetMatchingService(
        port, IOBSDNameMatching(port, 0, bsdName.toUtf8().constData()));
    if (media == IO_OBJECT_NULL) {
        return;
    }

    // The characteristics live on the block-storage device, several parent
    // hops above the partition's IOMedia node — hence the recursive search.
    const auto characteristics = static_cast<CFDictionaryRef>(
        IORegistryEntrySearchCFProperty(media, kIOServicePlane,
                                        CFSTR(kIOPropertyDeviceCharacteristicsKey),
                                        kCFAllocatorDefault,
                                        kIORegistryIterateParents | kIORegistryIterateRecursively));

    if (characteristics != nullptr) {
        const auto medium = static_cast<CFStringRef>(
            CFDictionaryGetValue(characteristics, CFSTR(kIOPropertyMediumTypeKey)));
        const QString mediumType = cfStringToQString(medium);
        if (mediumType == QLatin1String(kIOPropertyMediumTypeSolidStateKey)) {
            storage.systemDiskType = QStringLiteral("ssd");
        } else if (mediumType == QLatin1String(kIOPropertyMediumTypeRotationalKey)) {
            storage.systemDiskType = QStringLiteral("hdd");
        }

        storage.vendor = cfStringToQString(static_cast<CFStringRef>(
            CFDictionaryGetValue(characteristics, CFSTR(kIOPropertyVendorNameKey))));
        storage.model = cfStringToQString(static_cast<CFStringRef>(
            CFDictionaryGetValue(characteristics, CFSTR(kIOPropertyProductNameKey))));

        CFRelease(characteristics);
    }

    // NVMe is a protocol rather than a medium, so it is reported separately
    // and is the more specific answer when present.
    const auto protocol = static_cast<CFDictionaryRef>(
        IORegistryEntrySearchCFProperty(media, kIOServicePlane,
                                        CFSTR(kIOPropertyProtocolCharacteristicsKey),
                                        kCFAllocatorDefault,
                                        kIORegistryIterateParents | kIORegistryIterateRecursively));
    if (protocol != nullptr) {
        const auto interconnect = static_cast<CFStringRef>(
            CFDictionaryGetValue(protocol, CFSTR(kIOPropertyPhysicalInterconnectTypeKey)));
        if (cfStringToQString(interconnect).contains(QLatin1String("NVMe"),
                                                     Qt::CaseInsensitive)) {
            storage.systemDiskType = QStringLiteral("nvme");
        }
        CFRelease(protocol);
    }

    IOObjectRelease(media);
}

#endif

} // namespace

Cpu cpuInfo()
{
    Cpu cpu;
    cpu.architecture = QSysInfo::currentCpuArchitecture();
    cpu.logicalCores = qMax(0, QThread::idealThreadCount());

#ifdef Q_OS_WIN
    // The processor name is only exposed through the registry — there is no
    // Win32 API that returns it.
    const QSettings cpuKey(
        QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
        QSettings::NativeFormat);
    cpu.model = cpuKey.value(QStringLiteral("ProcessorNameString")).toString().trimmed();
    cpu.physicalCores = windowsPhysicalCores();

#elif defined(Q_OS_LINUX)
    cpu.model = linuxCpuModel();
    cpu.physicalCores = linuxPhysicalCores();

#elif defined(Q_OS_MAC)
    cpu.model = sysctlString("machdep.cpu.brand_string");
    cpu.physicalCores = static_cast<int>(sysctlNumber("hw.physicalcpu"));
#endif

    return cpu;
}

Memory memoryInfo()
{
    Memory memory;

#ifdef Q_OS_WIN
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        memory.totalBytes = static_cast<qint64>(status.ullTotalPhys);
    }
    parseSmbiosMemory(windowsSmbiosTable(), memory);

#elif defined(Q_OS_LINUX)
    struct sysinfo s_info;
    if (::sysinfo(&s_info) == 0) { // Linux sys/sysinfo.h disambiguation
        memory.totalBytes = qint64(s_info.totalram) * s_info.mem_unit;
    }

    // Readable only by root; a normal user silently gets no identity fields.
    QFile dmi(QStringLiteral("/sys/firmware/dmi/tables/DMI"));
    if (dmi.open(QIODevice::ReadOnly)) {
        parseSmbiosMemory(dmi.readAll(), memory);
    }

#elif defined(Q_OS_MAC)
    memory.totalBytes = sysctlNumber("hw.memsize");
    // Module identity has no macOS equivalent: there is no SMBIOS table, and
    // on Apple Silicon the memory is on-package with nothing to enumerate.
#endif

    return memory;
}

Storage storageInfo()
{
    Storage storage;

#ifdef Q_OS_WIN
    const int driveNumber = systemPhysicalDriveNumber();
    if (driveNumber >= 0) {
        const HANDLE disk =
            openDevice(QStringLiteral("\\\\.\\PhysicalDrive%1").arg(driveNumber));
        if (disk != INVALID_HANDLE_VALUE) {
            storage.systemDiskType = windowsDiskType(disk);
            storage.diskCapacityBytes = windowsDiskCapacity(disk);
            windowsDiskIdentity(disk, storage);
            CloseHandle(disk);
        }
    }

#elif defined(Q_OS_LINUX)
    const QString rootDevice = linuxRootBlockDevice();
    if (!rootDevice.isEmpty()) {
        const QString disk = linuxBaseBlockDevice(rootDevice);
        storage.systemDiskType = linuxDiskType(disk);
        linuxStorageIdentity(disk, storage);
    }

#elif defined(Q_OS_MAC)
    macStorageIdentity(storage);
#endif

    return storage;
}

Hardware collectHardware()
{
    Hardware hw;
    hw.cpu = sysinfo::cpuInfo();
    hw.memory = sysinfo::memoryInfo();
    hw.storage = sysinfo::storageInfo();
    return hw;
}

} // namespace sysinfo
