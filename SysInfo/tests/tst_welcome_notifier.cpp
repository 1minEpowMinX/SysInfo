#include "app/notification_sink.h"
#include "app/welcome_notifier.h"
#include "core/settings/onboarding_flags.h"

#include <QSignalSpy>
#include <QTest>

namespace {

/// Records what WelcomeNotifier asked for and lets a test act as the user
/// clicking a notification.
class FakeSink : public NotificationSink
{
public:
    struct Shown
    {
        QString title;
        QString body;
        int msecs = 0;
    };

    void showNotification(const QString &title, const QString &body, int msecs) override
    {
        shown.append({title, body, msecs});
    }

    /// Stands in for the user clicking whatever notification is on screen.
    void click() { emit notificationClicked(); }

    QList<Shown> shown;
};

/// Holds the two flags in memory, so no test reaches the developer's real
/// settings store. QSettings on Windows is the registry, which neither
/// QStandardPaths test mode nor QSettings::setDefaultFormat() redirects.
class FakeFlags : public OnboardingFlags
{
public:
    bool showWelcome() const override { return welcome; }
    void setShowWelcome(bool value) override { welcome = value; }
    bool showTrayGuide() const override { return trayGuide; }

    bool welcome   = true;
    bool trayGuide = true;
};

/// Short enough that a full welcome-then-hint sequence costs milliseconds
/// rather than the 40 seconds the production values would.
constexpr WelcomeNotifier::Lifetimes kFastLifetimes{30, 30};

/// Covers a notification's whole lifetime plus the timer that follows it.
constexpr int kSettleMs = 200;

} // namespace

class TestWelcomeNotifier : public QObject
{
    Q_OBJECT

private slots:
    void scheduleShow_showsTheWelcomeMessage();
    void scheduleShow_clearsTheWelcomeFlag();
    void welcomeShownTwice_isSuppressedBySecondRun();
    void welcomeAlreadySeen_goesStraightToTheHint();
    void hintFollowsTheWelcome_neverOverlappingIt();
    void clickDuringTheHint_requestsTheGuide();
    void clickAfterTheHintExpired_isIgnored();
    void clickDuringTheWelcome_doesNotRequestTheGuide();
    void trayGuideAlreadyDismissed_showsNoHint();
};

void TestWelcomeNotifier::scheduleShow_showsTheWelcomeMessage()
{
    FakeSink sink;
    FakeFlags flags;
    WelcomeNotifier notifier(sink, flags, kFastLifetimes);

    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    QCOMPARE(sink.shown.first().msecs, kFastLifetimes.welcomeMs);
    QVERIFY(!sink.shown.first().title.isEmpty());
    QVERIFY(!sink.shown.first().body.isEmpty());
}

void TestWelcomeNotifier::scheduleShow_clearsTheWelcomeFlag()
{
    FakeSink sink;
    FakeFlags flags;
    QVERIFY(flags.showWelcome());

    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    notifier.scheduleShow(0);

    QTRY_VERIFY(!flags.showWelcome());
}

void TestWelcomeNotifier::welcomeShownTwice_isSuppressedBySecondRun()
{
    FakeSink sink;
    FakeFlags flags;

    {
        WelcomeNotifier first(sink, flags, kFastLifetimes);
        first.scheduleShow(0);
        QTRY_VERIFY(!sink.shown.isEmpty());
    }
    const QString welcomeTitle = sink.shown.first().title;
    QTest::qWait(kSettleMs);
    sink.shown.clear();

    WelcomeNotifier second(sink, flags, kFastLifetimes);
    second.scheduleShow(0);
    QTest::qWait(kSettleMs);

    for (const FakeSink::Shown &shown : sink.shown) {
        QVERIFY2(shown.title != welcomeTitle,
                 "the welcome message must fire at most once per profile");
    }
}

void TestWelcomeNotifier::welcomeAlreadySeen_goesStraightToTheHint()
{
#ifndef Q_OS_WINDOWS
    QSKIP("the tray-pinning hint is Windows-only");
#endif
    FakeSink sink;
    FakeFlags flags;
    flags.welcome = false;

    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    // Straight to the hint means the first notification already carries the
    // hint's lifetime, with no welcome ahead of it.
    QCOMPARE(sink.shown.size(), 1);
    QCOMPARE(sink.shown.first().msecs, kFastLifetimes.trayGuideMs);
}

void TestWelcomeNotifier::hintFollowsTheWelcome_neverOverlappingIt()
{
#ifndef Q_OS_WINDOWS
    QSKIP("the tray-pinning hint is Windows-only");
#endif
    FakeSink sink;
    FakeFlags flags;
    WelcomeNotifier notifier(sink, flags, kFastLifetimes);

    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    // Only the welcome so far: the hint waits for it to expire, so that one
    // notification at most is ever clickable.
    QCOMPARE(sink.shown.size(), 1);
    QCOMPARE(sink.shown.first().msecs, kFastLifetimes.welcomeMs);

    QTRY_COMPARE(sink.shown.size(), 2);
    QCOMPARE(sink.shown.at(1).msecs, kFastLifetimes.trayGuideMs);
}

void TestWelcomeNotifier::clickDuringTheHint_requestsTheGuide()
{
#ifndef Q_OS_WINDOWS
    QSKIP("the tray-pinning hint is Windows-only");
#endif
    FakeSink sink;
    FakeFlags flags;
    flags.welcome = false;

    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    QSignalSpy requested(&notifier, &WelcomeNotifier::trayGuideRequested);

    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    sink.click();
    QCOMPARE(requested.count(), 1);
}

void TestWelcomeNotifier::clickAfterTheHintExpired_isIgnored()
{
#ifndef Q_OS_WINDOWS
    QSKIP("the tray-pinning hint is Windows-only");
#endif
    FakeSink sink;
    FakeFlags flags;
    flags.welcome = false;

    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    QSignalSpy requested(&notifier, &WelcomeNotifier::trayGuideRequested);

    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    // Past the hint's lifetime the subscription is gone, so a click now
    // belongs to some other notification and must not open the guide.
    QTest::qWait(kSettleMs);
    sink.click();

    QCOMPARE(requested.count(), 0);
}

void TestWelcomeNotifier::clickDuringTheWelcome_doesNotRequestTheGuide()
{
    FakeSink sink;
    FakeFlags flags;
    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    QSignalSpy requested(&notifier, &WelcomeNotifier::trayGuideRequested);

    notifier.scheduleShow(0);
    QTRY_VERIFY(!sink.shown.isEmpty());

    // The subscription starts with the hint, so the welcome is not clickable
    // into the guide.
    sink.click();
    QCOMPARE(requested.count(), 0);
}

void TestWelcomeNotifier::trayGuideAlreadyDismissed_showsNoHint()
{
    FakeSink sink;
    FakeFlags flags;
    flags.welcome = false;
    flags.trayGuide = false;

    WelcomeNotifier notifier(sink, flags, kFastLifetimes);
    notifier.scheduleShow(0);
    QTest::qWait(kSettleMs);

    QVERIFY2(sink.shown.isEmpty(),
             "a dismissed hint and a seen welcome leave nothing to show");
}

QTEST_GUILESS_MAIN(TestWelcomeNotifier)
#include "tst_welcome_notifier.moc"
