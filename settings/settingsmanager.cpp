#include "settingsmanager.h"

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject{parent}, m_settings("Pivdenny", "SysInfo")
{}

bool SettingsManager::showGuide() const {
    return m_settings.value("General/ShowGuide", true).toBool();
}

void SettingsManager::setShowGuide(bool value) {
    m_settings.setValue("General/ShowGuide", value);
}
