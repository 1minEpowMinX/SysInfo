#ifndef INFO_SOURCE_H
#define INFO_SOURCE_H

#include "system_info.h"

#include <QElapsedTimer>

#include <functional>

namespace sysinfo {

/**
 * @brief Owns the session snapshot and hands it to everyone who needs one.
 *
 * Holds one collected Info and collects again once the held snapshot is older
 * than the window, so callers arriving within one window share a single
 * collection and one answer to how old that snapshot may be.
 *
 * The window bounds staleness, not the polling rate: a caller polling on an
 * interval longer than the window is served a fresh collection every time it
 * asks. A caller answering a user action calls refresh() first.
 *
 * Not thread-safe.
 */
class InfoSource
{
public:
    /// Produces a snapshot; sysinfo::collect() unless the caller names another.
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
