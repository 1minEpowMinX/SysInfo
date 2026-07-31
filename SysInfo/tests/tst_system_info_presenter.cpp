#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QJsonObject>
#include <QSysInfo>
#include <QTest>

class TestSystemInfoPresenter : public QObject
{
    Q_OBJECT

private slots:
    void toText_formatsAllFields();
    void toJson_hasExpectedKeys_andNoLabels();
    void toJsonWithLabels_includesAllLabels();
    void toJsonWithLabels_containsSameDataAsToJson();
    void toJson_keepsEmptyIpEmpty();
    void toJson_keepsEmptyUptimeEmpty();
    void toText_substitutesFallbacksForEmptyFields();
    void toSystemDetailsHtml_pairsEachLabelWithItsValue();
    void toSystemDetailsHtml_namesTheOperatingSystem();
};

namespace {
sysinfo::Info sample()
{
    sysinfo::Info s;
    s.hostname = "host-a";
    s.username = "user-b";
    s.ip       = "10.11.12.13";
    s.uptime   = "01.01.2026 12:00";
    return s;
}
} // namespace

void TestSystemInfoPresenter::toText_formatsAllFields()
{
    const sysinfo::Info s = sample();
    const QString text = sysinfo::presenter::toText(s);

    QVERIFY(text.contains(s.hostname));
    QVERIFY(text.contains(s.username));
    QVERIFY(text.contains(s.ip));
    QVERIFY(text.contains(s.uptime));
}

void TestSystemInfoPresenter::toJson_hasExpectedKeys_andNoLabels()
{
    const sysinfo::Info s = sample();
    const QJsonObject obj = sysinfo::presenter::toJson(s);

    QCOMPARE(obj.value("hostname").toString(), s.hostname);
    QCOMPARE(obj.value("username").toString(), s.username);
    QCOMPARE(obj.value("ip").toString(),       s.ip);
    QCOMPARE(obj.value("uptime").toString(),   s.uptime);
    QVERIFY(!obj.contains("labels"));
}

void TestSystemInfoPresenter::toJsonWithLabels_includesAllLabels()
{
    sysinfo::Info s;
    const QJsonObject obj = sysinfo::presenter::toJsonWithLabels(s);

    QVERIFY(obj.contains("labels"));
    const QJsonObject labels = obj.value("labels").toObject();
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QVERIFY2(labels.contains(key), qPrintable("missing label: " + key));
        QVERIFY2(!labels.value(key).toString().isEmpty(),
                 qPrintable("empty label: " + key));
    }
}

void TestSystemInfoPresenter::toJsonWithLabels_containsSameDataAsToJson()
{
    const sysinfo::Info s = sample();
    const QJsonObject plain   = sysinfo::presenter::toJson(s);
    const QJsonObject labeled = sysinfo::presenter::toJsonWithLabels(s);

    // Adding labels must not alter the data fields.
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QCOMPARE(labeled.value(key).toString(), plain.value(key).toString());
    }
}

void TestSystemInfoPresenter::toJson_keepsEmptyIpEmpty()
{
    sysinfo::Info s = sample();
    s.ip.clear();

    const QJsonObject obj = sysinfo::presenter::toJson(s);

    // The key stays, so the document shape does not depend on what was
    // obtainable; the value stays raw, so no translated placeholder reaches
    // a data field of the API contract.
    QVERIFY2(obj.contains("ip"), "the key must survive an unobtainable value");
    QCOMPARE(obj.value("ip").toString(), QString());
}

void TestSystemInfoPresenter::toJson_keepsEmptyUptimeEmpty()
{
    sysinfo::Info s = sample();
    s.uptime.clear();

    const QJsonObject obj = sysinfo::presenter::toJson(s);

    QVERIFY2(obj.contains("uptime"), "the key must survive an unobtainable value");
    QCOMPARE(obj.value("uptime").toString(), QString());
}

void TestSystemInfoPresenter::toText_substitutesFallbacksForEmptyFields()
{
    sysinfo::Info s = sample();
    s.ip.clear();
    s.uptime.clear();

    const QString text = sysinfo::presenter::toText(s);

    // The plain-text rendering must not contain the sequence "IP address: \n"
    // (= empty value followed by a newline) or "Uptime: " at end of string.
    QVERIFY2(!text.contains("IP address: \n"),
             qPrintable("empty ip leaked into text: " + text));
    QVERIFY2(!text.endsWith("Uptime: "),
             qPrintable("empty uptime leaked into text: " + text));
}

void TestSystemInfoPresenter::toSystemDetailsHtml_pairsEachLabelWithItsValue()
{
    const sysinfo::Info s = sample();
    const QString settingsPath = QStringLiteral("C:/tmp/SysInfo.ini");

    const QString html = sysinfo::presenter::toSystemDetailsHtml(s, settingsPath);

    QVERIFY(html.contains(s.username));
    QVERIFY(html.contains(s.hostname));
    QVERIFY(html.contains(settingsPath));

    // A swapped .arg() order still contains every value, so what pins the
    // pairing down is the order the template lays them out in:
    // OS, User, Device, Settings file.
    QVERIFY2(html.indexOf(s.username) < html.indexOf(s.hostname),
             qPrintable("user and device values are swapped: " + html));
    QVERIFY2(html.indexOf(s.hostname) < html.indexOf(settingsPath),
             qPrintable("device and settings path are swapped: " + html));

    // The block describes the machine, not the session.
    QVERIFY2(!html.contains(s.ip),
             qPrintable("the IP address does not belong here: " + html));
    QVERIFY2(!html.contains(s.uptime),
             qPrintable("the uptime does not belong here: " + html));
}

void TestSystemInfoPresenter::toSystemDetailsHtml_namesTheOperatingSystem()
{
    const QString html =
        sysinfo::presenter::toSystemDetailsHtml(sample(), QStringLiteral("path"));

    QVERIFY(html.contains(QSysInfo::prettyProductName()));
}

QTEST_GUILESS_MAIN(TestSystemInfoPresenter)
#include "tst_system_info_presenter.moc"
