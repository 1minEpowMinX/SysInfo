#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Utils {
QString GetHostname();
QString GetUsername();
QString GetActiveIPAddress();
QString GetLastBootTime();
QString GetSystemInfo();
}

#endif // UTILS_H
