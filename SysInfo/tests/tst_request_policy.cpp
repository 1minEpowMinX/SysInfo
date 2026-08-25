#include "services/request_policy.h"

#include <QTest>

using integration::RequestContext;

class TestRequestPolicy : public QObject
{
    Q_OBJECT

private slots:
    void isBrowserExtensionOrigin_acceptsEverySupportedScheme_data();
    void isBrowserExtensionOrigin_acceptsEverySupportedScheme();
    void isBrowserExtensionOrigin_rejectsMalformedValues_data();
    void isBrowserExtensionOrigin_rejectsMalformedValues();

    void isFromBrowser_needsOneOfTheStampedHeaders_data();
    void isFromBrowser_needsOneOfTheStampedHeaders();

    void isContextAllowed_appliesLayerOne_data();
    void isContextAllowed_appliesLayerOne();

    void isClientAllowed_appliesLayerTwo_data();
    void isClientAllowed_appliesLayerTwo();

    void isRequestAllowed_requiresBothLayers();
    void isRequestAllowed_rejectsAWhitelistedIdFromAPageOrigin();

    void effectiveAllowedExtensionIds_fallsBackToTheDefaults();
    void effectiveAllowedExtensionIds_prefersTheOverride();

    void corsAllowOrigin_reflectsAnExtensionOrigin();
    void corsAllowOrigin_isWildcardWithoutOne();
    void corsAllowOrigin_neverReflectsAPageOrigin();
};

namespace {

constexpr const char *kChromeOrigin = "chrome-extension://mjdcgdoembmihkaajaabkffkejompofj";

/// Builds the context of a request from the SysInfo Chrome extension.
RequestContext fromExtension()
{
    return {kChromeOrigin, "none", "cors",
            "mjdcgdoembmihkaajaabkffkejompofj"};
}

/// Builds the context of a request from curl — no browser-stamped header.
RequestContext fromCurl()
{
    return {{}, {}, {}, {}};
}

} // namespace

void TestRequestPolicy::isBrowserExtensionOrigin_acceptsEverySupportedScheme_data()
{
    QTest::addColumn<QByteArray>("origin");

    QTest::newRow("chrome")  << QByteArray("chrome-extension://abcdefghijklmnop");
    QTest::newRow("firefox") << QByteArray("moz-extension://a1b2c3d4-0000-4000-8000-abcdefabcdef");
    QTest::newRow("edge")    << QByteArray("edge-extension://abcdefghijklmnop");
}

void TestRequestPolicy::isBrowserExtensionOrigin_acceptsEverySupportedScheme()
{
    QFETCH(QByteArray, origin);
    QVERIFY(integration::isBrowserExtensionOrigin(origin));
}

void TestRequestPolicy::isBrowserExtensionOrigin_rejectsMalformedValues_data()
{
    QTest::addColumn<QByteArray>("origin");

    QTest::newRow("empty")            << QByteArray();
    QTest::newRow("page https")       << QByteArray("https://evil.com");
    QTest::newRow("page http")        << QByteArray("http://localhost:8734");
    QTest::newRow("scheme only")      << QByteArray("chrome-extension://");
    QTest::newRow("unknown scheme")   << QByteArray("safari-extension://abcdef");
    QTest::newRow("path separator")   << QByteArray("chrome-extension://abc/../evil");
    QTest::newRow("embedded space")   << QByteArray("chrome-extension://abc def");
    QTest::newRow("crlf injection")   << QByteArray("chrome-extension://abc\r\nX-Evil: 1");
    QTest::newRow("over long")        << QByteArray("chrome-extension://" + QByteArray(65, 'a'));
}

void TestRequestPolicy::isBrowserExtensionOrigin_rejectsMalformedValues()
{
    QFETCH(QByteArray, origin);

    // This value is echoed into Access-Control-Allow-Origin, so anything the
    // check lets through reaches a response header verbatim.
    QVERIFY2(!integration::isBrowserExtensionOrigin(origin),
             qPrintable("accepted: " + origin));
}

void TestRequestPolicy::isFromBrowser_needsOneOfTheStampedHeaders_data()
{
    QTest::addColumn<QByteArray>("origin");
    QTest::addColumn<QByteArray>("secFetchSite");
    QTest::addColumn<bool>("expected");

    QTest::newRow("neither")        << QByteArray()      << QByteArray()            << false;
    QTest::newRow("origin only")    << QByteArray("x")   << QByteArray()            << true;
    QTest::newRow("sec-fetch only") << QByteArray()      << QByteArray("cross-site")<< true;
    QTest::newRow("both")           << QByteArray("x")   << QByteArray("none")      << true;
}

void TestRequestPolicy::isFromBrowser_needsOneOfTheStampedHeaders()
{
    QFETCH(QByteArray, origin);
    QFETCH(QByteArray, secFetchSite);
    QFETCH(bool, expected);

    const RequestContext context{origin, secFetchSite, {}, {}};
    QCOMPARE(integration::isFromBrowser(context), expected);
}

