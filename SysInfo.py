import socket
import os
import sys
import platform
import psutil
from pystray import Icon, Menu, MenuItem
from PIL import Image, ImageDraw
from datetime import datetime


def get_system_info():
    """Собирает статическую информацию о системе."""
    pc_name = platform.node()  # Имя компьютера
    user_name = psutil.users()[0].name  # Имя пользователя
    ip_address = socket.gethostbyname(socket.gethostname())  # IP-адрес
    boot_time = datetime.fromtimestamp(psutil.boot_time()).strftime(
        "%d-%m-%Y %H:%M")  # Время включения
    return f"Ім’я ПК: {pc_name}\nЛогін: {user_name}\nIP-адреса: {ip_address}\nЧас вмикання ПК: {boot_time}"


def resource_path(relative_path):
    """Получает путь к ресурсам (иконке) внутри EXE."""
    if hasattr(sys, "_MEIPASS"):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)


def load_tray_icon():
    """Загружает иконку для трея из ресурсов."""
    icon_path = resource_path("bginfo-icon.ico")
    return Image.open(icon_path)


# Получаем данные о системе
system_info = get_system_info()

menu = Menu(
    # Добавляем пункт меню для выхода
    MenuItem("Exit", lambda icon, _: icon.stop())
)

# Создаём значок в трее с заранее подготовленным текстом подсказки
icon = Icon("System Info", load_tray_icon(), menu=menu, title=system_info)

icon.run()
