#ifdef Q_OS_MAC

#include "macos_notifications.h"

#import <UserNotifications/UserNotifications.h>
#import <AppKit/AppKit.h>

void MacOSNotifications::getStatus(std::function<void(Status)> callback)
{
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
        dispatch_async(dispatch_get_main_queue(), ^{
            switch (settings.authorizationStatus) {
                case UNAuthorizationStatusAuthorized:
                case UNAuthorizationStatusProvisional:
                    callback(Status::Authorized);
                    break;
                case UNAuthorizationStatusDenied:
                    callback(Status::Denied);
                    break;
                default: // UNAuthorizationStatusNotDetermined / Ephemeral
                    callback(Status::NotDetermined);
                    break;
            }
        });
    }];
}

void MacOSNotifications::requestPermission(std::function<void(bool)> callback)
{
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
    UNAuthorizationOptions options =
        UNAuthorizationOptionAlert |
        UNAuthorizationOptionSound;
    [center requestAuthorizationWithOptions:options
                          completionHandler:^(BOOL granted, NSError *) {
        dispatch_async(dispatch_get_main_queue(), ^{
            callback(granted);
        });
    }];
}

void MacOSNotifications::openSystemNotificationSettings()
{
    NSURL *url = [NSURL URLWithString:@"x-apple.systempreferences:com.apple.preference.notifications"];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

#endif // Q_OS_MAC
