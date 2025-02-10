from platform import node
from psutil import users, boot_time
from socket import gethostname, gethostbyname
from datetime import datetime
from collections import namedtuple

SystemInfo = namedtuple(
    "SystemInfo", ["pc_name", "user_name", "ip_address", "boot_time"]
)


def get_system_info():
    """Собирает статическую информацию о системе."""
    return SystemInfo(
        pc_name=node(),
        user_name=users()[0].name,
        ip_address=gethostbyname(gethostname()),
        boot_time=datetime.fromtimestamp(boot_time()).strftime("%d.%m.%Y %H:%M"),
    )


def format_system_info(info):
    """Форматирует информацию о системе."""
    return (
        f"Ім’я ПК: {info.pc_name}\n"
        f"Логін: {info.user_name}\n"
        f"IP-адреса: {info.ip_address}\n"
        f"Час вмикання ПК: {info.boot_time}"
    )
