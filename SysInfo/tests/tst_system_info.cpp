#include "core/sysinfo/system_info.h"

#include <QRegularExpression>
#include <QTest>

class TestSystemInfo : public QObject
{
    Q_OBJECT

private slots:
    void collectSystemInfo_basicSanity();
    void getActiveIPAddress_returnsIpOrFallback();
    void getHostname_isNotEmpty();
    void getLastBootTime_matchesExpectedShape();
};

void TestSystemInfo::collectSystemInfo_basicSanity()
{
    const Utils::SystemInfo s = Utils::collectSystemInfo();

    // Hostname must always be available even on a minimal CI runner.
    QVERIFY(!s.hostname.isEmpty());
    // Uptime is either a formatted date, or empty (unsupported / syscall
    // failure). Localised "Unavailable" fallback is the presenter's job
    // and must NOT leak into the data layer.
}

void TestSystemInfo::getActiveIPAddress_returnsIpOrFallback()
{
    const QString ip = Utils::getActiveIPAddress();

    // Either it's a dotted-quad IPv4, or empty ("no IP found").
    // The model never returns a localised placeholder.
    const QRegularExpression ipv4(
        QStringLiteral("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"));
    const bool looksLikeIpv4 = ipv4.match(ip).hasMatch();
    QVERIFY2(looksLikeIpv4 || ip.isEmpty(),
             qPrintable("unexpected IP value: " + ip));
}

void TestSystemInfo::getHostname_isNotEmpty()
{
    QVERIFY(!Utils::getHostname().isEmpty());
}

void TestSystemInfo::getLastBootTime_matchesExpectedShape()
{
    const QString boot = Utils::getLastBootTime();

    // Either the supported-platform format "dd.MM.yyyy HH:mm" or empty.
    // The model never returns a localised "Unavailable"/"Not supported"
    // string — that's the presenter's job.
    const QRegularExpression fmt(
        QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4} \\d{2}:\\d{2}$"));
    const bool looksLikeDate = fmt.match(boot).hasMatch();
    QVERIFY2(looksLikeDate || boot.isEmpty(),
             qPrintable("unexpected boot time: " + boot));
}

QTEST_GUILESS_MAIN(TestSystemInfo)
#include "tst_system_info.moc"
