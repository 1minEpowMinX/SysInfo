#ifndef DEVICE_INVENTORY_H
#define DEVICE_INVENTORY_H

#include <QJsonObject>

/**
 * @brief Payload builder for the DeviceInventory diagnostic log event.
 *
 * Sits beside sysinfo::presenter but serves the opposite audience: the
 * presenter renders Info for humans and localises as it goes, whereas this
 * namespace renders hardware facts for machines and never localises — the
 * log event is consumed by a shipper (Winlogbeat on Windows, Filebeat
 * elsewhere) and aggregated in Elasticsearch.
 *
 * The resulting keys form the field contract of that index, so renaming one
 * silently breaks existing dashboards and saved queries.
 */
namespace sysinfo::inventory {

/**
 * @brief Build the JSON payload describing this device.
 *
 * Shape (every field except "version" is omitted when unobtainable, so that
 * Elasticsearch records "unknown" as a missing field rather than as a
 * misleading zero):
 * @code
 * {
 *   "boot_time": 1750000000,             // epoch seconds, map as date/epoch_second
 *   "version":   "3.0.0",
 *   "os":      { "product": …, "kernel": …, "build": "26200.1234" },
 *   "cpu":     { "model": …, "arch": …, "cores_physical": …, "cores_logical": … },
 *   "memory":  { "manufacturer": …, "model": …, "type": "DDR5", "total_gib": 32 },
 *   "storage": { "system_disk_type": "nvme"|"ssd"|"hdd", "vendor": …, "model": …,
 *                "disk_capacity_gb": 1000 }
 * }
 * @endcode
 *
 * Everything here describes the machine as it was bought, at the granularity
 * a person would use to describe it. Three kinds of figure are left out on
 * purpose:
 *   - usage (free space, available memory) — it would describe the moment of
 *     collection rather than the device, and differ on every boot;
 *   - partitioning (system volume size) — it varies per install rather than
 *     per machine;
 *   - exact byte counts — sizes are reported only as the capacity the
 *     component is sold with.
 *
 * So "total_gib" and "disk_capacity_gb" are *nominal*: rounded to 32 GiB and
 * 1000 GB rather than to the 31.71 and 1024.2 the OS reports, because that is
 * the value that groups identical machines together. Their units follow the
 * convention of what they measure — memory is counted in binary GiB, drive
 * capacity in the decimal GB drives are sold in — hence the differing
 * suffixes.
 *
 * "boot_time" is a bare number rather than a formatted string so a date
 * field can read it directly. It is in epoch *seconds*, so that field must be
 * mapped with "format": "epoch_second" — the default epoch_millis would place
 * every document in 1970. The human rendering of the same instant lives in
 * sysinfo::lastBootTime().
 *
 * Queries the OS on every call — intended to be called once at startup.
 */
QJsonObject payload();

/**
 * @brief Round installed RAM up to the capacity the modules are sold as.
 *
 * Firmware reserves a slice of physical memory, so the OS reports slightly
 * less than what is installed — 31.71 GiB on a 32 GiB machine. Rounding up
 * to the next multiple of 2 GiB recovers the figure a human would quote,
 * which is what makes a "group by RAM size" aggregation readable. Sizes that
 * are not multiples of 2 GiB do not exist in practice, so nothing legitimate
 * is rounded away.
 *
 * Exposed alongside payload() because how a size is rounded is part of the
 * field contract, not an implementation detail.
 *
 * @return Nominal size in GiB, or 0 if @p bytes is not a positive count.
 */
int nominalMemoryGib(qint64 bytes);

/**
 * @brief Snap a drive's capacity to the size it is marketed as.
 *
 * Drives are sold in decimal GB and report a little less once the
 * controller's spare area is deducted, so the byte count never equals the
 * number on the box. Snapping to the nearest standard capacity gives a
 * stable grouping key across drives of the same class.
 *
 * @return Nominal size in decimal GB; a plain conversion when the capacity
 *         is more than 12% away from every known size, or 0 if @p bytes is
 *         not a positive count.
 */
int nominalDiskGb(qint64 bytes);

} // namespace sysinfo::inventory

#endif // DEVICE_INVENTORY_H
