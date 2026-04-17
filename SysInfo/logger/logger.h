#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

#ifdef Q_OS_MACOS
#  include <os/log.h>
#endif

class Logger
{
public:
    enum class EventId : uint16_t
    {
        /*
        1000–1999 : info
        2000–2999 : warning
        3000–3999 : error
        */

        // INFO
        AppStart = 1000,
        AppExit = 1001,

        // WARNING
        TSLoadFailed = 2000,
        TrayIconMissing = 2001,
        ClipboardUnavailable = 2002,
        UiResourceMissing = 2003,

        // ERROR
        ServerStartError = 3000,
        TrayUnavailable = 3001,
        SettingsWriteFailed = 3002,
    };

    static void log(EventId id, const QString& msg);

private:
    enum class LogSeverity : uint8_t
    {
        Info,
        Warning,
        Error
    };

    static constexpr LogSeverity severityFromEventId(EventId id);

#ifdef Q_OS_WIN
    static unsigned short toWinEventType(LogSeverity severity);
#endif

#ifdef Q_OS_LINUX
    static int toSyslogPrio(LogSeverity severity);
#endif

#ifdef Q_OS_MACOS
    static os_log_type_t toMacLogType(LogSeverity severity);
    static os_log_t osLog();
#endif
};

#endif // LOGGER_H