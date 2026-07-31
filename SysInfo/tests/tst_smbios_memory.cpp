#include "core/sysinfo/hardware_info.h"
#include "core/sysinfo/smbios_memory.h"

#include <QTest>

class TestSmbiosMemory : public QObject
{
    Q_OBJECT

private slots:
    void parseMemory_readsTheFirstPopulatedModule();
    void parseMemory_skipsAnEmptySocket();
    void parseMemory_leavesTypeEmptyForAnUnrecognisedCode();
    void parseMemory_ignoresAnEmptyTable();
    void parseMemory_stopsAtTheEndOfTableMarker();
    void parseMemory_survivesATruncatedStringSet();
    void parseMemory_survivesAStructureLongerThanTheTable();
};

namespace {

/// SMBIOS type code of a Memory Device structure.
constexpr quint8 kMemoryDevice = 17;

/// Length of the fixed part this parser requires of a Memory Device.
constexpr quint8 kMemoryDeviceLength = 0x1B;

/// SMBIOS "Memory Type" code for DDR5.
constexpr quint8 kDdr5 = 0x22;

/**
 * @brief Builds one Memory Device structure as the firmware would lay it out.
 *
 * The manufacturer and part number are written as string indices 1 and 2, so
 * the two arguments land in the string set in that order. Every field the
 * parser does not read stays zero.
 *
 * @param size         Value of the Size field; 0 marks an unpopulated socket.
 * @param typeCode     Value of the Memory Type field.
 * @param manufacturer First entry of the string set.
 * @param partNumber   Second entry of the string set.
 * @return The fixed part followed by its double-NUL-terminated string set.
 */
QByteArray memoryDevice(quint16 size,
                        quint8 typeCode,
                        const QByteArray &manufacturer,
                        const QByteArray &partNumber)
{
    QByteArray out(kMemoryDeviceLength, '\0');
    out[0]    = char(kMemoryDevice);
    out[1]    = char(kMemoryDeviceLength);
    out[0x0C] = char(size & 0xFF);
    out[0x0D] = char((size >> 8) & 0xFF);
    out[0x12] = char(typeCode);
    out[0x17] = 1; // Manufacturer -> first string.
    out[0x1A] = 2; // Part Number  -> second string.

    out.append(manufacturer);
    out.append('\0');
    out.append(partNumber);
    out.append('\0');
    out.append('\0'); // Terminates the set.
    return out;
}

/// Builds the type 127 structure that closes a table.
QByteArray endOfTable()
{
    QByteArray out(4, '\0');
    out[0] = 127;
    out[1] = 4;
    out.append('\0');
    out.append('\0');
    return out;
}

} // namespace

void TestSmbiosMemory::parseMemory_readsTheFirstPopulatedModule()
{
    const QByteArray table =
        memoryDevice(16384, kDdr5, "Samsung", "M471A1K43CB1-CTD") + endOfTable();

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    // Pins down all four field offsets at once: a wrong one surfaces as a
    // value read out of the neighbouring field.
    QCOMPARE(memory.manufacturer, QStringLiteral("Samsung"));
    QCOMPARE(memory.model,        QStringLiteral("M471A1K43CB1-CTD"));
    QCOMPARE(memory.type,         QStringLiteral("DDR5"));
}

void TestSmbiosMemory::parseMemory_skipsAnEmptySocket()
{
    // A machine with one module installed and one socket free reports both.
    const QByteArray table = memoryDevice(0, kDdr5, "Nobody", "EMPTY-SLOT")
                           + memoryDevice(8192, kDdr5, "Crucial", "CT8G4SFRA32A")
                           + endOfTable();

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    QCOMPARE(memory.manufacturer, QStringLiteral("Crucial"));
    QCOMPARE(memory.model,        QStringLiteral("CT8G4SFRA32A"));
}

void TestSmbiosMemory::parseMemory_leavesTypeEmptyForAnUnrecognisedCode()
{
    // 0x02 is the spec's "Unknown". The identity fields are still readable,
    // so an unmapped generation must not cost them.
    const QByteArray table = memoryDevice(16384, 0x02, "Kingston", "KF548S38") + endOfTable();

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    QVERIFY2(memory.type.isEmpty(), qPrintable("unexpected type: " + memory.type));
    QCOMPARE(memory.manufacturer, QStringLiteral("Kingston"));
    QCOMPARE(memory.model,        QStringLiteral("KF548S38"));
}

void TestSmbiosMemory::parseMemory_ignoresAnEmptyTable()
{
    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(QByteArray(), memory);

    QVERIFY(memory.manufacturer.isEmpty());
    QVERIFY(memory.model.isEmpty());
    QVERIFY(memory.type.isEmpty());
}

void TestSmbiosMemory::parseMemory_stopsAtTheEndOfTableMarker()
{
    // Nothing follows type 127 on real firmware; a structure placed after it
    // stands in for whatever bytes happen to sit there.
    const QByteArray table =
        endOfTable() + memoryDevice(16384, kDdr5, "Samsung", "M471A1K43CB1-CTD");

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    QVERIFY2(memory.manufacturer.isEmpty(),
             qPrintable("read past the end marker: " + memory.manufacturer));
}

void TestSmbiosMemory::parseMemory_survivesATruncatedStringSet()
{
    // Cut inside the string set, so it never reaches its double NUL.
    QByteArray table = memoryDevice(16384, kDdr5, "Samsung", "M471A1K43CB1-CTD");
    table.chop(8);

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    QVERIFY2(memory.manufacturer.isEmpty(),
             qPrintable("accepted a truncated set: " + memory.manufacturer));
}

void TestSmbiosMemory::parseMemory_survivesAStructureLongerThanTheTable()
{
    // A header claiming more bytes than remain is the shape a malformed table
    // takes; the walk must stop rather than run off the end.
    QByteArray table(6, '\0');
    table[0] = char(kMemoryDevice);
    table[1] = char(0x60); // Claims 96 bytes of fixed part inside 6 bytes.

    sysinfo::Memory memory;
    sysinfo::smbios::parseMemory(table, memory);

    QVERIFY(memory.manufacturer.isEmpty());
    QVERIFY(memory.type.isEmpty());
}

QTEST_GUILESS_MAIN(TestSmbiosMemory)
#include "tst_smbios_memory.moc"
