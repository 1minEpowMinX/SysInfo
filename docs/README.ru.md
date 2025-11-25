<h1 align="center">Welcome to SysInfo 👋</h1>

<p align="center">
  🌐 <b>Доступные языки:</b><br>
  <a href="../README.md"><img src="https://img.shields.io/badge/English-blue?style=flat-square"></a>
  <a href="README.ru.md"><img src="https://img.shields.io/badge/Русский-green?style=flat-square"></a>
  <a href="README.uk.md"><img src="https://img.shields.io/badge/Українська-yellow?style=flat-square"></a>
</p>

> SysInfo – это кроссплатформенная утилита, которая предоставляет пользователям ключевую системную информацию в удобном формате. Программа работает в системном трее, позволяя одним щелчком мыши получать данные и копировать их в буфер обмена. Ее также можно интегрировать с Jira SM с помощью расширения для браузера.

## ✨ Возможности

* Собирает и отображает системную информацию (название устройства, пользователь, IP-адрес, время включения).
* Работает незаметно в фоновом режиме через системный трей.
* Обеспечивает быстрый доступ через контекстное меню на значке в трее.
* Позволяет копировать текущую системную информацию в буфер обмена.
* Предлагает минималистичный, легкий и отзывчивый дизайн для повседневной диагностики.
* Добавляет интеграцию с Jira SM через расширение браузера.
* Автоматически прикрепляет системную информацию к тикетам Jira SM (**доступно только при установленном расширении**).
* Наследует локализацию из настроек языка браузера и системы (английский, русский, украинский).

## 🛠️ Создано с помощью

![Qt](https://img.shields.io/badge/Qt%206.10.0-009639?logo=Qt&logoColor=fff)
![C++17](https://img.shields.io/badge/C++%2017-%2300599C.svg?logo=c%2B%2B&logoColor=white)
![Git](https://img.shields.io/badge/Git-F05032?logo=git&logoColor=fff)
![Chrome extension](https://img.shields.io/badge/Google%20Chrome%20Extension-4285F4?logo=GoogleChrome&logoColor=white)
![Edge extension](https://custom-icon-badges.demolab.com/badge/Microsoft%20Edge%20Extension-2771D8?logo=edge-white&logoColor=white)
![Firefox extension](https://img.shields.io/badge/Firefox%20Extension-FF7139?logo=Firefox&logoColor=white)

## ⚙️ Установка

### 1. Скачать последнюю версию

👉 [SysInfo Releases on GitHub](https://github.com/1minEpowMinX/SysInfo/releases/latest)

### 2. Извлечь архив

```sh
# Linux
sudo apt install p7zip-full
sudo 7z x SysInfo-2.0.0-linux-x64.7z -o/usr/bin/

# MacOS:
brew install p7zip
sudo 7z x SysInfo-2.0.0-macos-x64.7z -o/usr/local/bin/

# Windows:
"C:\Program Files\7-Zip\7z.exe" x SysInfo-2.0.0-windows-x64.7z -o"C:\Program Files\"
```

### 3. Добавить в автозагрузку

```sh
# Linux
curl -L -o /etc/xdg/autostart/sysinfo.desktop https://raw.githubusercontent.com/1minEpowMinX/SysInfo/refs/heads/dev/SysInfo/resources/sysinfo.desktop

# macOS:
curl -L -o /Library/LaunchAgents/com.pivdenny.sysinfo.plist https://raw.githubusercontent.com/1minEpowMinX/SysInfo/refs/heads/main/SysInfo/extras/com.pivdenny.sysinfo.plist
launchctl load /Library/LaunchAgents/com.pivdenny.sysinfo.plist

# Windows:
$source = "C:\Path\To\SysInfo.exe"
$shortcut = "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup\SysInfo.lnk"

$WshShell = New-Object -ComObject WScript.Shell
$shortcutObj = $WshShell.CreateShortcut($shortcut)
$shortcutObj.TargetPath = $source
$shortcutObj.WorkingDirectory = Split-Path $source
$shortcutObj.Save()
```

## 🚀 Использование

После установки SysInfo работает в фоновом режиме через системный трей.

### 🖥️ Быстрый старт

* Запустите SysInfo вручную или настройте его автоматический запуск при входе в систему (см. [Добавить в автозагрузку](#3-add-to-startup)).
* Найдите значок SysInfo в системном трее (области уведомлений).
* Щелкните правой кнопкой мыши по значку, чтобы открыть меню действий.

### 🧩 Интеграция

Если расширение браузера установлено, SysInfo автоматически прикрепляет сведения о системе к тикетам Jira SM.

### ⚠️ Примечания

* Примеры путей в этом руководстве предполагают использование путей установки по умолчанию. Всегда проверяйте их и при необходимости заменяйте (**включая исходные файлы**).
* Для интеграции с Jira SM требуется дополнительное расширение для браузера.
Возможно, вам потребуется адаптировать его настройки или исходный код (например, URL-адреса, сопоставление полей тикетов) в соответствии с экземпляром Jira вашей компании.

## 📘 Автор

### 👤 Kirill Bitskyi

* Github: [@1minEpowMinX](https://github.com/1minEpowMinX)
* LinkedIn: [@Kirill Bitskyi](https://www.linkedin.com/in/kirill-bitskyi-025672284/)

## 🤝 Вклад

Приветствуются вклады, вопросы и запросы на добавление новых функций!  
Не стесняйтесь заглянуть на [страницу вопросов](https://github.com/1minEpowMinX/InvBinderBot/issues).

## 📝 Лицензия

Авторские права © 2025 [1minEpowMinX](https://github.com/1minEpowMinX).  
Этот проект лицензирован по лицензии [LGPL V3.0](https://github.com/1minEpowMinX/InvBinderBot/blob/main/LICENSE).
