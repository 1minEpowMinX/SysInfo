#include "hardware_info_platform.h"

#include <QByteArray>
#include <QString>

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDevice.h>
#include <sys/mount.h>
#include <sys/sysctl.h>

namespace sysinfo::platform {

namespace {

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

} // namespace

QString cpuModel()
{
    return sysctlString("machdep.cpu.brand_string");
}

int physicalCoreCount()
{
    return static_cast<int>(sysctlNumber("hw.physicalcpu"));
}

qint64 totalMemoryBytes()
{
    return sysctlNumber("hw.memsize");
}

void fillMemoryIdentity(Memory &memory)
{
    // Module identity has no macOS equivalent: there is no SMBIOS table, and
    // on Apple Silicon the memory is on-package with nothing to enumerate.
    Q_UNUSED(memory)
}

void fillStorage(Storage &storage)
{
    // Walks up from the IOMedia node of the root filesystem to the
    // IOBlockStorageDevice that owns it, whose "Device Characteristics"
    // dictionary carries the human-facing description of the drive.
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

} // namespace sysinfo::platform
