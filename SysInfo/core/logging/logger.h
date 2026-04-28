#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

#ifdef Q_OS_MACOS
#  include <os/log.h>
#endif

/**
 * @brief Cross-platform writer to the system event log.
 *
 * Static-only facade — no instances. The single entry point is log(),
 * which dispatches to the platform-native facility:
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
     * @brief Stable identifier for every distinct event SysInfo can log.
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

        // WARNING
        TSLoadFailed         = 2000,   ///< Translation file present but failed to load.
        TrayIconMissing      = 2001,   ///< Tray icon resource not found.
        ClipboardUnavailable = 2002,   ///< QApplication::clipboard() returned null.
        UiResourceMissing    = 2003,   ///< A UI asset (e.g. animation .gif) is missing.

        // ERROR
        ServerStartError     = 3000,   ///< IntegrationServer failed to bind/listen.
        TrayUnavailable      = 3001,   ///< System tray not available — fatal for SysInfo.
        SettingsWriteFailed  = 3002,   ///< QSettings::sync() reported a write error.
    };

    /**
     * @brief Write a single event to the platform log.
     * @param id  Stable event identifier (also encodes severity).
     * @param msg Free-form human-readable detail in English (not localised —
     *            logs are read by support, not end users).
     */
    static void log(EventId id, const QString& msg);

private:
    /// Internal severity derived from the numeric range of EventId.
    enum class LogSeverity : uint8_t
    {
        Info,
        Warning,
        Error
    };

    static constexpr LogSeverity severityFromEventId(EventId id);

#ifdef Q_OS_WIN
    /// Map our severity to a Win32 ReportEvent type (EVENTLOG_*).
    static unsigned short toWinEventType(LogSeverity severity);
#endif

#ifdef Q_OS_LINUX
    /// Map our severity to a syslog(3) priority (LOG_INFO/WARNING/ERR).
    static int toSyslogPrio(LogSeverity severity);
#endif

#ifdef Q_OS_MACOS
    /// Map our severity to an os_log_type_t.
    static os_log_type_t toMacLogType(LogSeverity severity);
    /// Lazily-initialised os_log_t singleton with subsystem "com.pivdenny.SysInfo".
    static os_log_t osLog();
#endif
};

#endif // LOGGER_H
