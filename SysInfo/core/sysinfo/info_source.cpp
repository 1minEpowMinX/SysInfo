#include "info_source.h"

#include <utility>

namespace sysinfo {

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
