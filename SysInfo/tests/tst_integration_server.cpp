#include "services/integration_server.h"
#include "services/request_policy.h"
#include "core/sysinfo/info_source.h"
#include "core/settings/extension_whitelist.h"

#include <QByteArray>
#include <QEventLoop>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTest>
#include <QTimer>
#include <QUrl>

namespace {

/// Serves the whitelist override straight from memory, so no test reaches the
/// developer's real settings store.
class FakeWhitelist : public ExtensionWhitelist
{
public:
    QStringList allowedExtensionIds() const override { return ids; }

    QStringList ids;
};

struct HttpResult {
    QNetworkReply::NetworkError error = QNetworkReply::NoError;
    int statusCode = 0;
    QByteArray body;
    QList<QPair<QByteArray, QByteArray>> headers;
};

/**
 * @brief Performs a one-shot HTTP request with custom method and headers.
 *
 * Built on QNetworkAccessManager — sufficient for these integration tests
 * because Qt allows raw-setting Origin / X-Sysinfo-Client / Sec-Fetch-*
 * (none of them are on QNAM's restricted-headers list).
 */
HttpResult httpRequest(QNetworkAccessManager &nam, const QUrl &url,
                       const QByteArray &method,
                       const QHash<QByteArray, QByteArray> &headers,
                       int timeoutMs = 3000)
{
    QNetworkRequest req(url);
    for (auto it = headers.cbegin(); it != headers.cend(); ++it) {
        req.setRawHeader(it.key(), it.value());
    }

    QNetworkReply *reply = (method == "GET")
            ? nam.get(req)
            : nam.sendCustomRequest(req, method);

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

HttpResult httpGet(QNetworkAccessManager &nam, const QUrl &url,
                   int timeoutMs = 3000)
{
    return httpRequest(nam, url, "GET", {}, timeoutMs);
}

QByteArray header(const HttpResult &r, const QByteArray &name)
{
    for (const auto &p : r.headers) {
        if (p.first.compare(name, Qt::CaseInsensitive) == 0)
            return p.second;
    }
    return {};
}

// Read off the policy rather than restated, so these cannot drift from the
// list the server actually serves.
const QByteArray kChromeProdId =
    integration::defaultAllowedExtensionIds().at(0).toUtf8();
const QByteArray kFirefoxProdId =
    integration::defaultAllowedExtensionIds().at(1).toUtf8();

QUrl statusUrl(const IntegrationServer &server)
{
    return QUrl(QString("http://127.0.0.1:%1/status").arg(server.boundPort()));
}

QUrl versionUrl(const IntegrationServer &server)
{
    return QUrl(QString("http://127.0.0.1:%1/version").arg(server.boundPort()));
}

QUrl systeminfoUrl(const IntegrationServer &server)
{
    return QUrl(QString("http://127.0.0.1:%1/systeminfo").arg(server.boundPort()));
}

} // namespace

class TestIntegrationServer : public QObject
{
    Q_OBJECT

private slots:
    // Lifecycle
    void startStop_togglesListening();
    void restart_afterStop_servesAgain();
    void start_withZeroPort_bindsEphemeral();

    // Happy paths via plain GET (non-browser context — no Origin)
    void statusEndpoint_returnsOk();
    void versionEndpoint_returnsJsonWithVersionAndBuild();
    void versionEndpoint_isRejectedFromBrowserWithoutClientId();
    void systemInfoForNonBrowser_returnsJsonWithLabels();
    void systemInfoForExtension_returnsJsonWithoutLabels();
    void corsHeaders_arePresentForNonBrowser();

    // Context filter (Origin + Sec-Fetch-Mode)
    void pageOrigin_isRejected();
    void navigateMode_isRejected();
    void malformedExtensionOrigin_isRejected_data();
    void malformedExtensionOrigin_isRejected();

    // Client whitelist (X-Sysinfo-Client)
    void extensionOriginWithoutClientId_isRejected();
    void extensionOriginWithBogusClientId_isRejected();
    void extensionOriginWithChromeDefaultId_returnsOk();
    void extensionOriginWithFirefoxDefaultId_returnsOk();

    // Preflight (OPTIONS)
    void preflightFromExtension_returnsNoContent();
    void preflightFromPage_isRejected();
    void preflightResponse_includesXSysinfoClientInAllowHeaders();

    // CORS Allow-Origin reflection
    void corsAllowOrigin_reflectsExtensionOrigin();
    void corsAllowOrigin_isWildcardForNonBrowser();

    // Settings override
    void settingsOverride_replacesDefaults();
};

// --- Lifecycle ---------------------------------------------------------------

void TestIntegrationServer::startStop_togglesListening()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(!server.isListening());

    QVERIFY(server.start(0));
    QVERIFY(server.isListening());
    QVERIFY(server.boundPort() > 0);

    server.stop();
    QVERIFY(!server.isListening());
}

