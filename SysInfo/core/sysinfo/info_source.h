#ifndef INFO_SOURCE_H
#define INFO_SOURCE_H

#include "system_info.h"

#include <QElapsedTimer>

#include <functional>

namespace sysinfo {

/**
 * @brief Owns the session snapshot and hands it to everyone who needs one.
 *
 * collect() enumerates every network interface — on Windows a
 * GetAdaptersAddresses call costing tens of milliseconds — and every caller in
 * SysInfo runs on the GUI thread. One instance shared between them means
 * callers arriving close together pay for a single collection, and means there
 * is one answer to "how old may the snapshot be" rather than one per caller.
 *
 * The window bounds staleness, not the polling rate: a caller that wants to
 * notice a change every 30 s asks that often and is served a fresh collection
 * each time, because 30 s exceeds any sensible window. A caller answering a
 * user action calls refresh() first.
 *
 * Not thread-safe, and not meant to be: it exists precisely because its
 * callers share one thread.
 */
class InfoSource
{
public:
    /// Produces a snapshot. Substituted in tests; sysinfo::collect elsewhere.
    using Collector = std::function<Info()>;

    /// Window the application runs with.
    static constexpr qint64 kDefaultTtlMs = 1000;

    /**
     * @param ttlMs   How long a collected snapshot stays good. A value of 0
     *                makes every current() collect afresh.
     * @param collect Where snapshots come from.
     */
    explicit InfoSource(qint64 ttlMs = kDefaultTtlMs,
                        Collector collect = [] { return sysinfo::collect(); });

    /**
     * @brief Returns the snapshot, collecting first if none is good any more.
     *
     * Nothing is collected before the first call, so constructing a source
     * costs nothing.
     *
     * @return The current snapshot, which may trail the true state by up to
     *         the window.
     */
    const Info &current();

    /// Discards the held snapshot, so the next current() collects afresh.
    void refresh();

private:
    qint64        m_ttlMs;    ///< Length of the window, in milliseconds.
    Collector     m_collect;  ///< Where snapshots come from.
    Info          m_cached;   ///< Valid only while m_age has not expired.
    QElapsedTimer m_age;      ///< Invalid until the first collection.
};

} // namespace sysinfo

#endif // INFO_SOURCE_H
