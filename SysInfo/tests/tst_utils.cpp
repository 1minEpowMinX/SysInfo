#include "../utils/utils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QTest>

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void toText_formatsAllFields();
    void toJson_withoutLabels_hasExpectedKeys();
    void toJson_withLabels_includesLabels();
    void collectSystemInfo_basicSanity();
    void getActiveIPAddress_returnsIpOrFallback();
    void getHostname_isNotEmpty();
    void getLastBootTime_matchesExpectedShape();
};

void TestUtils::toText_formatsAllFields()
{
    Utils::SystemInfo s;
    s.hostname = "host-a";
    s.username = "user-b";
    s.ip = "10.11.12.13";
    s.uptime = "01.01.2026 12:00";

    const QString text = Utils::toText(s);

    QVERIFY(text.contains(s.hostname));
    QVERIFY(text.contains(s.username));
    QVERIFY(text.contains(s.ip));
    QVERIFY(text.contains(s.uptime));
}

void TestUtils::toJson_withoutLabels_hasExpectedKeys()
{
    Utils::SystemInfo s;
    s.hostname = "h";
    s.username = "u";
    s.ip = "1.2.3.4";
    s.uptime = "up";

    const QJsonObject obj = Utils::toJson(s, false);

    QCOMPARE(obj.value("hostname").toString(), QString("h"));
    QCOMPARE(obj.value("username").toString(), QString("u"));
    QCOMPARE(obj.value("ip").toString(), QString("1.2.3.4"));
    QCOMPARE(obj.value("uptime").toString(), QString("up"));
    QVERIFY(!obj.contains("labels"));
}

void TestUtils::toJson_withLabels_includesLabels()
{
    Utils::SystemInfo s;
    const QJsonObject obj = Utils::toJson(s, true);

    QVERIFY(obj.contains("labels"));
    const QJsonObject labels = obj.value("labels").toObject();
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QVERIFY2(labels.contains(key), qPrintable("missing label: " + key));
        QVERIFY2(!labels.value(key).toString().isEmpty(),
                 qPrintable("empty label: " + key));
    }
}

void TestUtils::collectSystemInfo_basicSanity()
{
    const Utils::SystemInfo s = Utils::collectSystemInfo();

    // Hostname must always be available even on a minimal CI runner
    QVERIFY(!s.hostname.isEmpty());
    // Uptime should be non-empty (either formatted date or localized fallback)
    QVERIFY(!s.uptime.isEmpty());
}

void TestUtils::getActiveIPAddress_returnsIpOrFallback()
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

void TestUtils::getHostname_isNotEmpty()
{
    QVERIFY(!Utils::getHostname().isEmpty());
}

void TestUtils::getLastBootTime_matchesExpectedShape()
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

QTEST_GUILESS_MAIN(TestUtils)
#include "tst_utils.moc"
