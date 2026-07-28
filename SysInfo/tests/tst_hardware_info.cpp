#include "core/sysinfo/hardware_info.h"

#include <QTest>

class TestHardwareInfo : public QObject
{
    Q_OBJECT

private slots:
    void cpuInfo_reportsArchitectureAndLogicalCores();
    void cpuInfo_physicalCoresDoNotExceedLogical();
    void memoryInfo_reportsTotalOnSupportedPlatforms();
    void memoryInfo_typeIsAKnownGenerationOrEmpty();
    void storageInfo_typeIsKnownOrEmpty();
    void storageInfo_capacityIsPlausibleForADrive();
    void collectHardware_matchesIndividualGetters();
};

void TestHardwareInfo::cpuInfo_reportsArchitectureAndLogicalCores()
{
    const sysinfo::Cpu cpu = sysinfo::cpuInfo();

    // Both come from portable Qt APIs, so they hold on every platform.
    QVERIFY(!cpu.architecture.isEmpty());
    QVERIFY(cpu.logicalCores > 0);
}

void TestHardwareInfo::cpuInfo_physicalCoresDoNotExceedLogical()
{
    const sysinfo::Cpu cpu = sysinfo::cpuInfo();
    if (cpu.physicalCores == 0) {
        return; // Unobtainable on this kernel — allowed by the contract.
    }

    QVERIFY2(cpu.physicalCores <= cpu.logicalCores,
             qPrintable(QString("physical %1 > logical %2")
                            .arg(cpu.physicalCores).arg(cpu.logicalCores)));
}

void TestHardwareInfo::memoryInfo_reportsTotalOnSupportedPlatforms()
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    QVERIFY2(sysinfo::memoryInfo().totalBytes > 0,
             "total RAM is queryable on this platform");
#else
    QSKIP("no memory collector on this platform");
#endif
}

void TestHardwareInfo::memoryInfo_typeIsAKnownGenerationOrEmpty()
{
    const QString type = sysinfo::memoryInfo().type;
    if (type.isEmpty()) {
        return; // SMBIOS unreadable (non-root Linux, macOS) — allowed.
    }

    // Guards the SMBIOS enum mapping: a wrong offset would surface here as
    // a value that is not a memory generation at all.
    QVERIFY2(type.startsWith("DDR") || type.startsWith("LPDDR")
                 || type.startsWith("HBM"),
             qPrintable("unexpected memory type: " + type));
}

void TestHardwareInfo::storageInfo_typeIsKnownOrEmpty()
{
    const QString type = sysinfo::storageInfo().systemDiskType;

    // Empty means undetermined; otherwise it must be one of the three
    // values the inventory field contract allows.
    const bool known = type.isEmpty()
                    || type == "nvme" || type == "ssd" || type == "hdd";
    QVERIFY2(known, qPrintable("unexpected disk type: " + type));
}

void TestHardwareInfo::storageInfo_capacityIsPlausibleForADrive()
{
    const qint64 capacity = sysinfo::storageInfo().diskCapacityBytes;
    if (capacity == 0) {
        return; // Unobtainable — allowed by the contract.
    }

    // Guards against reading the wrong device or mistaking a sector count
    // for a byte count: no boot drive is smaller than 8 GB or larger than 64 TB.
    QVERIFY2(capacity > Q_INT64_C(8'000'000'000),
             qPrintable(QString("implausibly small drive: %1 bytes").arg(capacity)));
    QVERIFY2(capacity < Q_INT64_C(64'000'000'000'000),
             qPrintable(QString("implausibly large drive: %1 bytes").arg(capacity)));
}

void TestHardwareInfo::collectHardware_matchesIndividualGetters()
{
    const sysinfo::Hardware hw = sysinfo::collectHardware();

    // Sizes fluctuate between calls, so compare only the static identity.
    QCOMPARE(hw.cpu.model, sysinfo::cpuInfo().model);
    QCOMPARE(hw.memory.type, sysinfo::memoryInfo().type);
    QCOMPARE(hw.storage.systemDiskType, sysinfo::storageInfo().systemDiskType);
}

QTEST_GUILESS_MAIN(TestHardwareInfo)
#include "tst_hardware_info.moc"
