from json import load, dump
from socket import gethostname, gethostbyname
from os import path
import sys
from platform import node
from psutil import users, boot_time
from pystray import Icon, Menu, MenuItem
from PIL import Image
from datetime import datetime
from ctypes import windll
from collections import namedtuple

# Определяем компактную структуру для системной информации
SystemInfo = namedtuple(
    "SystemInfo", ["pc_name", "user_name", "ip_address", "boot_time"])


def get_system_info():
    """Собирает статическую информацию о системе."""
    return SystemInfo(
        pc_name=node(),
        user_name=users()[0].name,
        ip_address=gethostbyname(gethostname()),
        boot_time=datetime.fromtimestamp(
            boot_time()).strftime("%d.%m.%Y %H:%M")
    )


def format_system_info(info):
    """Форматирует информацию о системе для отображения."""
    return (
        f"Ім’я ПК: {info.pc_name}\n"
        f"Логін: {info.user_name}\n"
        f"IP-адреса: {info.ip_address}\n"
        f"Час вмикання ПК: {info.boot_time}"
    )


def resource_path(relative_path):
    """Оптимизированный способ получения пути к ресурсам."""
    try:
        return path.join(sys._MEIPASS, relative_path) if hasattr(sys, "_MEIPASS") else path.join(path.abspath("."), relative_path)
    except Exception:
        return path.join(path.abspath("."), relative_path)


def load_tray_icon(icon_path=None):
    """Загружает иконку для трея из ресурсов."""
    icon_path = icon_path or resource_path("SysInfo-DALL.E.ico")
    with Image.open(icon_path) as img:
        return img.copy()


def copy_to_clipboard(system_info):
    """Копирует информацию о системе в буфер обмена."""
    from pyperclip import copy
    copy(system_info)


def is_first_run(config_path):
    """Проверяет, является ли это первый запуск программы."""
    if not path.exists(config_path):
        return True
    with open(config_path, 'r') as f:
        data = load(f)
        return data.get('first_run', True)


def set_first_run_complete(config_path):
    """Отмечает, что первый запуск завершён."""
    with open(config_path, 'w') as f:
        dump({'first_run': False}, f)


def show_first_run_message():
    """Показывает улучшенное сообщение при первом запуске."""
    from tkinter import Tk, Toplevel, Label, Button, BOTH, LEFT
    from threading import Thread

    def display_window():
        """Создает и показывает окно уведомления."""
        root = Tk()
        root.withdraw()  # Скрываем основное окно Tkinter

        # Создаем окно уведомления
        first_run_window = Toplevel(root)
        first_run_window.title("Закріплення програми у треї")
        first_run_window.geometry("600x600")
        first_run_window.resizable(False, False)

        # Заголовок
        Label(
            first_run_window,
            text="Вітаємо!",
            font=("Arial", 16, "bold"),
            pady=10
        ).pack()

        # Текст уведомления
        text_message = (
            "Ви вперше запустили додаток «Системна інформація».\n\n"
            "Ця програма призначена для швидкого, зручного перегляду інформації про ваш комп'ютер "
            "та її копіювання в буфер обміну. Вона працює в області повідомлень (поруч із годинником) "
            "і завжди готова до використання.\n\n"
            "Щоб закріпити її в області повідомлень (поруч із годинником):\n"
            "1. Подивіться в нижній правий кут екрана, де розташований годинник.\n"
            "2. Якщо ви не бачите іконку програми, натисніть на стрілочку поруч із годинником (^).\n"
            "3. Знайдіть значок програми в списку, що з'явився (він має вигляд блакитного монітора на помаранчевому тлі).\n"
            "4. Зажміть ліву кнопку миші на цьому значку і перетягніть його на панель поруч із годинником.\n\n"
            "Тепер програма буде завжди доступна в області повідомлень. Вам потрібно лише навести на неї курсор!\n\n"
            "Для копіювання інформації про систему в буфер обміну натисніть на значок програми правою кнопкою миші та оберіть відповідний пункт."
        )
        Label(
            first_run_window,
            text=text_message,
            font=("Arial", 12),
            wraplength=480,
            justify=LEFT,
            padx=10,
            pady=5
        ).pack(fill=BOTH, expand=True)

        # Кнопка закрытия
        def close_window():
            first_run_window.destroy()
            root.destroy()  # Уничтожаем корневой объект

        Button(
            first_run_window,
            text="Закрити",
            command=close_window,
            font=("Arial", 12),
            pady=5
        ).pack(pady=10)

        # Центрирование окна
        first_run_window.update_idletasks()
        width = first_run_window.winfo_width()
        height = first_run_window.winfo_height()
        x = (first_run_window.winfo_screenwidth() // 2) - (width // 2)
        y = (first_run_window.winfo_screenheight() // 2) - (height // 2)
        first_run_window.geometry(f"{width}x{height}+{x}+{y}")

        root.mainloop()

    # Запуск окна уведомления в отдельном потоке
    Thread(target=display_window, daemon=True).start()


def confirm_exit(icon):
    """Подтверждает выход пользователя из программы через MessageBox."""
    MB_YESNO = 0x04  # Флаги для кнопок "Yes" и "No"
    MB_ICONQUESTION = 0x20  # Иконка вопроса
    IDYES = 6  # Возвращаемое значение для кнопки "Yes"

    result = windll.user32.MessageBoxW(
        None,
        "Ви впевнені, що хочете вийти?",
        "Підтвердження виходу",
        MB_YESNO | MB_ICONQUESTION
    )

    if result == IDYES:
        icon.stop()


def main():
    """Основная функция программы."""
    # Создаем мьютекс
    mutex_name = "MyUniqueMutexName"
    handle = windll.kernel32.CreateMutexW(None, False, mutex_name)
    last_error = windll.kernel32.GetLastError()

    # Если мьютекс уже существует, программа завершает работу
    if last_error == 183:  # ERROR_ALREADY_EXISTS
        print("Програма вже запущена!")
        sys.exit()

    # Конфигурация
    if is_first_run("sysinfo_config.json"):
        show_first_run_message()
        set_first_run_complete("sysinfo_config.json")

    try:
        menu = Menu(
            MenuItem("Скопіювати до буферу обміну", lambda icon, _: (copy_to_clipboard(
                get_system_info()), icon.notify("Інформація скопійована в буфер обміну!", title="Системна інформація"))),
            MenuItem("Вихід", lambda icon, _: confirm_exit(icon))
        )

        icon = Icon("System Info", load_tray_icon(),
                    menu=menu, title=format_system_info(get_system_info()))

        icon.run()
    finally:
        # Освобождаем мьютекс
        windll.kernel32.ReleaseMutex(handle)


if __name__ == "__main__":
    main()
