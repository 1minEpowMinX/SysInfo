#include "app/app.h"
#include "logger/logger.h"

#include <QApplication>
#include <QFile>
#include <QLocale>
#include <QTranslator>
#include <QSystemSemaphore>
#include <QSharedMemory>

bool isAlreadyRunning()
{
    const QString sharedKey = "SysInfoMutex";
    // A semaphore is needed to avoid race conditions when multiple processes start simultaneously
    QSystemSemaphore semaphore(sharedKey + "_sem", 1);
    semaphore.acquire();

    static QSharedMemory sharedMemory(sharedKey);
    const bool alreadyRunning = !sharedMemory.create(1);
    semaphore.release();

    return alreadyRunning;
}

void loadTranslator(QApplication &a, QTranslator &translator)
{
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    QString triedLocale;

    for (const QString &locale : uiLanguages) {
        const QString baseName = "sysinfo_" + QLocale(locale).name();
        const QString path = ":resources/i18n/" + baseName + ".qm";

        if (!QFile::exists(path)) {
            continue;
        }

        triedLocale = baseName;
        if (translator.load(path)) {
            a.installTranslator(&translator);
            return;
        }

        Logger::log(Logger::EventId::TSLoadFailed,
                    QString("Translation file exists but failed to load: %1").arg(path));
    }
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (isAlreadyRunning()) {
        return 0;
    }

    QTranslator translator;
    loadTranslator(a, translator);

    App::instance().startApp();
    Logger::log(Logger::EventId::AppStart,
                QString("SysInfo started. Version=%1").arg(PROJECT_VERSION));

    return a.exec();
}
