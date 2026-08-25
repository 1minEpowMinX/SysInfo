#include "core/sysinfo/device_inventory.h"
#include "core/sysinfo/hardware_info.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QTest>

class TestDeviceInventory : public QObject
{
    Q_OBJECT

private slots:
    void payload_alwaysCarriesVersion();
    void payload_bootTimeIsNumericEpochSeconds();
    void payload_omitsUnobtainableFieldsRatherThanZeroing();
    void payload_groupsAreObjects();
    void payload_hasMandatoryCpuAndMemoryOnSupportedPlatforms();
    void payload_osProductIsAUsableProductName();
    void payload_nominalMemorySizeIsRoundedUpFromReported();
    void payload_reportsSizesOnlyAsNominalCapacity();
    void nominalMemoryGib_recoversInstalledSize();
    void nominalDiskGb_snapsToTheMarketedSize();
    void nominalSizes_returnZeroForNonPositiveInput();
};

void TestDeviceInventory::payload_alwaysCarriesVersion()
{
    const QJsonObject obj = sysinfo::inventory::payload();

    // Unconditional by design: it is what makes the log document
    // self-contained without joining against the AppStart event.
    QVERIFY(obj.contains("version"));
    QVERIFY(!obj.value("version").toString().isEmpty());
}

void TestDeviceInventory::payload_bootTimeIsNumericEpochSeconds()
{
    const QJsonObject obj = sysinfo::inventory::payload();
    if (!obj.contains("boot_time")) {
        return; // Unsupported platform — omitted rather than zeroed.
    }

    const QJsonValue bootTime = obj.value("boot_time");

    // Must serialise as a JSON number: Elasticsearch maps the field as a
    // date read in epoch_second, and a quoted string would not parse.
    QVERIFY2(bootTime.isDouble(), "boot_time must be a JSON number, not a string");
    // Epoch seconds sit around 1.7e9; a millisecond value (~1.7e12) here
    // would be read against the epoch_second mapping as a far-future date.
    QVERIFY2(bootTime.toDouble() > 1e9 && bootTime.toDouble() < 1e12,
             "boot_time must be epoch seconds, not milliseconds");
}

void TestDeviceInventory::payload_omitsUnobtainableFieldsRatherThanZeroing()
{
    const QJsonObject obj = sysinfo::inventory::payload();

    // A zero would aggregate as a real measurement (a machine with 0 bytes
    // of RAM); absence is the only honest encoding of "unknown".
    for (const QString &group : {"os", "cpu", "memory", "storage"}) {
        const QJsonObject fields = obj.value(group).toObject();
        for (const QString &key : fields.keys()) {
            const QJsonValue value = fields.value(key);
            if (value.isDouble()) {
                QVERIFY2(value.toDouble() > 0,
                         qPrintable(QString("%1.%2 was emitted as zero").arg(group, key)));
            } else {
                QVERIFY2(!value.toString().isEmpty(),
                         qPrintable(QString("%1.%2 was emitted as empty").arg(group, key)));
            }
        }
    }
}

void TestDeviceInventory::payload_groupsAreObjects()
{
    const QJsonObject obj = sysinfo::inventory::payload();

    // Nested shape is part of the field contract — flattening it would
    // rename every field in the index (cpu.model -> cpu_model).
    for (const QString &group : {"os", "cpu", "memory", "storage"}) {
        if (obj.contains(group)) {
            QVERIFY2(obj.value(group).isObject(),
                     qPrintable(group + " must be a nested object"));
        }
    }
}

void TestDeviceInventory::payload_hasMandatoryCpuAndMemoryOnSupportedPlatforms()
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    const QJsonObject obj = sysinfo::inventory::payload();

    // CPU and RAM are the two components the telemetry consumer requires;
    // every supported platform can answer for them.
    const QJsonObject cpu = obj.value("cpu").toObject();
    QVERIFY(cpu.contains("cores_logical"));
    QVERIFY(cpu.contains("arch"));

    const QJsonObject memory = obj.value("memory").toObject();
    QVERIFY(memory.contains("total_gib"));
#else
    QSKIP("no hardware collectors on this platform");
#endif
}

void TestDeviceInventory::payload_osProductIsAUsableProductName()
{
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    const QJsonObject os = sysinfo::inventory::payload().value("os").toObject();

    // os.product comes straight from Qt, whose derivation of the name has
    // changed across releases. It is also a grouping key in the published
    // field contract, so a Qt upgrade that alters it silently splits the
    // buckets of every dashboard built on it — hence this guard.
    QVERIFY2(os.contains("product"), "os.product is queryable on this platform");

    const QJsonValue product = os.value("product");
    QVERIFY2(product.isString(), "os.product must be a string — it maps as keyword");

    // Qt substitutes a placeholder when it cannot identify the system. Such a
    // value indexes fine but is useless to group by, so catch it here rather
    // than let it reach the dashboards unnoticed.
    const QString name = product.toString();
    QVERIFY2(!name.contains(QLatin1String("unknown"), Qt::CaseInsensitive),
             qPrintable("os.product carries no usable product name: " + name));
#else
    QSKIP("no OS product name on this platform");
#endif
}

