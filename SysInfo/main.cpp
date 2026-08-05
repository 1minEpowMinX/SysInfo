/**
 * @file main.cpp
 * @brief Starts the application and enters the Qt event loop.
 *
 * The composition root: everything App and the integration server are handed
 * is built here, on the stack, and torn down in reverse.
 */

#include "app/app.h"
#include "core/logging/logger.h"
#include "core/runtime/single_instance_guard.h"
#include "core/settings/settings_manager.h"
#include "core/sysinfo/device_inventory.h"
#include "core/sysinfo/info_source.h"
#include "services/integration_server.h"
#include "ui/message_box_prompt.h"
#include "ui/tray_controller.h"
#include "ui/widget_dialogs.h"

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

	QTranslator translator;
	loadTranslator(a, translator);

	SingleInstanceGuard instance(SingleInstanceGuard::defaultLockFilePath());
	switch (instance.tryAcquire())
	{
	case SingleInstanceGuard::Result::AlreadyRunning:
		return 0;

	case SingleInstanceGuard::Result::Unavailable:
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
	}

	if (instance.reclaimedStaleLock())
	{
		Logger::log(Logger::EventId::StaleLockReclaimed,
					QString("Removed an unreadable single-instance lock file left "
							"by an unclean shutdown: %1")
						.arg(SingleInstanceGuard::defaultLockFilePath()));
	}

	SettingsManager settings;
	MessageBoxPrompt prompt;
	TrayController tray;
	WidgetDialogs dialogs;
	sysinfo::InfoSource info;
    // The settings store is handed to each consumer as the interface that consumer takes
	IntegrationServer server(settings, info);
	App app(settings, server, prompt, tray, dialogs, info, settings.filePath());
	if (!app.start())
	{
		return 1;
	}

	Logger::log(Logger::EventId::AppStart,
				QString("SysInfo started. Version=%1").arg(PROJECT_VERSION));

    // Async device snapshot to avoid application start delay.
	QTimer::singleShot(0, &a, []
					   { Logger::log(Logger::EventId::DeviceInventory,
									 QStringLiteral("Device inventory snapshot."),
									 sysinfo::inventory::payload()); });

	return a.exec();
}
