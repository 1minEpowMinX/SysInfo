#include "hardware_info_platform.h"

#include "smbios_memory.h"

#include <QByteArray>
#include <QSettings>
#include <QString>

#include <windows.h>
#include <winioctl.h>

namespace sysinfo::platform {

namespace {

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
QString diskType(HANDLE disk)
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
void diskIdentity(HANDLE disk, Storage &storage)
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
qint64 diskCapacity(HANDLE disk)
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

/// @return The raw SMBIOS table, or empty if the firmware would not hand it over.
QByteArray smbiosTable()
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

} // namespace

QString cpuModel()
{
    // The processor name is only exposed through the registry — there is no
    // Win32 API that returns it.
    const QSettings cpuKey(
        QStringLiteral("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"),
        QSettings::NativeFormat);
    return cpuKey.value(QStringLiteral("ProcessorNameString")).toString().trimmed();
}

int physicalCoreCount()
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

qint64 totalMemoryBytes()
{
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) {
        return 0;
    }
    return static_cast<qint64>(status.ullTotalPhys);
}

void fillMemoryIdentity(Memory &memory)
{
    smbios::parseMemory(smbiosTable(), memory);
}

void fillStorage(Storage &storage)
{
    const int driveNumber = systemPhysicalDriveNumber();
    if (driveNumber < 0) {
        return;
    }

    const HANDLE disk =
        openDevice(QStringLiteral("\\\\.\\PhysicalDrive%1").arg(driveNumber));
    if (disk == INVALID_HANDLE_VALUE) {
        return;
    }

    storage.systemDiskType = diskType(disk);
    storage.diskCapacityBytes = diskCapacity(disk);
    diskIdentity(disk, storage);
    CloseHandle(disk);
}

} // namespace sysinfo::platform
