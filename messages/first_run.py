from tkinter import Tk, Toplevel, Label, Button, BOTH, LEFT, RIGHT, PhotoImage
from threading import Thread


def show_first_run_message():
    """Показывает улучшенное сообщение при первом запуске."""

    def display_window():
        """Создает и показывает окно уведомления."""
        root = Tk()
        root.withdraw()  # Скрываем основное окно Tkinter

        # Создаем окно уведомления
        first_run_window = Toplevel(root)
        first_run_window.title("Закріплення програми у треї")
        first_run_window.geometry("900x700")
        first_run_window.resizable(False, False)

        # Заголовок
        Label(
            first_run_window, text="Вітаємо!", font=("Arial", 16, "bold"), pady=10
        ).pack()

        # Контейнер для текста и png
        container = Label(first_run_window)
        container.pack(fill=BOTH, expand=True)

        # Текст уведомления
        text_message = (
            "Ви вперше запустили додаток «Системна інформація».\n\n"
            "Ця програма призначена для зручного перегляду інформації про ваш комп'ютер "
            "та її копіювання в буфер обміну. Вона працює в області повідомлень (поруч із годинником) "
            "і завжди готова до використання.\n\n"
            "Щоб закріпити її в області повідомлень (поруч із годинником):\n"
            "1. Подивіться в нижній правий кут екрана, де розташований годинник.\n"
            "2. Якщо ви не бачите іконку програми, натисніть на стрілочку ^ поруч із годинником (позначено числом «1» на зображенні).\n"
            "3. Знайдіть значок програми в списку, що з'явився (позначено числом «2» на зображенні — іконка монітора на помаранчевому тлі).\n"
            "4. Зажміть ліву кнопку миші на цьому значку і перетягніть його на панель поруч із годинником. (позначено числом «3» на зображенні)\n\n"
            "Тепер програма буде завжди доступна в області повідомлень. Вам потрібно лише навести на неї курсор!\n\n"
            "Для копіювання інформації про систему в буфер обміну натисніть на значок програми правою кнопкою миші та оберіть відповідний пункт."
        )

        Label(
            container,
            text=text_message,
            font=("Arial", 12),
            wraplength=400,
            justify=LEFT,
            padx=10,
            pady=5,
        ).pack(side=LEFT, fill=BOTH, expand=True)

        # Загрузка png
        try:
            # Ауть к png-файлу
            png = PhotoImage(file="assets/instructions.png")
            Label(container, image=png).pack(side=RIGHT, padx=10)
        except Exception as e:
            print(f"Не удалось загрузить PNG: {e}")

        # Кнопка закрытия
        def close_window():
            first_run_window.destroy()
            root.destroy()  # Уничтожаем корневой объект

        Button(
            first_run_window,
            text="Закрити",
            command=close_window,
            font=("Arial", 12),
            pady=5,
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
