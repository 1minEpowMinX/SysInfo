#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>

class SettingsManager : public QObject
{
public:
    static SettingsManager& instance();

    bool showGuide() const;
    void setShowGuide(bool value);

private:
    explicit SettingsManager(QObject *parent = nullptr);
    QSettings m_settings;

};

#endif // SETTINGSMANAGER_H
