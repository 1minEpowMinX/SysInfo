# SystemInfo  

**SystemInfo** — это утилита для Windows, которая отображает ключевую информацию о компьютере (имя ПК, имя пользователя, IP-адрес, время включения) в виде всплывающей подсказки при наведении на значок в системном трее.

Программа разработана на Python и упакована в EXE-файл с помощью PyInstaller.

---

## **⚙ Функционал**

- Отображение системной информации:  
  - Имя компьютера.
  - Имя пользователя.
  - Локальный IP-адрес.
  - Локальный IP-адрес.

- Всплывающая подсказка при наведении на значок в трее.
- Возможность выхода из программы через контекстное меню.
- Возможность скопировать информацию из подсказки через контекстное меню в буфер обмена.

---

  ## **📸 Скриншоты**

Иконка приложения в трее:
<p align="center">
  <img src="https://github.com/user-attachments/assets/59a9349d-88e2-4da5-bf29-a1e86bcc47e9" alt="Значок в трее">
</p>  

Подсказка с информацией при наведении на иконку:  
<p align="center">
  <img src="https://github.com/user-attachments/assets/d2640219-62dd-4baf-8d24-205474950ee0" alt="Информация при наведении на иконку">
</p>

Контекстное меню при клике на иконку:  
<p align="center">
  <img src="https://github.com/user-attachments/assets/458d85de-78ad-405f-90a2-38eda285c87c" alt="Контекстное меню">
</p>

---

## **🔧 Установка и использование**

**Системные требования**
- Windows 7/10/11.
- Нет необходимости в установленном Python (EXE-файл автономный за счет включенных зависимостей).

**Запуск программы**
1. Скачайте `.zip`-архив из раздела [Releases](https://github.com/1minEpowMinX/SysInfo/releases).
2. Распакуйте архив в удобное для вас место.
3. Запустите файл `SystemInfo.exe`.
4. Программа добавится в виде значка в системный трей.
5. Наведите курсор мыши на значок, чтобы увидеть информацию о системе.

---

## **📖 Подробности реализации**

- **Сбор системной информации:** 
  - Используются стандартные библиотеки Python (`platform`, `socket`, `psutil`) для получения имени компьютера, IP-адреса, и времени включения.
- **Работа с системным треем:**
  - Для управления иконкой трея используется библиотека `pystray`.
  - Подсказка и меню создаются автоматически.
- **Упаковка:**
  - PyInstaller используется для создания автономного EXE-файла, который включает все необходимые ресурсы.

---

## **🚧 Известные проблемы**

1. При запуске EXE-файла без привязки иконки (`--icon`), иконка в трее может не отображаться.
2. Программа отображает локальный IP-адрес. Для поддержки сложных сетевых конфигураций потребуется доработка.

---

## **📜 Лицензия**

Этот проект распространяется под лицензией [MIT](https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE).