// stop() closes the socket while the route table and the bind survive it, so the second start()
// serves from the same routes rather than needing them rebuilt.
void TestIntegrationServer::restart_afterStop_servesAgain()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QNetworkAccessManager nam;

    QVERIFY(server.start(0));
    QCOMPARE(httpGet(nam, statusUrl(server)).body, QByteArray("OK"));

    server.stop();
    QVERIFY(server.start(0));
    QVERIFY(server.isListening());

    const HttpResult r = httpGet(nam, statusUrl(server));
    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);
    QCOMPARE(r.body, QByteArray("OK"));
}

void TestIntegrationServer::start_withZeroPort_bindsEphemeral()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer a(whitelist, info);
    IntegrationServer b(whitelist, info);
    QVERIFY(a.start(0));
    QVERIFY(b.start(0));
    QVERIFY(a.boundPort() != b.boundPort());
}

// --- Happy paths via plain GET (no Origin) -----------------------------------

void TestIntegrationServer::statusEndpoint_returnsOk()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, statusUrl(server));

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);
    QCOMPARE(r.body.trimmed(), QByteArray("OK"));
}

void TestIntegrationServer::versionEndpoint_returnsJsonWithVersionAndBuild()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, versionUrl(server));

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
    // Both fields must be present and non-empty. Compared exactly against
    // the same PROJECT_VERSION / BUILD_DATE the server is built against,
    // so a CMake project(VERSION) bump that wasn't propagated would fail
    // this test loudly.
    QCOMPARE(obj.value("version").toString(),
             QString::fromUtf8(PROJECT_VERSION));
    QCOMPARE(obj.value("build").toString(),
             QString::fromUtf8(BUILD_DATE));
}

void TestIntegrationServer::versionEndpoint_isRejectedFromBrowserWithoutClientId()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    // Same gate as the other endpoints — a browser caller without a
    // valid X-Sysinfo-Client must NOT learn the server version.
    const HttpResult r = httpRequest(nam, versionUrl(server), "GET",
        {{"Origin", QByteArray("chrome-extension://") + kChromeProdId}});

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::systemInfoForNonBrowser_returnsJsonWithLabels()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    // No Origin / Sec-Fetch-* — non-browser caller (curl, support).
    // Should receive labels for human-readable inspection.
    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, systeminfoUrl(server));

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
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QVERIFY2(labels.contains(key),
                 qPrintable("missing label: " + key));
    }
}

void TestIntegrationServer::systemInfoForExtension_returnsJsonWithoutLabels()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    // Browser caller (extension SW). The extension localises labels
    // client-side via browser.i18n, so the server omits the "labels"
    // object — smaller wire payload, less duplicated translation logic.
    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET", {
        {"Origin",            QByteArray("chrome-extension://") + kChromeProdId},
        {"X-Sysinfo-Client",  kChromeProdId},
    });

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(r.body, &parseError);
    QCOMPARE(parseError.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());

    const QJsonObject obj = doc.object();
    for (const QString &key : {"hostname", "username", "ip", "uptime"}) {
        QVERIFY2(obj.contains(key), qPrintable("missing field: " + key));
    }
    QVERIFY2(!obj.contains("labels"),
             "browser caller should NOT receive labels — extension i18n owns them");
}

void TestIntegrationServer::corsHeaders_arePresentForNonBrowser()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, statusUrl(server));

    QCOMPARE(r.error, QNetworkReply::NoError);
    // No Origin sent ⇒ wildcard fallback.
    QCOMPARE(header(r, "Access-Control-Allow-Origin"), QByteArray("*"));
    QVERIFY(!header(r, "Access-Control-Allow-Methods").isEmpty());
    QVERIFY(!header(r, "Access-Control-Allow-Headers").isEmpty());
}

// --- Context filter ----------------------------------------------------------

void TestIntegrationServer::pageOrigin_isRejected()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET",
                                     {{"Origin", "https://evil.com"}});

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::navigateMode_isRejected()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET",
                                     {{"Sec-Fetch-Mode", "navigate"}});

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::malformedExtensionOrigin_isRejected_data()
{
    QTest::addColumn<QByteArray>("origin");

    // The extension scheme alone is not enough: whatever follows it is
    // echoed back in Access-Control-Allow-Origin, so the identifier must
    // look like one before it is served — and to reflect — it.
    QTest::newRow("empty id")
        << QByteArray("chrome-extension://");
    QTest::newRow("path traversal")
        << QByteArray("chrome-extension://abcdef/../../evil");
    QTest::newRow("dotted host")
        << QByteArray("moz-extension://evil.com");
    QTest::newRow("wildcard")
        << QByteArray("chrome-extension://*");
    QTest::newRow("comma-separated second origin")
        << QByteArray("chrome-extension://abcdef,https://evil.com");
    QTest::newRow("over-long id")
        << (QByteArray("edge-extension://") + QByteArray(200, 'a'));
}

void TestIntegrationServer::malformedExtensionOrigin_isRejected()
{
    QFETCH(QByteArray, origin);

    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET", {
        {"Origin",           origin},
        {"X-Sysinfo-Client", kChromeProdId},
    });

    QCOMPARE(r.statusCode, 403);
    // Nothing may be reflected back for a rejected origin.
    QVERIFY2(header(r, "Access-Control-Allow-Origin") != origin,
             "a rejected origin must never be echoed into the CORS header");
}

