#include "services/integration_server.h"
#include "core/settings/settings_manager.h"

#include <QByteArray>
#include <QEventLoop>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QStandardPaths>
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

/**
 * @brief One-shot HTTP request with custom method and headers.
 *
 * Built on QNetworkAccessManager — sufficient for our integration tests
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

// Built-in default extension IDs (kept in sync with kDefaultAllowedExtensionIds
// in integration_server.cpp). Tests rely on these matching.
constexpr const char *kChromeProdId = "mjdcgdoembmihkaajaabkffkejompofj";
constexpr const char *kFirefoxProdId = "sysinfo-addon@pivdenny.ua";

QUrl statusUrl(const IntegrationServer &server)
{
    return QUrl(QString("http://127.0.0.1:%1/status").arg(server.boundPort()));
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
    void initTestCase();
    void cleanup();

    // Lifecycle
    void startStop_togglesListening();
    void start_withZeroPort_bindsEphemeral();

    // Happy paths via plain GET (non-browser context — no Origin)
    void statusEndpoint_returnsOk();
    void systemInfoEndpoint_returnsJsonWithLabels();
    void corsHeaders_arePresentForNonBrowser();

    // Context filter (Origin + Sec-Fetch-Mode)
    void pageOrigin_isRejected();
    void navigateMode_isRejected();

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

void TestIntegrationServer::initTestCase()
{
    // Isolate QSettings("Pivdenny", "SysInfo") so settingsOverride_* tests
    // don't touch the developer's real registry/ini file.
    QStandardPaths::setTestModeEnabled(true);
}

void TestIntegrationServer::cleanup()
{
    // Wipe any test-mode settings between cases so override leakage cannot
    // affect the next test's whitelist.
    QSettings("Pivdenny", "SysInfo").clear();
}

// --- Lifecycle ---------------------------------------------------------------

void TestIntegrationServer::startStop_togglesListening()
{
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(!server.isListening());

    QVERIFY(server.start(0));
    QVERIFY(server.isListening());
    QVERIFY(server.boundPort() > 0);

    server.stop();
    QVERIFY(!server.isListening());
}

void TestIntegrationServer::start_withZeroPort_bindsEphemeral()
{
    SettingsManager settings;
    IntegrationServer a(settings);
    IntegrationServer b(settings);
    QVERIFY(a.start(0));
    QVERIFY(b.start(0));
    QVERIFY(a.boundPort() != b.boundPort());
}

// --- Happy paths via plain GET (no Origin) -----------------------------------

void TestIntegrationServer::statusEndpoint_returnsOk()
{
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, statusUrl(server));

    QCOMPARE(r.error, QNetworkReply::NoError);
    QCOMPARE(r.statusCode, 200);
    QCOMPARE(r.body.trimmed(), QByteArray("OK"));
}

void TestIntegrationServer::systemInfoEndpoint_returnsJsonWithLabels()
{
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

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
}

void TestIntegrationServer::corsHeaders_arePresentForNonBrowser()
{
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET",
                                     {{"Origin", "https://evil.com"}});

    QCOMPARE(r.statusCode, 403);
}

void TestIntegrationServer::navigateMode_isRejected()
{
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpRequest(nam, systeminfoUrl(server), "GET",
                                     {{"Sec-Fetch-Mode", "navigate"}});

    QCOMPARE(r.statusCode, 403);
}

// --- Client whitelist --------------------------------------------------------

void TestIntegrationServer::extensionOriginWithoutClientId_isRejected()
{
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
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
    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;
    const HttpResult r = httpGet(nam, statusUrl(server));

    QCOMPARE(r.statusCode, 200);
    QCOMPARE(header(r, "Access-Control-Allow-Origin"), QByteArray("*"));
}

// --- Settings override -------------------------------------------------------

void TestIntegrationServer::settingsOverride_replacesDefaults()
{
    // Prepopulate isolated test settings with a custom whitelist BEFORE
    // constructing SettingsManager / IntegrationServer.
    {
        QSettings s("Pivdenny", "SysInfo");
        s.setValue("Integration/AllowedExtensionIds",
                   QStringList{"corp-custom-extension-id"});
        s.sync();
    }

    SettingsManager settings;
    IntegrationServer server(settings);
    QVERIFY(server.start(0));

    QNetworkAccessManager nam;

    // The compiled-in default IDs must NOT be honoured anymore — override
    // *replaces* defaults rather than augmenting them.
    const HttpResult prodReject = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            QByteArray("chrome-extension://") + kChromeProdId},
        {"X-Sysinfo-Client",  kChromeProdId},
    });
    QCOMPARE(prodReject.statusCode, 403);

    // The custom ID from settings should pass.
    const HttpResult customAllow = httpRequest(nam, statusUrl(server), "GET", {
        {"Origin",            "chrome-extension://corp-custom-extension-id"},
        {"X-Sysinfo-Client",  "corp-custom-extension-id"},
    });
    QCOMPARE(customAllow.statusCode, 200);
}

QTEST_GUILESS_MAIN(TestIntegrationServer)
#include "tst_integration_server.moc"
