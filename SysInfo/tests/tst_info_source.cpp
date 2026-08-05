#include "core/sysinfo/info_source.h"
#include "core/sysinfo/system_info.h"

#include <QTest>

class TestInfoSource : public QObject
{
    Q_OBJECT

private slots:
    void current_collectsOnTheFirstCall();
    void current_reusesTheSnapshotInsideTheWindow();
    void current_recollectsOnceTheWindowHasPassed();
    void refresh_forcesTheNextCallToCollect();
    void current_handsBackWhatTheCollectorProduced();
};

namespace {

/// Counts collections and stamps each one, so a test can tell a fresh
/// snapshot from a reused one.
class CountingCollector
{
public:
    sysinfo::Info operator()()
    {
        ++calls;
        sysinfo::Info info;
        info.hostname = QStringLiteral("host-%1").arg(calls);
        return info;
    }

    int calls = 0;
};

/// Window short enough to keep the suite quick, long enough that a wait of
/// twice its length is not a race.
constexpr qint64 kShortTtlMs = 50;

} // namespace

void TestInfoSource::current_collectsOnTheFirstCall()
{
    CountingCollector collector;
    sysinfo::InfoSource source(kShortTtlMs, std::ref(collector));

    QCOMPARE(collector.calls, 0); // Construction must not touch the OS.
    source.current();
    QCOMPARE(collector.calls, 1);
}

void TestInfoSource::current_reusesTheSnapshotInsideTheWindow()
{
    CountingCollector collector;
    sysinfo::InfoSource source(kShortTtlMs, std::ref(collector));

    const QString first = source.current().hostname;
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(source.current().hostname, first);
    }

    // The whole point of the window: a burst of callers shares one collection.
    QCOMPARE(collector.calls, 1);
}

void TestInfoSource::current_recollectsOnceTheWindowHasPassed()
{
    CountingCollector collector;
    sysinfo::InfoSource source(kShortTtlMs, std::ref(collector));

    const QString first = source.current().hostname;
    QTest::qWait(kShortTtlMs * 2);

    QVERIFY2(source.current().hostname != first,
             "the snapshot outlived its window");
    QCOMPARE(collector.calls, 2);
}

void TestInfoSource::refresh_forcesTheNextCallToCollect()
{
    CountingCollector collector;
    sysinfo::InfoSource source(kShortTtlMs, std::ref(collector));

    const QString first = source.current().hostname;
    source.refresh();

    // Answers a user action, which must not be served a snapshot the window
    // merely has not retired yet.
    QVERIFY2(source.current().hostname != first, "refresh() left the snapshot in place");
    QCOMPARE(collector.calls, 2);
}

void TestInfoSource::current_handsBackWhatTheCollectorProduced()
{
    sysinfo::InfoSource source(kShortTtlMs, [] {
        sysinfo::Info info;
        info.hostname     = QStringLiteral("box");
        info.username     = QStringLiteral("someone");
        info.ip           = QStringLiteral("10.0.0.1");
        info.lastBootTime = QStringLiteral("01.01.2026 00:00");
        return info;
    });

    const sysinfo::Info &info = source.current();
    QCOMPARE(info.hostname,     QStringLiteral("box"));
    QCOMPARE(info.username,     QStringLiteral("someone"));
    QCOMPARE(info.ip,           QStringLiteral("10.0.0.1"));
    QCOMPARE(info.lastBootTime, QStringLiteral("01.01.2026 00:00"));
}

QTEST_GUILESS_MAIN(TestInfoSource)
#include "tst_info_source.moc"
