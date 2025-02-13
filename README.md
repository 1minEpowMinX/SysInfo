# SystemInfo  

**SystemInfo** — это утилита для Windows, которая предоставляет пользователю ключевую информацию о системе в удобном формате. Программа работает в системном трее, позволяя получать данные одним кликом, а также копировать их в буфер обмена.

---

## **⚙ Функционал**

### **Основные возможности**:

- **Отображение системной информации**:  
  - Имя компьютера.
  - Имя пользователя.
  - Локальный IP-адрес.
  - Время последнего включения компьютера.

- **Интерактивная работа с системным треем**:
  - Всплывающая подсказка с информацией о системе.
  - Контекстное меню:
    - Копирование данных в буфер обмена.
    - Выход из программы с последующим подтверждением.
  
- **Уведомление при первом запуске**:
  - Описание функционала программы.
  - Инструкция по закреплению иконки в области уведомлений.

### **Технические особенности**:
- Поддержка автономного запуска (EXE-файл не требует установки Python).
- Оптимизированная загрузка ресурсов (путь к файлам адаптирован для использования Cx_Freeze и PyInstaller).
- Программа поставляется в виде ZIP-архива, содержащего исполняемый файл и необходимые зависимости.
- Обработка ошибок: программа завершает работу, если уже запущена другая копия.

---

  ## **📸 Скриншоты**

**Иконка в трее**  
Иконка отображается рядом с часами и позволяет легко получить доступ к функции программы:
<p align="center">
  <img src="https://github.com/user-attachments/assets/28220dc4-007a-42a5-98bf-fd2fb8effad6" alt="Значок в трее">
</p>  

**Всплывающая подсказка**  
При наведении на иконку всплывает подсказка с информационной сводкой программы:  
<p align="center">
  <img src="https://github.com/user-attachments/assets/45308020-86d2-4f6d-8839-86f7ec8ac59c" alt="Информация при наведении на иконку">
</p>

**Контекстное меню**  
Щелчок правой кнопкой по иконке открывает меню с функциями копирования данных и завершения программы:  
<p align="center">
  <img src="https://github.com/user-attachments/assets/88ccc9cd-aa2a-4d27-b365-74c1dce5febc" alt="Контекстное меню">
</p>

**Уведомление при первом запуске**
Программа покажет подробное сообщение о закреплении значка в трее и использовании основных функций:
<p align="center">
  <img src="https://github.com/user-attachments/assets/292625e6-05ac-463a-ba23-f04caf8d7f01" alt="Контекстное меню">
</p>

---

## **🔧 Установка и использование**

**Системные требования**:
- Windows 7/10/11.
- Нет необходимости в установленном Python и завимостях.

**Установка**:
1. Скачайте `.zip`-архив с программой из раздела [Releases](https://github.com/1minEpowMinX/SysInfo/releases).
2. Распакуйте архив в удобное для вас место.
3. Запустите файл `SysInfo.exe`.

**Использование**:
1. При запуске программа добавляется в область уведомлений (рядом с часами).
2. Наведите курсор на значок, чтобы увидеть системную информацию.
3. Щёлкните правой кнопкой по значку для доступа к меню.

---

## **📖 Подробности реализации**

**Сбор и обновление информации**:
Используются стандартные библиотеки Python (`platform`, `socket`, `psutil`) для получения системных данных.

**Работа с треем**:
- Для управления иконкой и меню используется библиотека `pystray` вместе с `Pillow`.
- Всплывающие подсказки реализованы встроенными методами `pystray.Icon`.

**Упаковка**:
- Программа упакована с помощью Cx_Freeze 7.2.
- Файлы и зависимости упакованы в ZIP-архив для удобства распространения.
- Оптимизированы пути к ресурсам для работы в собранной версии программы.

---

## **🚧 Известные проблемы**

1. Программа не предупреждает о недоступности файлов ресурса (например, иконки), если они отсутствуют.
2. Для поддержки сложных сетевых конфигураций потребуется доработка.

---

## **📜 Лицензия**

Этот проект распространяется под лицензией [MIT](https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE).
