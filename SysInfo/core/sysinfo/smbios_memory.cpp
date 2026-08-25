#include "smbios_memory.h"

#include <QString>

namespace sysinfo::smbios {

// Taking the table as an argument rather than reading it here is what keeps the
// parser callable, and testable, on a platform that has no SMBIOS to read.

namespace {

/**
 * @brief Reads string number @p index (1-based) from a structure's string set.
 *
 * SMBIOS stores strings in a NUL-separated block that follows the fixed part
 * of each structure. Index 0 is the spec's way of saying "no string".
 *
 * @param strings Start of the set.
 * @param end     One past its last byte — the position of the terminating
 *                double NUL, not the end of the whole table, so a structure
 *                can never hand out bytes belonging to the next one.
 * @param index   1-based position within the set.
 * @return        The string, or empty if @p index is 0 or out of range.
 */
QString smbiosString(const char *strings, const char *end, quint8 index)
{
    if (index == 0) {
        return {};
    }

    const char *cursor = strings;
    for (int i = 1; cursor < end; ++i) {
        const qsizetype length = qstrnlen(cursor, end - cursor);
        if (i == index) {
            return QString::fromLatin1(cursor, length).trimmed();
        }
        if (length == 0) {
            break; // Empty string here means the set has ended.
        }
        cursor += length + 1;
    }
    return {};
}

/// Translates the SMBIOS "Memory Type" enum into the name people use.
QString smbiosMemoryType(quint8 code)
{
    switch (code) {
    case 0x12: return QStringLiteral("DDR");
    case 0x13: return QStringLiteral("DDR2");
    case 0x14: return QStringLiteral("DDR2 FB-DIMM");
    case 0x18: return QStringLiteral("DDR3");
    case 0x1A: return QStringLiteral("DDR4");
    case 0x1B: return QStringLiteral("LPDDR");
    case 0x1C: return QStringLiteral("LPDDR2");
    case 0x1D: return QStringLiteral("LPDDR3");
    case 0x1E: return QStringLiteral("LPDDR4");
    case 0x20: return QStringLiteral("HBM");
    case 0x21: return QStringLiteral("HBM2");
    case 0x22: return QStringLiteral("DDR5");
    case 0x23: return QStringLiteral("LPDDR5");
    default:   return {}; // Includes "Other" and "Unknown" — nothing worth logging.
    }
}

} // namespace

void parseMemory(const QByteArray &table, Memory &memory)
{
    const char *const base = table.constData();
    const char *const end = base + table.size();
    const char *entry = base;

    // Field offsets within a type 17 structure, per the SMBIOS spec.
    constexpr int kSizeOffset         = 0x0C;
    constexpr int kMemoryTypeOffset   = 0x12;
    constexpr int kManufacturerOffset = 0x17;
    constexpr int kPartNumberOffset   = 0x1A;
    constexpr int kMinLength          = 0x1B;

    // Every bound below is expressed as a subtraction of two pointers into the table.
    while (end - entry >= 4) {
        const auto *fields = reinterpret_cast<const quint8 *>(entry);
        const quint8 type = fields[0];
        const quint8 length = fields[1];

        if (length < 4 || end - entry < length) {
            break; // Malformed header — stop rather than walk off the table.
        }
        if (type == 127) {
            break; // End-of-table marker.
        }

        // The string set runs from the end of the fixed part to a double NUL.
        const char *const strings = entry + length;
        const char *cursor = strings;
        while (end - cursor >= 2 && !(cursor[0] == '\0' && cursor[1] == '\0')) {
            ++cursor;
        }
        if (end - cursor < 2) {
            break; // Set never terminates — the table is truncated.
        }

        if (type == 17 && length >= kMinLength) {
            const quint16 size =
                quint16(fields[kSizeOffset] | (fields[kSizeOffset + 1] << 8));
            if (size != 0) { // Populated slot.
                memory.type = smbiosMemoryType(fields[kMemoryTypeOffset]);
                memory.manufacturer =
                    smbiosString(strings, cursor, fields[kManufacturerOffset]);
                memory.model = smbiosString(strings, cursor, fields[kPartNumberOffset]);
                return;
            }
        }

        entry = cursor + 2;
    }
}

} // namespace sysinfo::smbios
