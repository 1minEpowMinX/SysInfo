#ifndef UTILS_H
#define UTILS_H

#include <QString>

namespace Utils {
QString getHostname();
QString getUsername();
QString getActiveIPAddress();
QString getLastBootTime();
}

#endif // UTILS_H
