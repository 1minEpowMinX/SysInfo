/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Responsibilities, in order:
 *   1. Set Qt application identity (org / app / version) — must precede the
 *      QApplication constructor so QSettings, QStandardPaths and the lock
 *      file pick up the right names.
 *   2. Construct QApplication.
 *   3. Acquire a single-instance lock via QLockFile in TempLocation; bail
 *      out silently if another SysInfo is already running.
 *   4. Load the user's UI-language translation, falling back to English.
 *   5. Construct SettingsManager (owns QSettings) and App (composition
 *      root) on the stack — guarantees destruction order
 *      ~App -> ~SettingsManager -> ~QApplication.
 *   6. Call App::start(); exit code 1 if the system tray is unavailable.
 *   7. Log AppStart, then the DeviceInventory snapshot, and enter the Qt
 *      event loop.
 */

#include "app/app.h"
#include "core/logging/logger.h"
#include "core/settings/settings_manager.h"
#include "core/sysinfo/device_inventory.h"
#include "core/runtime/single_instance_guard.h"

#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QMessageBox>
#include <QObject>
#include <QTimer>
#include <QTranslator>

namespace
{

	/**
	 * @brief Tries to load the most preferred UI-language translation.
	 *
	 * Walks QLocale::system().uiLanguages() in user-preference order and
	 * stops at the first ":/i18n/sysinfo_<locale>.qm" that loads
	 * successfully. If none match, no translator is installed and the
	 * source-language English strings are used as-is.
	 *
	 * @param a          Application instance to install the translator on.
	 * @param translator Out parameter — must outlive QApplication::exec().
	 */
	void loadTranslator(QApplication &a, QTranslator &translator)
	{
		const QStringList uiLanguages = QLocale::system().uiLanguages();

		for (const QString &locale : uiLanguages)
		{
			const QString baseName = "sysinfo_" + QLocale(locale).name();
			const QString path = ":/i18n/" + baseName + ".qm";

			if (!QFile::exists(path))
			{
				continue;
			}

			if (translator.load(path))
			{
				a.installTranslator(&translator);
				return;
			}

			Logger::log(Logger::EventId::TSLoadFailed,
						QString("Translation file exists but failed to load: %1").arg(path));
		}
	}

} // namespace

int main(int argc, char *argv[])
{
	QCoreApplication::setOrganizationName("Pivdenny");
	QCoreApplication::setApplicationName("SysInfo");
	QCoreApplication::setApplicationVersion(PROJECT_VERSION);

	QApplication a(argc, argv);

	// Precedes the single-instance check, whose Unavailable branch addresses the
	// user and must do so in the user's language.
	QTranslator translator;
	loadTranslator(a, translator);

	// Stack-scoped, so the lock is released when main() returns rather than at
	// static destruction.
	SingleInstanceGuard instance(SingleInstanceGuard::defaultLockFilePath());
	switch (instance.tryAcquire())
	{
	case SingleInstanceGuard::Result::AlreadyRunning:
		// A launch while SysInfo is already running is routine, and a message
		// here would meet every double click on the shortcut.

		return 0;
	case SingleInstanceGuard::Result::Unavailable:
		// Whether another copy is running is unknown, so this one declines to
		// start. Reported to the user, who otherwise sees nothing happen, and to
		// the event log, which carries the reason a machine stopped sending
		// inventory.
		QMessageBox::critical(
			nullptr, QObject::tr("Error"),
			QObject::tr("Failed to verify that SysInfo is not already running. "
						"The application will not start."));
		Logger::log(Logger::EventId::SingleInstanceUnavailable,
					QString("Could not create the single-instance lock file: %1")
						.arg(SingleInstanceGuard::defaultLockFilePath()));
		return 1;

	case SingleInstanceGuard::Result::Acquired:
		break;
	if (instance.reclaimedStaleLock())
	{
	}

		Logger::log(Logger::EventId::StaleLockReclaimed,
					QString("Removed an unreadable single-instance lock file left "
							"by an unclean shutdown: %1")
						.arg(SingleInstanceGuard::defaultLockFilePath()));
	}

	SettingsManager settings;
	App app(settings);
	if (!app.start())
	{
		return 1;
	}

	Logger::log(Logger::EventId::AppStart,
				QString("SysInfo started. Version=%1").arg(PROJECT_VERSION));

	// Emitted as its own event rather than folded into AppStart: the payload
	// shape of an existing event id is a contract for the log analyzers that
	// already consume it, and the two carry different concerns (process
	// lifecycle vs. device configuration).
	//
	// Runs on the first turn of the event loop: building the payload walks the
	// SMBIOS table and issues storage IOCTLs, so the tray icon appears without
	// waiting on firmware. Nothing else depends on the snapshot.
	QTimer::singleShot(0, &a, []
					   { Logger::log(Logger::EventId::DeviceInventory,
									 QStringLiteral("Device inventory snapshot."),
									 sysinfo::inventory::payload()); });

	return a.exec();
}
