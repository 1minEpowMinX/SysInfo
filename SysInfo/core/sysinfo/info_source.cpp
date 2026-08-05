#include "info_source.h"

#include <utility>

namespace sysinfo {

// One shared source rather than a collector per caller: collect() enumerates
// every network interface — on Windows a GetAdaptersAddresses call costing tens
// of milliseconds — and every caller in SysInfo runs on the GUI thread. That
// one thread is also why nothing here is guarded.

InfoSource::InfoSource(qint64 ttlMs, Collector collect)
    : m_ttlMs(ttlMs)
    , m_collect(std::move(collect))
{}

const Info &InfoSource::current()
{
    if (!m_age.isValid() || m_age.hasExpired(m_ttlMs)) {
        m_cached = m_collect();
        m_age.start();
    }
    return m_cached;
}

void InfoSource::refresh()
{
    m_age.invalidate();
}

} // namespace sysinfo
