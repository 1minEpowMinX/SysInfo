#include "logger.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonDocument>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <syslog.h>
#endif

#ifdef Q_OS_MACOS
#include <os/log.h>
#endif

// A static facade and not a port, unlike the rest of SysInfo's collaborators:
// call sites stay free of a logging parameter and no constructor widens to
// carry one, at the price that a write cannot be observed from a test. That a
// failing QSettings::sync() produces SettingsWriteFailed, or a refused bind
// ServerStartError, is asserted nowhere — injecting the facility is what those
// assertions would cost.

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
namespace
{

	/**
	 * @brief Appends @p payload to @p message, space-separated.
	 *
	 * syslog and os_log carry one flat string, so the JSON payload rides at
	 * the end of the line. Keeping it last, and separated by a single space,
	 * is what lets Filebeat split the message from the object it must decode.
	 */
	QString joinPayload(const QString &message, const QString &payload)
	{
		return payload.isEmpty() ? message : message + QLatin1Char(' ') + payload;
	}

} // namespace
#endif

constexpr Logger::LogSeverity Logger::severityFromEventId(EventId id)
{
	const uint16_t value = static_cast<uint16_t>(id);

	if (value >= 3000)
	{
		return LogSeverity::Error;
	}
	if (value >= 2000)
	{
		return LogSeverity::Warning;
	}

	return LogSeverity::Info;
}

#ifdef Q_OS_WIN
namespace
{

	/**
	 * @brief Returns the "SysInfo" event source handle, opened once per process.
	 *
	 * RegisterEventSourceW is an RPC to the Event Log service, so the handle is
	 * obtained on first use rather than per message. It stays valid for the life
	 * of the process and is released when the static is destroyed at exit.
	 *
	 * A non-null handle is not by itself proof that anything gets recorded:
	 * unless the installer has registered the source under
	 * HKLM\SYSTEM\CurrentControlSet\Services\EventLog\<log>\SysInfo, the
	 * service accepts the writes and discards them.
	 *
	 * @return Event source handle, or nullptr if registration failed.
	 */
	HANDLE eventSourceHandle()
	{
		struct Source
		{
			HANDLE handle = RegisterEventSourceW(nullptr, L"SysInfo");

			~Source()
			{
				if (handle != nullptr)
				{
					DeregisterEventSource(handle);
				}
			}
		};

		static Source source;
		return source.handle;
	}

} // namespace

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

/// Opens the syslog connection under the identity "SysInfo", once per process.
static void ensureSyslogOpen()
{
	static const bool opened = []()
	{
		openlog("SysInfo", LOG_PID | LOG_NDELAY, LOG_USER);
		return true;
	}();
	Q_UNUSED(opened);
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

void Logger::log(EventId id, const QString &msg)
{
	write(id, msg, QString());
}

void Logger::log(EventId id, const QString &msg, const QJsonObject &data)
{
	write(id, msg,
		  QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact)));
}

void Logger::write(EventId id, const QString &msg, const QString &payload)
{
	const LogSeverity severity = severityFromEventId(id);

	const QString fullMessage = QString("[%1] [%2] %3")
									.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs),
										 QString::number(static_cast<uint16_t>(id)),
										 msg);

#ifdef Q_OS_WIN
	const HANDLE eventSource = eventSourceHandle();
	if (eventSource != nullptr)
	{
		// The payload travels as its own insertion string so that log
		// shippers can read it as a discrete field instead of grokking a
		// sentence apart to recover it.
		const std::wstring wideMessage = fullMessage.toStdWString();
		const std::wstring widePayload = payload.toStdWString();
		LPCWSTR strings[] = {wideMessage.c_str(), widePayload.c_str()};

		// The count decides how much of `strings` the service reads: one
		// string for a bare message, two when a payload rides along.
		ReportEventW(eventSource,
					 toWinEventType(severity),
					 0, // No category: the source registers no category file.
					 static_cast<DWORD>(static_cast<uint16_t>(id)),
					 nullptr, // No user SID.
					 payload.isEmpty() ? WORD(1) : WORD(2),
					 0, // No binary data.
					 strings,
					 nullptr);
	}

#elif defined(Q_OS_LINUX)
	ensureSyslogOpen();
	const QByteArray utf8Message = joinPayload(fullMessage, payload).toUtf8();
	syslog(toSyslogPrio(severity), "%s", utf8Message.constData());

#elif defined(Q_OS_MACOS)
	const QByteArray utf8Message = joinPayload(fullMessage, payload).toUtf8();

	os_log_with_type(
		osLog(),
		toMacLogType(severity),
		"%{public}s",
		utf8Message.constData());

#else
	Q_UNUSED(id)
	Q_UNUSED(msg)
	Q_UNUSED(payload)
#endif
}