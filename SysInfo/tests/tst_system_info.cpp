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

    // Hostname must always be available even on a minimal CI runner
    QVERIFY(!s.hostname.isEmpty());
    // Uptime should be non-empty (either formatted date or localized fallback)
    QVERIFY(!s.uptime.isEmpty());
}

void TestSystemInfo::getActiveIPAddress_returnsIpOrFallback()
{
    const QString ip = Utils::getActiveIPAddress();
    QVERIFY(!ip.isEmpty());

    // Either it's a dotted-quad IPv4, or the localized fallback
    const QRegularExpression ipv4(
        QStringLiteral("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"));
    const bool looksLikeIpv4 = ipv4.match(ip).hasMatch();
    const bool looksLikeFallback = ip == QObject::tr("No IP");
    QVERIFY2(looksLikeIpv4 || looksLikeFallback,
             qPrintable("unexpected IP value: " + ip));
}

void TestSystemInfo::getHostname_isNotEmpty()
{
    QVERIFY(!Utils::getHostname().isEmpty());
}

void TestSystemInfo::getLastBootTime_matchesExpectedShape()
{
    const QString boot = Utils::getLastBootTime();
    QVERIFY(!boot.isEmpty());

    // Either the supported-platform format "dd.MM.yyyy HH:mm" or a fallback phrase
    const QRegularExpression fmt(
        QStringLiteral("^\\d{2}\\.\\d{2}\\.\\d{4} \\d{2}:\\d{2}$"));
    const bool looksLikeDate = fmt.match(boot).hasMatch();
    const bool isFallback = boot == QObject::tr("Unavailable")
                            || boot == QObject::tr("Not supported");
    QVERIFY2(looksLikeDate || isFallback,
             qPrintable("unexpected boot time: " + boot));
}

QTEST_GUILESS_MAIN(TestSystemInfo)
#include "tst_system_info.moc"
