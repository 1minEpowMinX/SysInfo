#ifndef HARDWARE_INFO_PLATFORM_H
#define HARDWARE_INFO_PLATFORM_H

#include "hardware_info.h"

#include <QString>

/**
 * @brief Declares the per-platform half of the hardware collectors.
 *
 * Internal to core/sysinfo: consumers include hardware_info.h and never this
 * header. Exactly one implementation is compiled into the target, chosen by
 * CMake from the host platform — hardware_info_win.cpp, _linux.cpp, _mac.cpp,
 * or _stub.cpp where none of the three applies. Selecting the file rather than
 * branching inside one is what keeps each implementation free of preprocessor
 * directives.
 *
 * The conventions of the data layer carry over: nothing is localised, and "no
 * value" is an empty QString or 0 rather than a placeholder. The portable half
 * of each collector — the fields Qt answers on every platform — stays in
 * hardware_info.cpp and is absent here.
 */
namespace sysinfo::platform {

/// @return Marketing name of the processor, or empty if unobtainable.
QString cpuModel();

/// @return Physical (non-SMT) core count, or 0 if unobtainable.
int physicalCoreCount();

/// @return Installed physical RAM in bytes, or 0 if unobtainable.
qint64 totalMemoryBytes();

/**
 * @brief Fills the module identity of @p memory: manufacturer, model and type.
 *
 * Leaves a field as the caller initialised it when the platform cannot answer
 * for it.
 *
 * @param memory Destination; totalBytes is not read or written.
 */
void fillMemoryIdentity(Memory &memory);

/**
 * @brief Fills @p storage from the drive the OS booted from.
 *
 * Leaves a field as the caller initialised it when the platform cannot answer
 * for it.
 *
 * @param storage Destination for the media type, vendor, model and capacity.
 */
void fillStorage(Storage &storage);

} // namespace sysinfo::platform

#endif // HARDWARE_INFO_PLATFORM_H
