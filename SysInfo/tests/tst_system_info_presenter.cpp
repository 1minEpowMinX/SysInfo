#include "core/sysinfo/system_info.h"
#include "core/sysinfo/system_info_presenter.h"

#include <QJsonObject>
#include <QTest>

class TestSystemInfoPresenter : public QObject
{
    Q_OBJECT

private slots:
    void toText_formatsAllFields();
    void toJson_hasExpectedKeys_andNoLabels();
    void toJsonWithLabels_includesAllLabels();
    void toJsonWithLabels_containsSameDataAsToJson();
};

namespace {
Utils::SystemInfo sample()
{
    Utils::SystemInfo s;
    s.hostname = "host-a";
    s.username = "user-b";
    s.ip       = "10.11.12.13";
    s.uptime   = "01.01.2026 12:00";
    return s;
}
} // namespace

void TestSystemInfoPresenter::toText_formatsAllFields()
{
    const Utils::SystemInfo s = sample();
    const QString text = Utils::Presenter::toText(s);

    QVERIFY(text.contains(s.hostname));
    QVERIFY(text.contains(s.username));
    QVERIFY(text.contains(s.ip));
    QVERIFY(text.contains(s.uptime));
}

void TestSystemInfoPresenter::toJson_hasExpectedKeys_andNoLabels()
{
    const Utils::SystemInfo s = sample();
    const QJsonObject obj = Utils::Presenter::toJson(s);

    QCOMPARE(obj.value("hostname").toString(), s.hostname);
    QCOMPARE(obj.value("username").toString(), s.username);
    QCOMPARE(obj.value("ip").toString(),       s.ip);
    QCOMPARE(obj.value("uptime").toString(),   s.uptime);
    QVERIFY(!obj.contains("labels"));
}

void TestSystemInfoPresenter::toJsonWithLabels_includesAllLabels()
{
    Utils::SystemInfo s;
    const QJsonObject obj = Utils::Presenter::toJsonWithLabels(s);

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
    const Utils::SystemInfo s = sample();
    const QJsonObject plain   = Utils::Presenter::toJson(s);
    const QJsonObject labeled = Utils::Presenter::toJsonWithLabels(s);

    // Adding labels must not alter the data fields.
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QCOMPARE(labeled.value(key).toString(), plain.value(key).toString());
    }
}

QTEST_GUILESS_MAIN(TestSystemInfoPresenter)
#include "tst_system_info_presenter.moc"
