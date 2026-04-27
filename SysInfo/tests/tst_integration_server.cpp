#include "services/integration_server.h"

#include <QByteArray>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTest>
#include <QTimer>
#include <QUrl>

namespace {

struct HttpResult {
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    int statusCode = 0;
    QByteArray body;
    QList<QPair<QByteArray, QByteArray>> headers;
};

HttpResult httpGet(QNetworkAccessManager &nam, const QUrl &url,
                   int timeoutMs = 3000)
{
    QNetworkRequest req(url);
    QNetworkReply *reply = nam.get(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeoutMs);

    loop.exec();

    HttpResult r;
    if (!reply->isFinished()) {
        reply->abort();
        r.error = QNetworkReply::TimeoutError;
    } else {
        r.error = reply->error();
        r.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        r.body = reply->readAll();
        for (const auto &pair : reply->rawHeaderPairs()) {
            r.headers.append(pair);
        }
    }
    reply->deleteLater();
    return r;
}

QByteArray header(const HttpResult &r, const QByteArray &name)
{
    for (const auto &p : r.headers) {
        if (p.first.compare(name, Qt::CaseInsensitive) == 0)
            return p.second;
    }
    return {};
}

} // namespace

class TestIntegrationServer : public QObject
{
    Q_OBJECT

private slots:
    void startStop_togglesListening();
    void start_withZeroPort_bindsEphemeral();
    void statusEndpoint_returnsOk();
    void systemInfoEndpoint_returnsJsonWithLabels();
    void corsHeaders_arePresent();
};

void TestIntegrationServer::startStop_togglesListening()
{
    IntegrationServer server;
    QVERIFY(!server.isListening());

    QVERIFY(server.start(0));
    QVERIFY(server.isListening());
    QVERIFY(server.boundPort() > 0);

    server.stop();
    QVERIFY(!server.isListening());
}

void TestIntegrationServer::start_withZeroPort_bindsEphemeral()
{
    IntegrationServer a;
    IntegrationServer b;
    QVERIFY(a.start(0));
    QVERIFY(b.start(0));
    QVERIFY(a.boundPort() != b.boundPort());
}

void TestIntegrationServer::statusEndpoint_returnsOk()
{
    IntegrationServer server;
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const QUrl url(QString("http://127.0.0.1:%1/status").arg(server.boundPort()));
    const HttpResult r = httpGet(nam, url);

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);
    QCOMPARE(r.body.trimmed(), QByteArray("OK"));
}

void TestIntegrationServer::systemInfoEndpoint_returnsJsonWithLabels()
{
    IntegrationServer server;
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const QUrl url(QString("http://127.0.0.1:%1/systeminfo").arg(server.boundPort()));
    const HttpResult r = httpGet(nam, url);

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);

    const QByteArray contentType = header(r, "Content-Type");
    QVERIFY2(contentType.contains("application/json"),
             qPrintable("unexpected Content-Type: " + contentType));

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(r.body, &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());

    const QJsonObject obj = doc.object();
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QVERIFY2(obj.contains(key), qPrintable("missing field: " + key));
    }

    QVERIFY(obj.contains("labels"));
    const QJsonObject labels = obj.value("labels").toObject();
    QVERIFY(!labels.isEmpty());
}

void TestIntegrationServer::corsHeaders_arePresent()
{
    IntegrationServer server;
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const QUrl url(QString("http://127.0.0.1:%1/status").arg(server.boundPort()));
    const HttpResult r = httpGet(nam, url);

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(header(r, "Access-Control-Allow-Origin"), QByteArray("*"));
    QVERIFY(!header(r, "Access-Control-Allow-Methods").isEmpty());
    QVERIFY(!header(r, "Access-Control-Allow-Headers").isEmpty());
}

QTEST_GUILESS_MAIN(TestIntegrationServer)
#include "tst_integration_server.moc"
