#include "core/runtime/single_instance_guard.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using Result = SingleInstanceGuard::Result;

namespace QTest {

/// Without this a failing QCOMPARE prints the enumerators as raw bytes.
template <>
char *toString(const Result &result)
{
    switch (result) {
    case Result::Acquired:       return qstrdup("Acquired");
    case Result::AlreadyRunning: return qstrdup("AlreadyRunning");
    case Result::Unavailable:    return qstrdup("Unavailable");
    }
    return qstrdup("<unknown>");
}

} // namespace QTest

class TestSingleInstanceGuard : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void tryAcquire_onFreePath_acquires();
    void tryAcquire_whileHeld_isIdempotent();
    void secondGuard_onSameLock_reportsAlreadyRunning();
    void destruction_releasesTheLock();

    void tryAcquire_reclaimsUnreadableLockFile_data();
    void tryAcquire_reclaimsUnreadableLockFile();
    void liveLock_isNeverReclaimed();

    void tryAcquire_whenLockFileCannotBeCreated_reportsUnavailable();
    void defaultLockFilePath_isAbsoluteAndNamedForTheApp();

private:
    QString lockPath() const { return m_dir.filePath("SysInfo.lock"); }

    /// Writes @p contents to the lock path, standing in for what an unclean
    /// shutdown can leave behind.
    void writeLeftoverLockFile(const QByteArray &contents);

    QTemporaryDir m_dir;
};

void TestSingleInstanceGuard::init()
{
    QVERIFY2(m_dir.isValid(), qPrintable(m_dir.errorString()));
    // Guards run against a shared path, so clear it between cases.
    QFile::remove(lockPath());
}

void TestSingleInstanceGuard::writeLeftoverLockFile(const QByteArray &contents)
{
    QFile file(lockPath());
    QVERIFY(file.open(QIODevice::WriteOnly));
    QCOMPARE(file.write(contents), qint64(contents.size()));
    file.close();
    QVERIFY(QFile::exists(lockPath()));
}

// --- Basic ownership ---------------------------------------------------------

void TestSingleInstanceGuard::tryAcquire_onFreePath_acquires()
{
    SingleInstanceGuard guard(lockPath());
    QVERIFY(!guard.isHeld());

    QCOMPARE(guard.tryAcquire(), Result::Acquired);
    QVERIFY(guard.isHeld());
    QVERIFY(QFile::exists(lockPath()));
    QVERIFY2(!guard.reclaimedStaleLock(),
             "nothing was left over, so nothing should have been reclaimed");
}

void TestSingleInstanceGuard::tryAcquire_whileHeld_isIdempotent()
{
    SingleInstanceGuard guard(lockPath());
    QCOMPARE(guard.tryAcquire(), Result::Acquired);

    // QLockFile forbids locking recursively and answers false if asked; the
    // guard must not relay that as a failure to start.
    QCOMPARE(guard.tryAcquire(), Result::Acquired);
    QVERIFY(guard.isHeld());
}

void TestSingleInstanceGuard::secondGuard_onSameLock_reportsAlreadyRunning()
{
    SingleInstanceGuard first(lockPath());
    QCOMPARE(first.tryAcquire(), Result::Acquired);

    SingleInstanceGuard second(lockPath());
    QCOMPARE(second.tryAcquire(), Result::AlreadyRunning);
    QVERIFY(!second.isHeld());
    QVERIFY2(first.isHeld(), "the first guard must keep its lock");
}

void TestSingleInstanceGuard::destruction_releasesTheLock()
{
    {
        SingleInstanceGuard guard(lockPath());
        QCOMPARE(guard.tryAcquire(), Result::Acquired);
    }

    QVERIFY2(!QFile::exists(lockPath()),
             "the lock file should be gone once the guard is destroyed");

    SingleInstanceGuard next(lockPath());
    QCOMPARE(next.tryAcquire(), Result::Acquired);
}

// --- Recovery from an unclean shutdown ---------------------------------------

void TestSingleInstanceGuard::tryAcquire_reclaimsUnreadableLockFile_data()
{
    QTest::addColumn<QByteArray>("contents");

    // The lock file is created first and the owner written into it second, so
    // losing power in between leaves a file that names nobody. Zero bytes is
    // the plain case; a run of NULs is what NTFS can surface after an unclean
    // shutdown once the metadata reached disk before the data did.
    QTest::newRow("zero bytes")   << QByteArray();
    QTest::newRow("nul bytes")    << QByteArray(64, '\0');
    QTest::newRow("garbage text") << QByteArray("not-a-pid\ngarbage\nnowhere\n");
}

void TestSingleInstanceGuard::tryAcquire_reclaimsUnreadableLockFile()
{
    QFETCH(QByteArray, contents);
    writeLeftoverLockFile(contents);

    SingleInstanceGuard guard(lockPath());
    QCOMPARE(guard.tryAcquire(), Result::Acquired);
    QVERIFY(guard.isHeld());
    QVERIFY2(guard.reclaimedStaleLock(),
             "removing an unreadable lock file must be reported to the caller");
}

void TestSingleInstanceGuard::liveLock_isNeverReclaimed()
{
    // The counterpart to the case above: reclaiming must stay confined to files
    // that name no owner, or a second instance would evict a healthy first one.
    SingleInstanceGuard owner(lockPath());
    QCOMPARE(owner.tryAcquire(), Result::Acquired);

    SingleInstanceGuard intruder(lockPath());
    QCOMPARE(intruder.tryAcquire(), Result::AlreadyRunning);
    QVERIFY2(!intruder.reclaimedStaleLock(), "a live lock must not be reclaimed");
    QVERIFY(owner.isHeld());
    QVERIFY2(QFile::exists(lockPath()), "the owner's lock file must survive");
}

// --- The lock cannot be established at all -----------------------------------

void TestSingleInstanceGuard::tryAcquire_whenLockFileCannotBeCreated_reportsUnavailable()
{
    // A missing parent directory stands in for every environment where the file
    // cannot be created — no permission on the temp directory, a full partition.
    // What matters is that this is told apart from AlreadyRunning: the caller
    // stays silent for one and reports a fault for the other.
    SingleInstanceGuard guard(m_dir.filePath("no/such/directory/SysInfo.lock"));

    QCOMPARE(guard.tryAcquire(), Result::Unavailable);
    QVERIFY(!guard.isHeld());
    QVERIFY(!guard.reclaimedStaleLock());
}

void TestSingleInstanceGuard::defaultLockFilePath_isAbsoluteAndNamedForTheApp()
{
    const QString path = SingleInstanceGuard::defaultLockFilePath();

    QVERIFY(!path.isEmpty());
    QVERIFY2(QDir::isAbsolutePath(path), qPrintable(path));
    QVERIFY2(path.endsWith(QLatin1String("SysInfo.lock")), qPrintable(path));
}

QTEST_GUILESS_MAIN(TestSingleInstanceGuard)
#include "tst_single_instance_guard.moc"
