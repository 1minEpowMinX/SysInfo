#include "single_instance_guard.h"

#include <QDir>
#include <QStandardPaths>

namespace {

/// Covers a previous instance still releasing the lock during its own shutdown,
/// while keeping a rejected second launch instant.
constexpr int kAcquireTimeoutMs = 100;

} // namespace

SingleInstanceGuard::SingleInstanceGuard(const QString &lockFilePath)
    : m_lock(lockFilePath)
{
    m_lock.setStaleLockTime(0);
}

QString SingleInstanceGuard::defaultLockFilePath()
{
    return QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .absoluteFilePath(QStringLiteral("SysInfo.lock"));
}

SingleInstanceGuard::Result SingleInstanceGuard::tryAcquire()
{
    if (m_lock.isLocked()) {
        return Result::Acquired;
    }

    if (m_lock.tryLock(kAcquireTimeoutMs)) {
        return Result::Acquired;
    }

    // Anything other than "somebody holds it" — no permission on the parent
    // directory, a full partition — will not be cured by trying again.
    if (m_lock.error() != QLockFile::LockFailedError) {
        return Result::Unavailable;
    }

    // A file that names its owner is authoritative: either that process is
    // alive, or QLockFile would already have reclaimed the file on its own.
    qint64 pid = 0;
    QString hostname;
    QString appname;
    if (m_lock.getLockInfo(&pid, &hostname, &appname)) {
        return Result::AlreadyRunning;
    }

    // Nothing identifies a live holder. On Windows removeStaleLockFile() refuses
    // while any process holds the file open; elsewhere a live QLockFile always
    // has readable contents. Either way only an ownerless file is removed here.
    if (!m_lock.removeStaleLockFile() || !m_lock.tryLock(kAcquireTimeoutMs)) {
        return Result::AlreadyRunning;
    }

    m_reclaimedStaleLock = true;
    return Result::Acquired;
}

bool SingleInstanceGuard::isHeld() const
{
    return m_lock.isLocked();
}

bool SingleInstanceGuard::reclaimedStaleLock() const
{
    return m_reclaimedStaleLock;
}
