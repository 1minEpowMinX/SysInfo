#include "settings_manager.h"
#include "core/logging/logger.h"

// Consumers take one of the two ports rather than this class, which is what
// lets a test substitute either without a QSettings behind it. filePath() sits
// outside both: its answer is a fixed string a caller is handed, not an
// interface it queries. Copy and move are deleted because QSettings holds OS
// resources that should not be duplicated.

SettingsManager::SettingsManager()
    : m_settings("Pivdenny", "SysInfo")
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

QString SettingsManager::filePath() const {
    return m_settings.fileName();
}

QStringList SettingsManager::allowedExtensionIds() const {
    return m_settings.value("Integration/AllowedExtensionIds").toStringList();
}
