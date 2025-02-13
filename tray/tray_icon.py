from pystray import Icon, Menu, MenuItem
from PIL import Image
import sys
from os import path, _exit
from pyperclip import copy
from ctypes import windll, c_void_p, c_wchar_p, c_uint, c_int, POINTER, byref
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
    icon_path = icon_path or resource_path("assets/SysInfo-DALL.E.ico")
    with Image.open(icon_path) as img:
        return img.copy()


def copy_to_clipboard(icon):
    """Копирует информацию в буфер обмена."""
    copy(format_system_info(get_system_info()))
    icon.notify("Інформація скопійована в буфер обміну!", title="Системна інформація")


def confirm_exit():
    """Подтверждает выход из программы с помощью функции dialog_exit в отдельном потоке."""

    def dialog_exit():
        """Подтверждает выход из программы с помощью диалогового окна."""
        task_dialog = windll.comctl32.TaskDialog
        task_dialog.argtypes = [
            c_void_p,
            c_void_p,
            c_wchar_p,
            c_wchar_p,
            c_wchar_p,
            c_uint,
            c_void_p,
            POINTER(c_int),
        ]
        task_dialog.restype = c_int

        result = c_int()

        task_dialog(
            None,  # hWnd (окно-владелец)
            None,  # hInstance (приложение)
            "Підтвердження виходу",  # Заголовок окна
            "Ви впевнені, що хочете вийти?",  # Основное сообщение
            "Закриття зупине моніторинг системної інформації,\n"
            "що може бути корисна для Вас або фахівців.",  # Дополнительное описание (можно оставить пустым)
            2 | 4,  # Флаги (2/4 = Yes/No)
            32514,  # Стандартная иконка Question
            byref(result),  # Результат ответа пользователя (6 = Yes, 7 = No)
        )

        if result.value == 6:  # 6 = Yes
            _exit(0)  # Жесткий выход чтобы прервать работу потоков при блокировках ОС

    # Запускаем допольнительный поток для избежания блокировок основного потока со стороны ОС
    Thread(target=dialog_exit, daemon=True).start()


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
        title=f"{format_system_info(system_info)}",
    )

    # Запускаем поток для мониторинга IP
    Thread(
        target=monitor_ip_change,
        args=(icon, get_system_info, format_system_info),
        daemon=True,
    ).start()

    return icon
