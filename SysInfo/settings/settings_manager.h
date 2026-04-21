#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>

class SettingsManager
{
public:
    SettingsManager();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
    SettingsManager(SettingsManager&&) = delete;
    SettingsManager& operator=(SettingsManager&&) = delete;

    bool showWelcome() const;
    void setShowWelcome(bool value);
    bool showTrayGuide() const;
    void setShowTrayGuide(bool value);

private:
    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
