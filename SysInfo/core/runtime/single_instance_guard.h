#ifndef SINGLE_INSTANCE_GUARD_H
#define SINGLE_INSTANCE_GUARD_H

#include <QLockFile>
#include <QString>

/**
 * @brief Lets only one SysInfo run at a time, for as long as this object lives.
 *
 * tryAcquire() takes the lock and the destructor releases it, so the guard must
 * stay alive for the whole run. Not copyable — it owns a QLockFile.
 *
 * Leftover lock files
 * -------------------
 * A lock file survives a process that dies without unlocking. Three kinds of
 * leftover exist and each resolves differently:
 *
 *   - names a process that is gone — QLockFile deletes it and locks, matching
 *     the recorded PID and process name against the running process;
 *   - names a live process — the lock is genuinely held, tryAcquire() reports
 *     AlreadyRunning;
 *   - names nobody, being empty or unparseable — tryAcquire() deletes it and
 *     records the deletion for reclaimedStaleLock().
 *
 * Age-based stale detection is off (QLockFile::setStaleLockTime(0)), as Qt
 * prescribes for a lock held indefinitely.
 */
class SingleInstanceGuard
{
public:
    /**
     * @brief Enumerates the outcomes of tryAcquire().
     *
     * AlreadyRunning and Unavailable both mean this process must not continue.
     * They are separate values: the first is a routine second launch, the
     * second a fault.
     */
    enum class Result
    {
        Acquired,        ///< This process is the only instance.
        AlreadyRunning,  ///< Another SysInfo owns the lock.
        Unavailable,     ///< The lock file could not be created at all.
    };

    /**
     * @param lockFilePath Absolute path of the lock file; see
     *                     defaultLockFilePath() for the one the application uses.
     */
    explicit SingleInstanceGuard(const QString &lockFilePath);

    /// @return Absolute path of "SysInfo.lock" in the user's temporary directory.
    static QString defaultLockFilePath();

    /**
     * @brief Takes the lock, deleting an unparseable lock file if one blocks it.
     *
     * Idempotent: while the lock is held, further calls report Acquired without
     * re-locking.
     *
     * @return Acquired, AlreadyRunning or Unavailable; see Result.
     */
    Result tryAcquire();

    /// @return true if this guard currently owns the lock.
    bool isHeld() const;

    /**
     * @brief Reports whether the acquired lock replaced an unparseable leftover file.
     *
     * @return true if tryAcquire() deleted a lock file it could not parse.
     */
    bool reclaimedStaleLock() const;

private:
    QLockFile m_lock;
    bool m_reclaimedStaleLock = false;
};

#endif // SINGLE_INSTANCE_GUARD_H