void TestRequestPolicy::isContextAllowed_appliesLayerOne_data()
{
    QTest::addColumn<QByteArray>("origin");
    QTest::addColumn<QByteArray>("secFetchMode");
    QTest::addColumn<bool>("expected");

    QTest::newRow("extension cors")   << QByteArray(kChromeOrigin) << QByteArray("cors")     << true;
    QTest::newRow("no headers")       << QByteArray()              << QByteArray()           << true;
    QTest::newRow("page origin")      << QByteArray("https://evil.com") << QByteArray("cors")<< false;
    QTest::newRow("navigate")         << QByteArray()              << QByteArray("navigate") << false;

    // Address-bar navigation is refused even from an extension origin: the
    // point of the filter is keeping the JSON out of browser history.
    QTest::newRow("extension navigate")
        << QByteArray(kChromeOrigin) << QByteArray("navigate") << false;
}

void TestRequestPolicy::isContextAllowed_appliesLayerOne()
{
    QFETCH(QByteArray, origin);
    QFETCH(QByteArray, secFetchMode);
    QFETCH(bool, expected);

    const RequestContext context{origin, {}, secFetchMode, {}};
    QCOMPARE(integration::isContextAllowed(context), expected);
}

void TestRequestPolicy::isClientAllowed_appliesLayerTwo_data()
{
    QTest::addColumn<QByteArray>("origin");
    QTest::addColumn<QByteArray>("clientId");
    QTest::addColumn<bool>("expected");

    const QByteArray known = "mjdcgdoembmihkaajaabkffkejompofj";

    QTest::newRow("curl, no id")        << QByteArray()  << QByteArray()      << true;
    QTest::newRow("curl, known id")     << QByteArray()  << known             << true;
    QTest::newRow("curl, unknown id")   << QByteArray()  << QByteArray("nope")<< false;
    QTest::newRow("browser, no id")     << QByteArray(kChromeOrigin) << QByteArray()      << false;
    QTest::newRow("browser, known id")  << QByteArray(kChromeOrigin) << known             << true;
    QTest::newRow("browser, other id")  << QByteArray(kChromeOrigin) << QByteArray("nope")<< false;
}

void TestRequestPolicy::isClientAllowed_appliesLayerTwo()
{
    QFETCH(QByteArray, origin);
    QFETCH(QByteArray, clientId);
    QFETCH(bool, expected);

    const RequestContext context{origin, {}, {}, clientId};
    QCOMPARE(integration::isClientAllowed(context, integration::defaultAllowedExtensionIds()),
             expected);
}

void TestRequestPolicy::isRequestAllowed_requiresBothLayers()
{
    const QStringList allowed = integration::defaultAllowedExtensionIds();

    QVERIFY(integration::isRequestAllowed(fromExtension(), allowed));
    QVERIFY(integration::isRequestAllowed(fromCurl(), allowed));
}

void TestRequestPolicy::isRequestAllowed_rejectsAWhitelistedIdFromAPageOrigin()
{
    // The header is a claim, so a page that copies a published extension id
    // must still be stopped — by layer 1, which it cannot forge.
    RequestContext context = fromExtension();
    context.origin = "https://evil.com";

    QVERIFY(!integration::isRequestAllowed(context,
                                           integration::defaultAllowedExtensionIds()));
}

void TestRequestPolicy::effectiveAllowedExtensionIds_fallsBackToTheDefaults()
{
    QCOMPARE(integration::effectiveAllowedExtensionIds({}),
             integration::defaultAllowedExtensionIds());
}

void TestRequestPolicy::effectiveAllowedExtensionIds_prefersTheOverride()
{
    const QStringList configured = {QStringLiteral("only-this-one")};

    // The override replaces the defaults rather than extending them, so an
    // administrator can retire a published id.
    const QStringList effective = integration::effectiveAllowedExtensionIds(configured);
    QCOMPARE(effective, configured);
    QVERIFY(!effective.contains(integration::defaultAllowedExtensionIds().first()));
}

void TestRequestPolicy::corsAllowOrigin_reflectsAnExtensionOrigin()
{
    QCOMPARE(integration::corsAllowOrigin(fromExtension()), QByteArray(kChromeOrigin));
}

void TestRequestPolicy::corsAllowOrigin_isWildcardWithoutOne()
{
    QCOMPARE(integration::corsAllowOrigin(fromCurl()), QByteArray("*"));
}

void TestRequestPolicy::corsAllowOrigin_neverReflectsAPageOrigin()
{
    const RequestContext context{"https://evil.com", {}, {}, {}};

    // Reached only on an approved request, but reflecting a page origin would
    // hand that page read access if the layers were ever reordered.
    QCOMPARE(integration::corsAllowOrigin(context), QByteArray("*"));
}

QTEST_GUILESS_MAIN(TestRequestPolicy)
#include "tst_request_policy.moc"
