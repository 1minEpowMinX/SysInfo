#ifndef WELCOME_NOTIFIER_H
#define WELCOME_NOTIFIER_H

#include <QObject>

class SettingsManager;
class TrayController;

class WelcomeNotifier : public QObject
{
    Q_OBJECT
public:
    explicit WelcomeNotifier(TrayController& tray,
                             SettingsManager& settings,
                             QObject* parent = nullptr);

    void scheduleShow(int delayMs = 60'000);

signals:
    void trayGuideRequested();

private:
    void onTimerFired();

    TrayController& m_tray;
    SettingsManager& m_settings;
};

#endif // WELCOME_NOTIFIER_H
