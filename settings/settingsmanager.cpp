#include "settingsmanager.h"

SettingsManager& SettingsManager::instance() {
    static SettingsManager inst;
    return inst;
}

SettingsManager::SettingsManager(QObject *parent)
    : QObject{parent}, m_settings("Pivdenny", "SysInfo")
{}

bool SettingsManager::showWelcome() const {
    return m_settings.value("General/ShowWelcome", true).toBool();
}

void SettingsManager::setShowWelcome(bool value) {
    m_settings.setValue("General/ShowWelcome", value);
}

bool SettingsManager::showTrayGuide() const {
    return m_settings.value("General/ShowTrayGuide", true).toBool();
}

void SettingsManager::setShowTrayGuide(bool value) {
    m_settings.setValue("General/ShowTrayGuide", value);
}
