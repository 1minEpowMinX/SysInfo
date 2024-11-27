from socket import gethostname, gethostbyname
from os import path, remove
import sys
from platform import node
from psutil import users, boot_time, Process, process_iter
from pystray import Icon, Menu, MenuItem
from PIL import Image
from datetime import datetime
from pyperclip import copy

LOCK_FILE = "sysinfo.lock"


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
        # Используем только один способ получения пути, если это возможно
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

    # Проверяем, существует ли уже файл блокировки
if path.exists(LOCK_FILE):
    print("Программа уже запущена!")
    sys.exit()

# Создаем лок-файл
try:
    with open(LOCK_FILE, "w") as lock_file:
        lock_file.write("locked")

    # Получаем данные о системе
    system_info = get_system_info()

    menu = Menu(
        # Пункт для копирования
        MenuItem("Copy to Clipboard", lambda icon, _: (copy_to_clipboard(
            system_info), icon.notify("Інформація скопійована в буфер обміну!"))),
        MenuItem("Exit", lambda icon, _: icon.stop())  # Пункт меню для выхода
    )

    # Создаём значок в трее с заранее подготовленным текстом подсказки
    icon = Icon("System Info", load_tray_icon(),
                menu=menu, title=system_info)

    icon.run()
finally:
    if path.exists(LOCK_FILE):
        remove(LOCK_FILE)  # Удаляем лок-файл


if __name__ == "__main__":
    main()  # Запускаем основную функцию, если программа запущена напрямую
