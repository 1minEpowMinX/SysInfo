from pystray import Icon, Menu, MenuItem
from PIL import Image
import sys
from os import path
from pyperclip import copy
from ctypes import windll
from threading import Thread
from monitoring.system_info import get_system_info, format_system_info
from monitoring.ip_monitor import monitor_ip_change


def resource_path(relative_path):
    """Получает путь к ресурсу."""
    return (
        path.join(sys._MEIPASS, relative_path)
        if hasattr(sys, "_MEIPASS")
        else path.abspath(relative_path)
    )


def load_tray_icon(icon_path=None):
    """Загружает иконку для трея."""
    icon_path = icon_path or resource_path("SysInfo-DALL.E.ico")
    with Image.open(icon_path) as img:
        return img.copy()


def copy_to_clipboard(icon):
    """Копирует информацию в буфер обмена."""
    copy(format_system_info(get_system_info()))
    icon.notify("Інформація скопійована в буфер обміну!", title="Системна інформація")


def confirm_exit(icon):
    """Подтверждает выход из программы."""
    MB_YESNO = 0x4
    MB_ICONQUESTION = 0x20
    IDYES = 6
    MB_SYSTEMMODAL = 0x1000
    result = windll.user32.MessageBoxW(
        None,
        "Ви впевнені, що хочете вийти?",
        "Підтвердження виходу",
        MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL,
    )

    if result == IDYES:
        icon.stop()


def create_tray_icon():
    """Создаёт и запускает трей-иконку."""
    system_info = get_system_info()

    menu = Menu(
        MenuItem("Скопіювати до буферу", copy_to_clipboard),
        MenuItem("Вихід", confirm_exit),
    )

    icon = Icon(
        "System Info",
        load_tray_icon(),
        menu=menu,
        title=f"IP: {format_system_info(system_info)}",
    )

    # Запускаем поток для мониторинга IP
    Thread(
        target=monitor_ip_change,
        args=(icon, get_system_info, format_system_info),
        daemon=True,
    ).start()

    return icon
