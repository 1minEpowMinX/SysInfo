#include "logger.h"

#include <QByteArray>
#include <QDateTime>
#include <QString>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#ifdef Q_OS_LINUX
#  include <syslog.h>
#endif

#ifdef Q_OS_MACOS
#  include <os/log.h>
#endif

constexpr Logger::LogSeverity Logger::severityFromEventId(EventId id)
{
    const uint16_t value = static_cast<uint16_t>(id);

    if (value >= 3000)
        return LogSeverity::Error;
    if (value >= 2000)
        return LogSeverity::Warning;

    return LogSeverity::Info;
}

#ifdef Q_OS_WIN
unsigned short Logger::toWinEventType(LogSeverity severity)
{
    switch (severity)
    {
    case LogSeverity::Error:
        return EVENTLOG_ERROR_TYPE;
    case LogSeverity::Warning:
        return EVENTLOG_WARNING_TYPE;
    case LogSeverity::Info:
    default:
        return EVENTLOG_INFORMATION_TYPE;
    }
}
#endif

#ifdef Q_OS_LINUX
int Logger::toSyslogPrio(LogSeverity severity)
{
    switch (severity)
    {
    case LogSeverity::Error:
        return LOG_ERR;
    case LogSeverity::Warning:
        return LOG_WARNING;
    case LogSeverity::Info:
    default:
        return LOG_INFO;
    }
}
#endif

#ifdef Q_OS_MACOS
os_log_type_t Logger::toMacLogType(LogSeverity severity)
{
    switch (severity)
    {
    case LogSeverity::Error:
        return OS_LOG_TYPE_ERROR;
    case LogSeverity::Warning:
        // macOS unified logging has no dedicated warning type;
        // use DEFAULT as the closest severity between INFO and ERROR.
        return OS_LOG_TYPE_DEFAULT;
    case LogSeverity::Info:
    default:
        return OS_LOG_TYPE_INFO;
    }
}

os_log_t Logger::osLog()
{
    static os_log_t s_log = os_log_create("com.pivdenny.SysInfo", "general");
    return s_log;
}
#endif

void Logger::log(EventId id, const QString& msg)
{
    const LogSeverity severity = severityFromEventId(id);

    const QString fullMessage = QString("[%1] [%2] %3")
                                    .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),
                                         QString::number(static_cast<uint16_t>(id)),
                                         msg);

#ifdef Q_OS_WIN
    HANDLE eventSource = RegisterEventSourceW(nullptr, L"SysInfo");
    if (eventSource != nullptr)
    {
        const std::wstring wideMessage = fullMessage.toStdWString();
        LPCWSTR strings[] = { wideMessage.c_str() };

        ReportEventW(
            eventSource,                                    // Event handler
            toWinEventType(severity),                       // Event type
            0,                                              // Category
            static_cast<DWORD>(static_cast<uint16_t>(id)),  // Event ID
            nullptr,                                        // User SID
            1,                                              // The number of placeholders for strings
            0,                                              // The number of bytes of event-specific data
            strings,                                        // Array of pointers to string
            nullptr                                         // A pointer to the buffer containing the binary data
            );

        DeregisterEventSource(eventSource);
    }

#elif defined(Q_OS_LINUX)
    const QByteArray utf8Message = fullMessage.toUtf8();
    syslog(toSyslogPrio(severity) | LOG_USER, "%s", utf8Message.constData());

#elif defined(Q_OS_MACOS)
    const QByteArray utf8Message = fullMessage.toUtf8();

    os_log_with_type(
        osLog(),
        toMacLogType(severity),
        "%{public}s",
        utf8Message.constData());

#else
    Q_UNUSED(id)
    Q_UNUSED(msg)
#endif
}