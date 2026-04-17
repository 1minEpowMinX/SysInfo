#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>

class SettingsManager
{
public:
    static SettingsManager& instance();

    bool showWelcome() const;
    void setShowWelcome(bool value);
    bool showTrayGuide() const;
    void setShowTrayGuide(bool value);

private:
    SettingsManager();

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
