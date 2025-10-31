#include "app/app.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSystemSemaphore>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const QString sharedKey = "SysInfoMutex"; // For correct operation, the name must match version 1

    // A semaphore is needed to avoid race conditions when multiple processes start simultaneously
    QSystemSemaphore semaphore(sharedKey + "_sem", 1);
    semaphore.acquire();

    QSharedMemory sharedMemory(sharedKey);
    bool isAlreadyRunning = false;

    if (!sharedMemory.create(1)) {
        // If the memory already exists, it means that the program has already been launched
        isAlreadyRunning = true;
    }

    semaphore.release();

    if (isAlreadyRunning) {
        // Just finish the application if there is already an instance
        return 0;
    }

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "sysinfo_" + QLocale(locale).name();
        if (translator.load(":resources/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    App::instance().startApp();

    return a.exec();
}
