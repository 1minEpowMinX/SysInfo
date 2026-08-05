#ifndef SMBIOS_MEMORY_H
#define SMBIOS_MEMORY_H

#include "hardware_info.h"

#include <QByteArray>

/**
 * @brief Reads memory-module identity out of a raw SMBIOS table.
 *
 * Windows and Linux hand out the same table and differ only in how the bytes
 * are obtained, so the parser is shared between their platform
 * implementations. It calls no OS API of its own — the input is a byte array —
 * and compiles on every platform.
 */
namespace sysinfo::smbios {

/**
 * @brief Fills the identity fields of @p memory from a raw SMBIOS table.
 *
 * Walks @p table for the first populated Memory Device (type 17) structure. A
 * slot whose size is 0 is an empty socket and is skipped, so the answer
 * describes a module that is actually installed. A table that is empty,
 * truncated or carries no such structure leaves @p memory untouched.
 *
 * @param table  Raw SMBIOS structure table, stripped of any provider header.
 * @param memory Destination; totalBytes is not read or written.
 */
void parseMemory(const QByteArray &table, Memory &memory);

} // namespace sysinfo::smbios

#endif // SMBIOS_MEMORY_H
