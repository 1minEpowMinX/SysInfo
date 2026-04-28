#include "core/sysinfo/system_info.h"

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
};

void TestSystemInfo::collect_basicSanity()
{
    const sysinfo::Info s = sysinfo::collect();

    // Hostname must always be available even on a minimal CI runner.
    QVERIFY(!s.hostname.isEmpty());
    // Uptime is either a formatted date, or empty (unsupported / syscall
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

QTEST_GUILESS_MAIN(TestSystemInfo)
#include "tst_system_info.moc"
