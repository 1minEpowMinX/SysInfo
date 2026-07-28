#include "system_info.h"

#include <QDateTime>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QString>

#ifdef Q_OS_WIN
#include <QSettings>
#elif defined(Q_OS_LINUX)
#include <QFile>
#endif

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#elif defined(Q_OS_MAC)
#include <sys/sysctl.h>
#endif

namespace sysinfo
{

	QString hostname()
	{
		return QHostInfo::localHostName();
	}

	QString username()
	{
#ifdef Q_OS_WIN
		return qEnvironmentVariable("USERNAME");
#else
		return qEnvironmentVariable("USER");
#endif
	}

	namespace
	{

		bool looksLikeVirtualBridge(const QString &nameLower)
		{
			static const char *const kBridgeKeywords[] = {
				"docker", "vethernet", "vmware", "vmnet",
				"virtualbox", "vboxnet", "hyper-v", "bluetooth"};
			for (const char *kw : kBridgeKeywords)
			{
                if (nameLower.contains(QLatin1String(kw))) {
					return true;
                }
			}
			return false;
		}

		bool looksLikeVpn(const QNetworkInterface &iface)
		{
            if (iface.type() == QNetworkInterface::Virtual) {
				return true;
            }

			const QString nameLower = iface.humanReadableName().toLower();
			static const char *const kVpnKeywords[] = {
				"vpn", "wireguard", "tailscale", "openvpn",
				"anyconnect", "cisco", "zerotier", "tun", "tap"};
			for (const char *kw : kVpnKeywords)
			{
                if (nameLower.contains(QLatin1String(kw))) {
					return true;
                }
			}
			return false;
		}

	} // namespace

	QString activeIpAddress()
	{
		QString vpnIp, lanIp;

		const auto &interfaces = QNetworkInterface::allInterfaces();
		for (const QNetworkInterface &iface : interfaces)
		{
			const auto flags = iface.flags();
			// Skip interfaces that are down, loopback or not running
			if (!flags.testFlag(QNetworkInterface::IsUp) ||
				!flags.testFlag(QNetworkInterface::IsRunning) ||
                flags.testFlag(QNetworkInterface::IsLoopBack)) {
				continue;
            }

			const QString nameLower = iface.humanReadableName().toLower();
            if (looksLikeVirtualBridge(nameLower)) {
				continue;
            }

			const bool isVpn = looksLikeVpn(iface);

			const auto &entries = iface.addressEntries();
			for (const QNetworkAddressEntry &entry : entries)
			{
				const QString ip = entry.ip().toString();
                if (ip.contains(QLatin1Char(':'))) {
					continue; // Skip IPv6
                }

				if (isVpn)
				{
					if (vpnIp.isEmpty())
						vpnIp = ip;
				}
				else if (lanIp.isEmpty())
				{
					lanIp = ip;
				}
			}
		}

        if (!vpnIp.isEmpty()) {
			return vpnIp;
        }
        if (!lanIp.isEmpty()) {
			return lanIp;
        }
		return {}; // empty = "no usable IPv4 found"; presenter localises the fallback
	}

	QString osBuild()
	{
#ifdef Q_OS_WIN
		// Neither GetVersionEx nor QSysInfo expose the UBR, so the registry
		// is the only source for the revision half of the build string.
		const QSettings currentVersion(
			QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
			QSettings::NativeFormat);

		const QString build =
			currentVersion.value(QStringLiteral("CurrentBuildNumber")).toString();
		if (build.isEmpty())
		{
			return {};
		}

		const QVariant revision = currentVersion.value(QStringLiteral("UBR"));
		return revision.isValid()
				   ? build + QLatin1Char('.') + QString::number(revision.toUInt())
				   : build;

#elif defined(Q_OS_LINUX)
		// Content looks like "#45-Ubuntu SMP PREEMPT_DYNAMIC Fri Aug 30 ...";
		// only the leading build tag is worth keeping.
		QFile version(QStringLiteral("/proc/sys/kernel/version"));
		if (!version.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			return {};
		}

		const QString tag = QString::fromUtf8(version.readLine())
								.trimmed()
								.section(QLatin1Char(' '), 0, 0);
		return tag.startsWith(QLatin1Char('#')) ? tag.mid(1) : tag;

#elif defined(Q_OS_MAC)
		char build[64] = {};
		size_t length = sizeof(build);
		if (sysctlbyname("kern.osversion", build, &length, nullptr, 0) != 0)
		{
			return {};
		}
		return QString::fromLatin1(build);

#else
		return {};
#endif
	}

	QDateTime bootTime()
	{
#ifdef Q_OS_WIN
		const ULONGLONG uptimeMs = GetTickCount64();
		return QDateTime::currentDateTime().addMSecs(-qint64(uptimeMs));

#elif defined(Q_OS_LINUX)
		struct sysinfo s_info;
		if (::sysinfo(&s_info) == 0) // Linux sys/sysinfo.h disambiguation
		{
			return QDateTime::currentDateTime().addSecs(-s_info.uptime);
		}
		return {};

#elif defined(Q_OS_MAC)
		// macOS does not have sysinfo, so we use sysctl
		struct timeval boottime;
		size_t len = sizeof(boottime); // Buffer size
		int mib[2] = {CTL_KERN, KERN_BOOTTIME};
		if (sysctl(mib, 2, &boottime, &len, nullptr, 0) == 0)
		{
			return QDateTime::fromSecsSinceEpoch(boottime.tv_sec);
		}
		return {};

#else
		return {};
#endif
	}

	QString lastBootTime()
	{
		const QDateTime boot = bootTime();
		return boot.isValid() ? boot.toString("dd.MM.yyyy HH:mm") : QString();
	}

	qint64 bootTimeSecs()
	{
		const QDateTime boot = bootTime();
		// toSecsSinceEpoch() is timezone-independent, so the local-time
		// QDateTime above still yields the correct absolute instant.
		return boot.isValid() ? boot.toSecsSinceEpoch() : 0;
	}

	Info collect()
	{
		Info s;
		s.hostname = sysinfo::hostname();
		s.username = sysinfo::username();
		s.ip = sysinfo::activeIpAddress();
		s.uptime = sysinfo::lastBootTime();
		return s;
	}


} // namespace sysinfo
