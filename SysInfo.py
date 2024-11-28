from socket import gethostname, gethostbyname
from os import path
import sys
from platform import node
from psutil import users, boot_time
from pystray import Icon, Menu, MenuItem
from PIL import Image
from datetime import datetime
from pyperclip import copy
from ctypes import windll


def get_system_info():
    """Собирает статическую информацию о системе."""
    pc_name = node()  # Имя компьютера
    user_name = users()[0].name  # Имя пользователя
    ip_address = gethostbyname(gethostname())  # IP-адрес
    sys_boot_time = datetime.fromtimestamp(boot_time()).strftime(
        "%d.%m.%Y %H:%M")  # Время включения

    return f"Ім’я ПК: {pc_name}\nЛогін: {user_name}\nIP-адреса: {ip_address}\nЧас вмикання ПК: {sys_boot_time}"


def resource_path(relative_path):
    """Оптимизированный способ получения пути к ресурсам."""
    try:
        return path.join(sys._MEIPASS, relative_path) if hasattr(sys, "_MEIPASS") else path.join(path.abspath("."), relative_path)
    except Exception:
        return path.join(path.abspath("."), relative_path)


def load_tray_icon():
    """Загружает иконку для трея из ресурсов."""
    icon_path = resource_path("bginfo-icon.ico")
    return Image.open(icon_path)


def copy_to_clipboard(system_info):
    """Копирует информацию о системе в буфер обмена."""
    copy(system_info)


def main():
    """Основная функция программы."""
    # Создаем мьютекс
    mutex_name = "MyUniqueMutexName"
    handle = windll.kernel32.CreateMutexW(None, False, mutex_name)
    last_error = windll.kernel32.GetLastError()

    # Если мьютекс уже существует, программа завершает работу
    if last_error == 183:  # ERROR_ALREADY_EXISTS
        print("Программа уже запущена!")
        sys.exit()

    try:
        # Получаем данные о системе
        system_info = get_system_info()

        menu = Menu(
            MenuItem("Скопіювати до буферу обміну", lambda icon, _: (copy_to_clipboard(
                system_info), icon.notify("Інформація скопійована в буфер обміну!"))),
            MenuItem("Вихід", lambda icon, _: icon.stop())
        )

        icon = Icon("System Info", load_tray_icon(),
                    menu=menu, title=system_info)

        icon.run()
    finally:
        # Освобождаем мьютекс
        windll.kernel32.ReleaseMutex(handle)


if __name__ == "__main__":
    main()
