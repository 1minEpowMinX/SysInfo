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

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QLockFile>
#include <QStandardPaths>
#include <QTranslator>

namespace
{

	/**
	 * @brief Lazily-initialised lock file used to enforce single-instance behaviour.
	 *
	 * Held by reference for the entire lifetime of the process so the lock
	 * is released only on exit. Stale-lock detection is disabled
	 * (setStaleLockTime(0)) — if a previous SysInfo crashed, the user can
	 * delete the lock file manually rather than us silently stealing it.
	 */
	QLockFile &singleInstanceLock()
	{
		static QLockFile lock(
			QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
				.absoluteFilePath("SysInfo.lock"));
		lock.setStaleLockTime(0);
		return lock;
	}

	/// @return true if this process is the first SysInfo instance, false otherwise.
	bool acquireSingleInstance()
	{
		return singleInstanceLock().tryLock(100);
	}

	/**
	 * @brief Try to load the most preferred UI-language translation.
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

	if (!acquireSingleInstance())
	{
		return 0;
	}

	QTranslator translator;
	loadTranslator(a, translator);

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
	Logger::log(Logger::EventId::DeviceInventory,
				QStringLiteral("Device inventory snapshot."),
				sysinfo::inventory::payload());

	return a.exec();
}
