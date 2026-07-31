#include "hardware_info.h"

#include "hardware_info_platform.h"

#include <QSysInfo>
#include <QThread>

namespace sysinfo {

Cpu cpuInfo()
{
    Cpu cpu;
    // Architecture and logical core count come from portable Qt APIs and are
    // answered the same way everywhere; the rest is platform work.
    cpu.architecture = QSysInfo::currentCpuArchitecture();
    cpu.logicalCores = qMax(0, QThread::idealThreadCount());
    cpu.model = platform::cpuModel();
    cpu.physicalCores = platform::physicalCoreCount();
    return cpu;
}

Memory memoryInfo()
{
    Memory memory;
    memory.totalBytes = platform::totalMemoryBytes();
    platform::fillMemoryIdentity(memory);
    return memory;
}

Storage storageInfo()
{
    Storage storage;
    platform::fillStorage(storage);
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
