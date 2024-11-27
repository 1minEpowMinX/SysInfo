from socket import gethostname, gethostbyname
from os import path
import sys
from platform import node
from psutil import users, boot_time
from pystray import Icon, Menu, MenuItem
from PIL import Image
from datetime import datetime
from tkinter import Tk


def get_system_info():
    """Собирает статическую информацию о системе."""
    pc_name = node()  # Имя компьютера
    user_name = users()[0].name  # Имя пользователя
    ip_address = gethostbyname(gethostname())  # IP-адрес
    sys_boot_time = datetime.fromtimestamp(boot_time()).strftime(
        "%d-%m-%Y %H:%M")  # Время включения

    return f"Ім’я ПК: {pc_name}\nЛогін: {user_name}\nIP-адреса: {ip_address}\nЧас вмикання ПК: {sys_boot_time}"


def resource_path(relative_path):
    """Получает путь к ресурсам (иконке) внутри EXE."""
    if hasattr(sys, "_MEIPASS"):
        return path.join(sys._MEIPASS, relative_path)

    return path.join(path.abspath("."), relative_path)


def load_tray_icon():
    """Загружает иконку для трея из ресурсов."""
    icon_path = resource_path("bginfo-icon.ico")

    return Image.open(icon_path)


def copy_to_clipboard(icon):
    """Копирует информацию о системе в буфер обмена."""
    tk = Tk()
    tk.withdraw()  # скрываем окно
    tk.clipboard_clear()  # очищаем буфер
    tk.clipboard_append(system_info)  # копируем в буфер
    tk.update()  # обновляем
    tk.destroy()  # удаляем

    # Показываем уведомление
    icon.notify("Інформація скопійована в буфер обміну!")


# Получаем данные о системе
system_info = get_system_info()

menu = Menu(
    MenuItem("Copy to Clipboard", copy_to_clipboard),  # Пункт для копирования
    MenuItem("Exit", lambda icon, _: icon.stop())  # Пункт меню для выхода
)

# Создаём значок в трее с заранее подготовленным текстом подсказки
icon = Icon("System Info", load_tray_icon(), menu=menu, title=system_info)

icon.run()
