#include "hardware_info_platform.h"

/**
 * @file hardware_info_stub.cpp
 * @brief Stands in for the platform collectors where none of the three real
 *        implementations applies.
 *
 * Compiled by CMake on any host that is neither Windows, macOS nor another
 * Unix. Every collector reports "unobtainable", which the data layer already
 * treats as a valid answer, so the inventory payload simply omits the fields
 * rather than failing to build.
 */

namespace sysinfo::platform {

QString cpuModel()
{
    return {};
}

int physicalCoreCount()
{
    return 0;
}

qint64 totalMemoryBytes()
{
    return 0;
}

void fillMemoryIdentity(Memory &memory)
{
    Q_UNUSED(memory)
}

void fillStorage(Storage &storage)
{
    Q_UNUSED(storage)
}

} // namespace sysinfo::platform
