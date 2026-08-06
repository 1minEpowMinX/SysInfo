#include "core/sysinfo/system_info.h"

#include <QDateTime>
#include <QRegularExpression>
#include <QTest>

class TestSystemInfo : public QObject
{
    Q_OBJECT

private slots:
    void collect_basicSanity();
    void activeIpAddress_returnsIpOrEmpty();
    void hostname_isNotEmpty();
    void lastBootTime_matchesExpectedShape();
    void bootTimeRenderings_agreeWithBootTime();
    void bootTimeSecs_isZeroOrPlausibleEpoch();
    void osBuild_isReportedOnSupportedPlatforms();
    void osBuild_carriesRevisionOnWindows();
};

void TestSystemInfo::collect_basicSanity()
{
    const sysinfo::Info s = sysinfo::collect();

    // Hostname must always be available even on a minimal CI runner.
    QVERIFY(!s.hostname.isEmpty());
    // The boot time is either a formatted date, or empty (unsupported / syscall
    // failure). Localised "Unavailable" fallback is the presenter's job
    // and must NOT leak into the data layer.
}

void TestSystemInfo::activeIpAddress_returnsIpOrEmpty()
{
    const QString ip = sysinfo::activeIpAddress();

    // Either it's a dotted-quad IPv4, or empty ("no IP found").
    // The model never returns a localised placeholder.
    const QRegularExpression ipv4(
        QStringLiteral("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"));
    const bool looksLikeIpv4 = ipv4.match(ip).hasMatch();
    QVERIFY2(looksLikeIpv4 || ip.isEmpty(),
             qPrintable("unexpected IP value: " + ip));
}

void TestSystemInfo::hostname_isNotEmpty()
{
    QVERIFY(!sysinfo::hostname().isEmpty());
}

void TestSystemInfo::lastBootTime_matchesExpectedShape()
{
    const QString boot = sysinfo::lastBootTime();

    // Either the supported-platform format "dd.MM.yyyy HH:mm" or empty.
    // The model never returns a localised "Unavailable" string — that's
    // the presenter's job.
    const QRegularExpression fmt(
        QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4} \\d{2}:\\d{2}$"));
    const bool looksLikeDate = fmt.match(boot).hasMatch();
    QVERIFY2(looksLikeDate || boot.isEmpty(),
             qPrintable("unexpected boot time: " + boot));
}

void TestSystemInfo::bootTimeRenderings_agreeWithBootTime()
{
    // lastBootTime() and bootTimeSecs() must stay pure renderings of
    // bootTime() — if either grows its own platform query, this drifts.
    const QDateTime boot = sysinfo::bootTime();

    if (!boot.isValid()) {
        QVERIFY(sysinfo::lastBootTime().isEmpty());
        QCOMPARE(sysinfo::bootTimeSecs(), qint64(0));
        return;
    }

    QCOMPARE(sysinfo::lastBootTime(), boot.toString("dd.MM.yyyy HH:mm"));

    // Boot time is derived from an uptime counter sampled per call, so two
    // calls drift by the time between them rather than matching exactly.
    const qint64 drift =
        qAbs(sysinfo::bootTimeSecs() - boot.toSecsSinceEpoch());
    QVERIFY2(drift < 5, qPrintable(QString("drift too large: %1 s").arg(drift)));
}

void TestSystemInfo::bootTimeSecs_isZeroOrPlausibleEpoch()
{
    const qint64 secs = sysinfo::bootTimeSecs();
    if (secs == 0) {
        return; // Unsupported platform / failed syscall.
    }

    // Must be epoch SECONDS, not milliseconds: the Elasticsearch field is
    // mapped epoch_second, so a millisecond value would be read as a date
    // ~50000 years out. 1e9 s is 2001; anything smaller is a unit bug.
    QVERIFY2(secs > Q_INT64_C(1'000'000'000),
             qPrintable(QString("implausibly early boot time: %1").arg(secs)));
    QVERIFY2(secs <= QDateTime::currentSecsSinceEpoch(),
             "boot time must not be in the future");
    // Guards against a stray millisecond value slipping through: nobody's
    // machine booted in the year 33000.
    QVERIFY2(secs < Q_INT64_C(1'000'000'000'000),
             qPrintable(QString("looks like epoch millis, not seconds: %1").arg(secs)));
}

void TestSystemInfo::osBuild_isReportedOnSupportedPlatforms()
{
    const QString build = sysinfo::osBuild();

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    QVERIFY2(!build.isEmpty(), "OS build is queryable on this platform");
#endif

    // Whatever it is, it must be a single token — the Linux source line is a
    // whole sentence and only its leading tag belongs in the field.
    QVERIFY2(!build.contains(QLatin1Char(' ')),
             qPrintable("os build should not contain spaces: " + build));
}

void TestSystemInfo::osBuild_carriesRevisionOnWindows()
{
#ifdef Q_OS_WIN
    const QString build = sysinfo::osBuild();

    // The whole point of reading the registry is the UBR after the dot;
    // without it this is no better than QSysInfo::kernelVersion().
    const QStringList parts = build.split(QLatin1Char('.'));
    QCOMPARE(parts.size(), 2);

    for (const QString &part : parts) {
        bool numeric = false;
        part.toUInt(&numeric);
        QVERIFY2(numeric, qPrintable("non-numeric build component: " + build));
    }
#else
    QSKIP("UBR is a Windows concept");
#endif
}

QTEST_GUILESS_MAIN(TestSystemInfo)
#include "tst_system_info.moc"
