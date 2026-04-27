#include "app/app.h"
#include "core/logging/logger.h"
#include "core/settings/settings_manager.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QLockFile>
#include <QStandardPaths>
#include <QTranslator>

namespace
{

	QLockFile &singleInstanceLock()
	{
		static QLockFile lock(
			QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
				.absoluteFilePath("SysInfo.lock"));
		lock.setStaleLockTime(0);
		return lock;
	}

	bool acquireSingleInstance()
	{
		return singleInstanceLock().tryLock(100);
	}

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

	return a.exec();
}
