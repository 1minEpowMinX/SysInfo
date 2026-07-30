#ifndef EXTENSION_WHITELIST_H
#define EXTENSION_WHITELIST_H

#include <QStringList>

/**
 * @brief Supplies the administrator override of the browser-extension whitelist.
 *
 * The interface IntegrationServer consults, so that which clients are trusted
 * is separable from where that answer is stored. SettingsManager is the
 * implementation the application runs with, backed by QSettings.
 *
 * Carries the override alone and not the effective whitelist: an empty list
 * means "not configured", and the compiled-in defaults that stand in for it
 * belong to IntegrationServer.
 */
class ExtensionWhitelist
{
public:
    virtual ~ExtensionWhitelist() = default;

    /**
     * @brief Reads the configured browser-extension IDs.
     *
     * Read fresh on each call, so an edit to the store takes effect without
     * restarting SysInfo.
     *
     * @return The configured IDs, or an empty list when the override is unset.
     */
    virtual QStringList allowedExtensionIds() const = 0;
};

#endif // EXTENSION_WHITELIST_H
