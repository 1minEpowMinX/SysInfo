#pragma once

#ifdef Q_OS_MAC

#include <functional>

/**
 * @brief Thin C++ wrapper around UNUserNotificationCenter for macOS.
 *
 * All callbacks are delivered on the main thread.
 */
class MacOSNotifications
{
public:
    enum class Status {
        NotDetermined,
        Authorized,
        Denied
    };

    /** Asynchronously query the current notification authorization status. */
    static void getStatus(std::function<void(Status)> callback);

    /** Request notification authorization from the OS.
     *  The OS dialog is shown at most once; subsequent calls return the
     *  stored decision without prompting the user again. */
    static void requestPermission(std::function<void(bool granted)> callback);

    /** Open System Settings → Notifications so the user can re-enable manually. */
    static void openSystemNotificationSettings();
};

#endif // Q_OS_MAC
