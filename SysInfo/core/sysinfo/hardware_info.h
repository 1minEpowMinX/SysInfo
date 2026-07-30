#ifndef HARDWARE_INFO_H
#define HARDWARE_INFO_H

#include <QString>

/**
 * @brief Collects the device's hardware configuration as pure data.
 *
 * The structures here describe near-immutable hardware and are collected once
 * for the diagnostic log event. The running session (who, where, since when),
 * re-polled for the tray, is sysinfo::Info in system_info.h.
 *
 * Same conventions as the rest of the data layer: nothing is localised, and
 * "no value" is an empty QString or 0 — never a placeholder. Sizes are raw
 * byte counts; rounding them into human-readable figures is the payload
 * builder's job (see device_inventory.h).
 *
 * Every field carries the model of a component and no per-unit identifier —
 * no serial numbers anywhere — so the data describes the machine's class
 * without fingerprinting the machine itself.
 */
namespace sysinfo {

/**
 * @brief Describes the CPU with facts that do not change at runtime.
 */
struct Cpu {
    QString model;          ///< Marketing name ("AMD Ryzen 7 5800H"); empty if unobtainable.
    QString architecture;   ///< QSysInfo::currentCpuArchitecture() ("x86_64"); never empty in practice.
    int physicalCores = 0;  ///< Physical core count; 0 if unobtainable.
    int logicalCores  = 0;  ///< Logical (SMT) core count; 0 if unobtainable.
};

/**
 * @brief Describes physical memory: sizes plus the identity of the installed modules.
 *
 * The identity fields describe the first populated module and stand in for the
 * machine as a whole; a mixed-vendor configuration is reported by that module
 * alone.
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
 * @brief Describes the drive the OS booted from.
 *
 * Covers the physical device, not the partitioning laid over it: volume sizes
 * and free space are absent, being properties of an install rather than of the
 * drive's specification.
 */
struct Storage {
    QString systemDiskType;        ///< "nvme", "ssd" or "hdd"; empty if undetermined.
    QString vendor;                ///< Drive vendor; often empty on NVMe, which folds it into the model.
    QString model;                 ///< Drive model ("SAMSUNG MZVL2512HCJQ-00BL7"); empty if unobtainable.
    qint64 diskCapacityBytes = 0;  ///< Whole physical drive; 0 if unobtainable.
};

/**
 * @brief Groups a snapshot of the device's hardware configuration.
 */
struct Hardware {
    Cpu     cpu;      ///< Processor facts, as cpuInfo() reports them.
    Memory  memory;   ///< Installed memory, as memoryInfo() reports it.
    Storage storage;  ///< Boot drive, as storageInfo() reports it.
};

/**
 * @brief Reports the CPU model, architecture and core counts.
 *
 * @return Static CPU facts; individual fields are empty/0 when unobtainable.
 */
Cpu cpuInfo();

/**
 * @brief Reports installed memory and, where readable, the module identity.
 *
 * The size uses a plain syscall on every platform. The identity fields need
 * the SMBIOS table: Windows serves it through GetSystemFirmwareTable, Linux
 * through /sys/firmware/dmi (root only), and macOS not at all.
 *
 * @return Installed memory; individual fields are empty/0 when unobtainable.
 */
Memory memoryInfo();

/**
 * @brief Queries the drive the OS booted from.
 *
 * Every field needs platform APIs: IOCTL_STORAGE_QUERY_PROPERTY on Windows,
 * /sys/block on Linux, and IOKit on macOS. Locating the drive behind the
 * root filesystem is itself platform-specific.
 *
 * @return The boot drive; individual fields are empty/0 when unobtainable.
 */
Storage storageInfo();

/**
 * @brief Collects a full Hardware snapshot.
 *
 * Convenience wrapper over cpuInfo(), memoryInfo() and storageInfo(). Every
 * call queries the OS afresh — no caching at this layer.
 *
 * @return All three groups, each carrying the conventions of its own getter.
 */
Hardware collectHardware();

} // namespace sysinfo

#endif // HARDWARE_INFO_H
