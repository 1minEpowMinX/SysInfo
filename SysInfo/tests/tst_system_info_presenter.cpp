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
    void toAboutFacts_carriesEachFieldToItsOwnMember();
    void toAboutFacts_namesTheOperatingSystem();
    void toAboutFacts_keepsEmptyFieldsEmpty();
};

namespace {
sysinfo::Info sample()
{
    sysinfo::Info s;
    s.hostname     = "host-a";
    s.username     = "user-b";
    s.ip           = "10.11.12.13";
    s.lastBootTime = "01.01.2026 12:00";
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
    QVERIFY(text.contains(s.lastBootTime));
}

void TestSystemInfoPresenter::toJson_hasExpectedKeys_andNoLabels()
{
    const sysinfo::Info s = sample();
    const QJsonObject obj = sysinfo::presenter::toJson(s);

    QCOMPARE(obj.value("hostname").toString(), s.hostname);
    QCOMPARE(obj.value("username").toString(), s.username);
    QCOMPARE(obj.value("ip").toString(),       s.ip);
    QCOMPARE(obj.value("uptime").toString(),   s.lastBootTime);
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
    s.lastBootTime.clear();

    const QJsonObject obj = sysinfo::presenter::toJson(s);

    QVERIFY2(obj.contains("uptime"), "the key must survive an unobtainable value");
    QCOMPARE(obj.value("uptime").toString(), QString());
}

void TestSystemInfoPresenter::toText_substitutesFallbacksForEmptyFields()
{
    sysinfo::Info s = sample();
    s.ip.clear();
    s.lastBootTime.clear();

    const QString text = sysinfo::presenter::toText(s);

    // The plain-text rendering must not contain the sequence "IP address: \n"
    // (= empty value followed by a newline) or "Last boot: " at end of string.
    QVERIFY2(!text.contains("IP address: \n"),
             qPrintable("empty ip leaked into text: " + text));
    QVERIFY2(!text.endsWith("Last boot: "),
             qPrintable("empty boot time leaked into text: " + text));
}

void TestSystemInfoPresenter::toAboutFacts_carriesEachFieldToItsOwnMember()
{
    const sysinfo::Info s = sample();
    const QString settingsPath =
        QStringLiteral("\\HKEY_CURRENT_USER\\Software\\Pivdenny\\SysInfo");

    const sysinfo::AboutFacts facts =
        sysinfo::presenter::toAboutFacts(s, settingsPath);

    // Username and hostname are both plain strings of the same shape, so a
    // swapped assignment would still populate every member; comparing them
    // member by member is what pins the mapping down.
    QCOMPARE(facts.username, s.username);
    QCOMPARE(facts.hostname, s.hostname);
    QCOMPARE(facts.settingsFilePath, settingsPath);

    // The panel describes the machine, not the session.
    QVERIFY2(!facts.operatingSystem.contains(s.ip),
             "the IP address does not belong here");
    QVERIFY2(!facts.operatingSystem.contains(s.lastBootTime),
             "the boot time does not belong here");
}

void TestSystemInfoPresenter::toAboutFacts_namesTheOperatingSystem()
{
    const sysinfo::AboutFacts facts =
        sysinfo::presenter::toAboutFacts(sample(), QStringLiteral("path"));

    QCOMPARE(facts.operatingSystem, QSysInfo::prettyProductName());
}

void TestSystemInfoPresenter::toAboutFacts_keepsEmptyFieldsEmpty()
{
    // Unlike toText(), this rendering substitutes no placeholder: the About
    // panel is read literally during support work, and a translated stand-in
    // there would be indistinguishable from a real value.
    sysinfo::Info s;
    s.hostname = QString();
    s.username = QString();

    const sysinfo::AboutFacts facts =
        sysinfo::presenter::toAboutFacts(s, QString());

    QVERIFY(facts.username.isEmpty());
    QVERIFY(facts.hostname.isEmpty());
    QVERIFY(facts.settingsFilePath.isEmpty());
}

QTEST_GUILESS_MAIN(TestSystemInfoPresenter)
#include "tst_system_info_presenter.moc"
