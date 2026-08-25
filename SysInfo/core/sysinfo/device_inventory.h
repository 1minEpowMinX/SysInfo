#ifndef DEVICE_INVENTORY_H
#define DEVICE_INVENTORY_H

#include <QJsonObject>

/**
 * @brief Builds the payload for the DeviceInventory diagnostic log event.
 *
 * Renders hardware facts for machines and never localises: the log event is
 * consumed by a shipper (Winlogbeat on Windows, Filebeat elsewhere) and
 * aggregated in Elasticsearch. The keys it emits are the field contract of
 * that index.
 */
namespace sysinfo::inventory {

/**
 * @brief Builds the JSON payload describing this device.
 *
 * Shape (every field except "version" is omitted when unobtainable):
 * @code
 * {
 *   "boot_time": 1750000000,             // epoch seconds, map as date/epoch_second
 *   "version":   "X.Y.Z",               // PROJECT_VERSION
 *   "os":      { "product": …, "kernel": …, "build": "26200.1234" },
 *   "cpu":     { "model": …, "arch": …, "cores_physical": …, "cores_logical": … },
 *   "memory":  { "manufacturer": …, "model": …, "type": "DDR5", "total_gib": 32 },
 *   "storage": { "system_disk_type": "nvme"|"ssd"|"hdd", "vendor": …, "model": …,
 *                "disk_capacity_gb": 1000 }
 * }
 * @endcode
 *
 * Describes the machine as it was bought, at the granularity a person would
 * use to describe it. Three kinds of figure fall outside that and are absent
 * from the document: usage (free space, available memory), partitioning
 * (system volume size), and exact byte counts.
 *
 * "total_gib" and "disk_capacity_gb" are *nominal*: 32 GiB and 1000 GB where
 * the OS reports 31.71 and 1024.2. Their units follow the
 * convention of what they measure — memory in binary GiB, drive capacity in
 * the decimal GB drives are sold in — hence the differing suffixes.
 *
 * "boot_time" is a bare number in epoch seconds, as sysinfo::bootTimeSecs()
 * reports it; the human rendering of the same instant lives in
 * sysinfo::lastBootTime().
 *
 * Queries the OS on every call — intended to be called once at startup.
 */
QJsonObject payload();

/**
 * @brief Rounds installed RAM up to the capacity the modules are sold as.
 *
 * Takes @p bytes to the next multiple of 2 GiB, recovering the figure a human
 * would quote from the slightly smaller one the OS reports.
 *
 * Part of the field contract of "memory.total_gib", not an implementation
 * detail of payload().
 *
 * @return Nominal size in GiB, or 0 if @p bytes is not a positive count.
 */
int nominalMemoryGib(qint64 bytes);

/**
 * @brief Snaps a drive's capacity to the size it is marketed as.
 *
 * Picks the standard decimal-GB capacity nearest to @p bytes, the number the
 * drive is sold under rather than the slightly smaller one it reports.
 *
 * @return Nominal size in decimal GB; a plain conversion when the capacity
 *         sits further from every known size than the tolerance allows, or 0
 *         if @p bytes is not a positive count.
 */
int nominalDiskGb(qint64 bytes);

} // namespace sysinfo::inventory

#endif // DEVICE_INVENTORY_H
