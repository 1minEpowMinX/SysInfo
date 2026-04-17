#include "settings_manager.h"
#include "../logger/logger.h"

#include <QObject>

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
    m_settings.sync();

    if(m_settings.status() != QSettings::NoError) {
        Logger::log(Logger::EventId::SettingsWriteFailed,
                    "SettingsManager: failed to write General/ShowWelcome settings.");
    }
}

bool SettingsManager::showTrayGuide() const {
    return m_settings.value("General/ShowTrayGuide", true).toBool();
}

void SettingsManager::setShowTrayGuide(bool value) {
    m_settings.setValue("General/ShowTrayGuide", value);
    m_settings.sync();

    if(m_settings.status() != QSettings::NoError) {
        Logger::log(Logger::EventId::SettingsWriteFailed,
                    "SettingsManager: failed to write General/ShowTrayGuide settings.");
    }
}
