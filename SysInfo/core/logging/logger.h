#ifndef LOGGER_H
#define LOGGER_H

#include <QJsonObject>
#include <QString>

#ifdef Q_OS_MACOS
#  include <os/log.h>
#endif

/**
 * @brief Writes events to the platform-native system log.
 *
 * Static-only facade — no instances. The entry point is log() (optionally
 * with a structured payload), which dispatches to the platform-native
 * facility:
 *   - Windows: ReportEvent (the application event log; the SysInfo source
 *     is registered by the installer, not by this code).
 *   - Linux:   syslog(3).
 *   - macOS:   os_log (unified logging system).
 *
 * Severity is derived from the numeric range of the EventId, not passed
 * separately, to keep call sites concise (Logger::log(EventId::Foo, msg)).
 */
class Logger
{
public:
    /**
     * @brief Identifies every distinct event SysInfo can log.
     *
     * The numeric range encodes severity:
     *   - 1000–1999 : Info
     *   - 2000–2999 : Warning
     *   - 3000–3999 : Error
     *
     * Add new entries at the end of the appropriate range; do not renumber
     * existing ones — these IDs may end up in support tickets and external
     * log analyzers.
     */
    enum class EventId : uint16_t
    {
        // INFO
        AppStart             = 1000,   ///< Application started successfully.
        AppExit              = 1001,   ///< User-initiated graceful exit.
        DeviceInventory      = 1002,   ///< Hardware/boot snapshot; carries a JSON payload.

        // WARNING
        TSLoadFailed         = 2000,   ///< Translation file present but failed to load.
        TrayIconMissing      = 2001,   ///< Tray icon resource not found.
        ClipboardUnavailable = 2002,   ///< QApplication::clipboard() returned null.
        UiResourceMissing    = 2003,   ///< A UI asset (e.g. animation .gif) is missing.
        StaleLockReclaimed   = 2004,   ///< Unreadable single-instance lock file removed at startup.

        // ERROR
        ServerStartError          = 3000,   ///< IntegrationServer failed to bind/listen.
        TrayUnavailable           = 3001,   ///< System tray not available — fatal for SysInfo.
        SettingsWriteFailed       = 3002,   ///< QSettings::sync() reported a write error.
        SingleInstanceUnavailable = 3003,   ///< Lock file uncreatable — startup aborted.
    };

    /**
     * @brief Writes a single event to the platform log.
     * @param id  Stable event identifier (also encodes severity).
     * @param msg Free-form human-readable detail in English (not localised —
     *            logs are read by support, not end users).
     */
    static void log(EventId id, const QString& msg);

    /**
     * @brief Writes an event carrying a machine-readable payload alongside @p msg.
     *
     * Log shippers (Winlogbeat on Windows, Filebeat elsewhere) forward the
     * payload to Elasticsearch, where each key becomes an aggregatable field.
     * It is emitted as a value of its own rather than interpolated into @p msg
     * so that consumers never have to grok a sentence apart:
     *   - Windows: a second insertion string, surfaced as event_data.param2;
     *   - Linux/macOS: appended after @p msg, separated by a single space.
     *
     * @param id   Stable event identifier (also encodes severity).
     * @param msg  Human-readable summary, same rules as the overload above.
     * @param data Payload object; serialised compactly. An empty object is
     *             written as "{}" — pass none by using the other overload.
     */
    static void log(EventId id, const QString& msg, const QJsonObject& data);

private:
    /**
     * @brief Implements both log() overloads.
     * @param id      Stable event identifier (also encodes severity).
     * @param msg     Human-readable summary, prefixed with a UTC timestamp and
     *                the numeric id before it reaches the platform facility.
     * @param payload Compact JSON, or empty to emit no payload at all.
     */
    static void write(EventId id, const QString& msg, const QString& payload);

    /// Enumerates severity levels, derived from the numeric range of EventId.
    enum class LogSeverity : uint8_t
    {
        Info,
        Warning,
        Error
    };

    /// Maps @p id onto the severity encoded in its numeric range.
    static constexpr LogSeverity severityFromEventId(EventId id);

#ifdef Q_OS_WIN
    /// Maps severity to a Win32 ReportEvent type (EVENTLOG_*).
    static unsigned short toWinEventType(LogSeverity severity);
#endif

#ifdef Q_OS_LINUX
    /// Maps severity to a syslog(3) priority (LOG_INFO/WARNING/ERR).
    static int toSyslogPrio(LogSeverity severity);
#endif

#ifdef Q_OS_MACOS
    /// Maps severity to an os_log_type_t.
    static os_log_type_t toMacLogType(LogSeverity severity);
    /// Returns the lazily-initialised os_log_t for subsystem "com.pivdenny.SysInfo".
    static os_log_t osLog();
#endif
};

#endif // LOGGER_H