// --- Client whitelist --------------------------------------------------------

void TestIntegrationServer::extensionOriginWithoutClientId_isRejected()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    // Old extension build / sibling extension that didn't learn about
    // X-Sysinfo-Client. Origin alone is not enough.
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET",
        {{"Origin", QByteArray("chrome-extension://") + kChromeProdId}});

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::extensionOriginWithBogusClientId_isRejected()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET", {
        {"Origin",            "chrome-extension://aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
        {"X-Sysinfo-Client",  "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"},
    });

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::extensionOriginWithChromeDefaultId_returnsOk()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            QByteArray("chrome-extension://") + kChromeProdId},
        {"X-Sysinfo-Client",  kChromeProdId},
    });

    QCOMPARE(r.statusCode, 200);
    QCOMPARE(r.body.trimmed(), QByteArray("OK"));
}

void TestIntegrationServer::extensionOriginWithFirefoxDefaultId_returnsOk()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    // Firefox stamps Sec-Fetch-Site: cross-site for extension SW fetches —
    // verify the new Origin-based policy still lets this through.
    const HttpResult r = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            "moz-extension://abcdef-1234-5678-90ab-cdef12345678"},
        {"Sec-Fetch-Site",    "cross-site"},
        {"Sec-Fetch-Mode",    "cors"},
        {"X-Sysinfo-Client",  kFirefoxProdId},
    });

    QCOMPARE(r.statusCode, 200);
}

// --- Preflight (OPTIONS) -----------------------------------------------------

void TestIntegrationServer::preflightFromExtension_returnsNoContent()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    // Preflight by spec carries no X-Sysinfo-Client, only the
    // Access-Control-Request-* hints. Must still pass the context gate.
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "OPTIONS", {
        {"Origin",                          "moz-extension://abcdef-1234-5678"},
        {"Access-Control-Request-Method",   "GET"},
        {"Access-Control-Request-Headers",  "x-sysinfo-client"},
    });

    QCOMPARE(r.statusCode, 204);
}

void TestIntegrationServer::preflightFromPage_isRejected()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "OPTIONS", {
        {"Origin",                          "https://evil.com"},
        {"Access-Control-Request-Method",   "GET"},
        {"Access-Control-Request-Headers",  "x-sysinfo-client"},
    });

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::preflightResponse_includesXSysinfoClientInAllowHeaders()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "OPTIONS", {
        {"Origin",                          "moz-extension://abcdef-1234-5678"},
        {"Access-Control-Request-Method",   "GET"},
        {"Access-Control-Request-Headers",  "x-sysinfo-client"},
    });

    QCOMPARE(r.statusCode, 204);
    const QByteArray allowHeaders = header(r, "Access-Control-Allow-Headers");
    QVERIFY2(allowHeaders.contains("X-Sysinfo-Client"),
             qPrintable("Allow-Headers must permit X-Sysinfo-Client; got: "
                        + allowHeaders));
}

// --- CORS Allow-Origin reflection -------------------------------------------

void TestIntegrationServer::corsAllowOrigin_reflectsExtensionOrigin()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    const QByteArray origin = "moz-extension://abcdef-1234-5678";

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            origin},
        {"X-Sysinfo-Client",  kFirefoxProdId},
    });

    QCOMPARE(r.statusCode, 200);
    QCOMPARE(header(r, "Access-Control-Allow-Origin"), origin);
    QCOMPARE(header(r, "Vary"), QByteArray("Origin"));
}

void TestIntegrationServer::corsAllowOrigin_isWildcardForNonBrowser()
{
    FakeWhitelist whitelist;
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, statusUrl(server));

    QCOMPARE(r.statusCode, 200);
    QCOMPARE(header(r, "Access-Control-Allow-Origin"), QByteArray("*"));
}

// --- Settings override -------------------------------------------------------

void TestIntegrationServer::settingsOverride_replacesDefaults()
{
    FakeWhitelist whitelist;
    whitelist.ids = QStringList{"corp-custom-extension-id"};
    sysinfo::InfoSource info;
    IntegrationServer server(whitelist, info);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;

    // The compiled-in default IDs must NOT be honoured anymore — override
    // *replaces* defaults rather than augmenting them.
    const HttpResult prodReject = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            QByteArray("chrome-extension://") + kChromeProdId},
        {"X-Sysinfo-Client",  kChromeProdId},
    });
    QCOMPARE(prodReject.statusCode, 403);

    // The custom ID from the override should pass.
    const HttpResult customAllow = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            "chrome-extension://corp-custom-extension-id"},
        {"X-Sysinfo-Client",  "corp-custom-extension-id"},
    });
    QCOMPARE(customAllow.statusCode, 200);
}

QTEST_GUILESS_MAIN(TestIntegrationServer)
#include "tst_integration_server.moc"
