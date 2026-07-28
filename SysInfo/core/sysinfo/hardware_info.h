#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <QString>

/**
 * @brief Pure data layer for the device's hardware configuration.
 *
 * Split out of system_info.h because the two answer different questions:
 * sysinfo::Info describes the running session (who, where, since when) and is
 * re-polled for the tray, whereas the structures here describe near-immutable
 * hardware and are collected once for the diagnostic log event.
 *
 * Same conventions as the rest of the data layer: nothing is localised, and
 * "no value" is an empty QString or 0 — never a placeholder. Sizes are raw
 * byte counts; rounding them into human-readable figures is the payload
 * builder's job (see device_inventory.h).
 *
 * Nothing here collects a serial number or any other per-unit identifier:
 * the fields are deliberately limited to the model of a component, which
 * describes the machine's class without fingerprinting the machine itself.
 */
namespace sysinfo {

/**
 * @brief Static CPU facts.
 */
struct Cpu {
    QString model;          ///< Marketing name ("AMD Ryzen 7 5800H"); empty if unobtainable.
    QString architecture;   ///< QSysInfo::currentCpuArchitecture() ("x86_64"); never empty in practice.
    int physicalCores = 0;  ///< Physical core count; 0 if unobtainable.
    int logicalCores  = 0;  ///< Logical (SMT) core count; 0 if unobtainable.
};

/**
 * @brief Physical memory: sizes plus the identity of the installed modules.
 *
 * The identity fields describe the first populated module, which stands in
 * for the machine as a whole — mixed-vendor configurations exist but are
 * rare enough that reporting every module would bloat the log event for no
 * analytical gain.
 *
 * They come from SMBIOS/DMI, which is not readable everywhere: Windows
 * exposes it to any process, Linux only to root, and Apple Silicon has no
 * equivalent at all. All three fields are therefore empty on some platforms.
 */
struct Memory {
    QString manufacturer;   ///< Module vendor ("Samsung"); empty if unobtainable.
    QString model;          ///< Module part number ("M471A1K43CB1-CTD"); empty if unobtainable.
    QString type;           ///< Generation ("DDR4", "LPDDR5"); empty if unobtainable.
    qint64 totalBytes = 0;  ///< Total installed physical RAM; 0 if unobtainable.
};

/**
 * @brief The drive the OS booted from.
 *
 * Describes the physical device, not the partitioning laid over it. Volume
 * sizes and free space are deliberately absent: how a drive is carved up
 * varies per install and changes over the machine's life, so neither belongs
 * in a snapshot of its specification.
 */
struct Storage {
    QString systemDiskType;        ///< "nvme", "ssd" or "hdd"; empty if undetermined.
    QString vendor;                ///< Drive vendor; often empty on NVMe, which folds it into the model.
    QString model;                 ///< Drive model ("SAMSUNG MZVL2512HCJQ-00BL7"); empty if unobtainable.
    qint64 diskCapacityBytes = 0;  ///< Whole physical drive; 0 if unobtainable.
};

/**
 * @brief Snapshot of the device's hardware configuration.
 */
struct Hardware {
    Cpu     cpu;
    Memory  memory;
    Storage storage;
};

/// @return Static CPU facts; individual fields are empty/0 when unobtainable.
Cpu cpuInfo();

/**
 * @brief Report installed memory and, where readable, the module identity.
 *
 * The size uses a plain syscall on every platform. The identity fields need
 * the SMBIOS table: Windows serves it through GetSystemFirmwareTable, Linux
 * through /sys/firmware/dmi (root only), and macOS not at all.
 */
Memory memoryInfo();

/**
 * @brief Describe the drive the OS booted from.
 *
 * Every field needs platform APIs: IOCTL_STORAGE_QUERY_PROPERTY on Windows,
 * /sys/block on Linux, and IOKit on macOS. Locating the drive behind the
 * root filesystem is itself platform-specific.
 */
Storage storageInfo();

/**
 * @brief Collect a full Hardware snapshot.
 *
 * Convenience wrapper over cpuInfo(), memoryInfo() and storageInfo(). Every
 * call queries the OS afresh — no caching at this layer.
 */
Hardware collectHardware();

} // namespace sysinfo

#endif // HARDWARE_INFO_H
