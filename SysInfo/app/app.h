#ifndef APP_H
#define APP_H

#include <QObject>
#include <QString>

class IntegrationServer;
class SettingsManager;
class TrayController;
class WelcomeNotifier;

class App : public QObject
{
    Q_OBJECT

public:
    explicit App(SettingsManager& settings, QObject* parent = nullptr);

    [[nodiscard]] bool start();

private slots:
    void onCopyRequested();
    void onAboutRequested();
    void onQuitRequested();
    void onTrayGuideRequested();

private:
    void startTrayUpdateTimer();

    SettingsManager& m_settings;
    TrayController* m_tray = nullptr;
    WelcomeNotifier* m_notifier = nullptr;
    IntegrationServer* m_server = nullptr;
    QString m_cachedInfo;
};

#endif // APP_H
