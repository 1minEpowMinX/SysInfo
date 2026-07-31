#ifndef SETTINGS_LOCATION_H
#define SETTINGS_LOCATION_H

#include <QString>

/**
 * @brief Reports where the settings store keeps its file.
 *
 * The interface App consults to name the store in the About dialog, so the
 * composition layer needs no access to the store's contents to do it.
 * SettingsManager is the implementation the application runs with.
 *
 * Carries the location and nothing else: what the file holds is reached
 * through OnboardingFlags and ExtensionWhitelist, and a reader who only has
 * this interface cannot get at either.
 */
class SettingsLocation
{
public:
    virtual ~SettingsLocation() = default;

    /**
     * @brief Names the file the settings live in.
     *
     * Useful for the About dialog, support tickets and manual cleanup.
     * Provides a single point of access — callers must NOT instantiate their
     * own QSettings to work this out.
     *
     * @return Absolute path of the on-disk settings file.
     */
    virtual QString filePath() const = 0;
};

#endif // SETTINGS_LOCATION_H