void TestDeviceInventory::payload_nominalMemorySizeIsRoundedUpFromReported()
{
    const QJsonObject memory =
        sysinfo::inventory::payload().value("memory").toObject();
    const qint64 reportedBytes = sysinfo::memoryInfo().totalBytes;
    if (!memory.contains("total_gib") || reportedBytes == 0) {
        QSKIP("memory size unobtainable on this platform");
    }

    const double reportedGib =
        static_cast<double>(reportedBytes) / (1024.0 * 1024 * 1024);
    const int nominalGib = memory.value("total_gib").toInt();

    // Nominal must round UP past what the OS reported (firmware reserves a
    // slice), land on an even number, and not overshoot by a whole step.
    QVERIFY2(nominalGib >= reportedGib,
             qPrintable(QString("nominal %1 < reported %2")
                            .arg(nominalGib).arg(reportedGib)));
    QCOMPARE(nominalGib % 2, 0);
    QVERIFY2(nominalGib - reportedGib < 2.0,
             qPrintable(QString("nominal %1 overshoots reported %2")
                            .arg(nominalGib).arg(reportedGib)));
}

void TestDeviceInventory::payload_reportsSizesOnlyAsNominalCapacity()
{
    const QJsonObject obj = sysinfo::inventory::payload();

    // Sizes are reported the way a person would state them. Raw byte counts,
    // usage figures and partitioning are all out of scope for a snapshot of
    // what the machine is.
    const QJsonObject memory = obj.value("memory").toObject();
    for (const QString &key : {"total_bytes", "available_bytes", "available_gib"}) {
        QVERIFY2(!memory.contains(key), qPrintable("memory." + key + " should be gone"));
    }

    const QJsonObject storage = obj.value("storage").toObject();
    for (const QString &key : {"disk_capacity_bytes", "volume_bytes", "volume_gib",
                               "total_bytes", "total_gib", "free_bytes", "free_gib"}) {
        QVERIFY2(!storage.contains(key), qPrintable("storage." + key + " should be gone"));
    }

    // Nothing anywhere in the payload should still be counting bytes.
    for (const QString &group : {"cpu", "memory", "storage"}) {
        const QJsonObject fields = obj.value(group).toObject();
        for (const QString &key : fields.keys()) {
            QVERIFY2(!key.endsWith("_bytes"),
                     qPrintable(QString("%1.%2 is a byte count").arg(group, key)));
        }
    }
}

void TestDeviceInventory::nominalMemoryGib_recoversInstalledSize()
{
    using sysinfo::inventory::nominalMemoryGib;
    constexpr qint64 gib = 1024LL * 1024 * 1024;

    // Real reported totals: firmware always keeps a slice for itself.
    QCOMPARE(nominalMemoryGib(34053414912), 32); // 31.71 GiB on a 32 GiB box
    QCOMPARE(nominalMemoryGib(17054932992), 16); // 15.88 GiB on a 16 GiB box
    QCOMPARE(nominalMemoryGib(8231837696),   8); //  7.67 GiB on an 8 GiB box

    // Exact sizes must not be pushed up to the next step.
    QCOMPARE(nominalMemoryGib(32 * gib), 32);
    QCOMPARE(nominalMemoryGib(16 * gib), 16);

    // Odd-but-real kit sizes stay themselves rather than snapping to a
    // power of two.
    QCOMPARE(nominalMemoryGib(24 * gib), 24);
    QCOMPARE(nominalMemoryGib(48 * gib), 48);
}

void TestDeviceInventory::nominalDiskGb_snapsToTheMarketedSize()
{
    using sysinfo::inventory::nominalDiskGb;

    // A 1 TB NVMe reports 1024.2 decimal GB. It must not land on 960, which
    // is also within tolerance but further away.
    QCOMPARE(nominalDiskGb(1024209543168), 1000);
    QCOMPARE(nominalDiskGb(512110190592),   512);
    QCOMPARE(nominalDiskGb(256060514304),   256);
    QCOMPARE(nominalDiskGb(960197124096),   960);
    QCOMPARE(nominalDiskGb(2048408248320), 2000);

    // Far from every known size: fall back to a plain conversion instead of
    // forcing it into the nearest bucket.
    QCOMPARE(nominalDiskGb(700LL * 1000 * 1000 * 1000), 700);
}

void TestDeviceInventory::nominalSizes_returnZeroForNonPositiveInput()
{
    // 0 is how the data layer says "unobtainable"; it must not become a size.
    QCOMPARE(sysinfo::inventory::nominalMemoryGib(0), 0);
    QCOMPARE(sysinfo::inventory::nominalDiskGb(0), 0);
    QCOMPARE(sysinfo::inventory::nominalMemoryGib(-1), 0);
    QCOMPARE(sysinfo::inventory::nominalDiskGb(-1), 0);
}

QTEST_GUILESS_MAIN(TestDeviceInventory)
#include "tst_device_inventory.moc"
