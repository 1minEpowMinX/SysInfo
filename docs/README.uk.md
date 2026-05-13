<h1 align="center">Welcome to SysInfo 👋</h1>

<p align="center">
  🌐 <b>Доступні мови:</b><br>
  <a href="../README.md"><img src="https://img.shields.io/badge/English-blue?style=flat-square"></a>
  <a href="README.ru.md"><img src="https://img.shields.io/badge/Русский-green?style=flat-square"></a>
  <a href="README.uk.md"><img src="https://img.shields.io/badge/Українська-yellow?style=flat-square"></a>
</p>

> SysInfo – це кросплатформна утиліта, яка надає користувачам ключову інформацію про систему в зручному форматі. Програма працює в системному треї, що дозволяє отримувати дані одним клацанням миші та копіювати їх у буфер обміну. Вона також може бути інтегрована з Jira SM за допомогою розширення для браузера.

## ✨ Можливості

* Збирає та відображає системну інформацію (назва пристрою, користувач, IP-адреса, час вмикання).
* Працює тихо у фоновому режимі через системний трей.
* Забезпечує швидкий доступ через контекстне меню на значку у треї.
* Дозволяє копіювати поточну системну інформацію до буфера обміну.
* Пропонує мінімалістичний, легкий та чуйний дизайн для щоденної діагностики.
* Додає інтеграцію з Jira SM через розширення браузера.
* Автоматично додає інформацію про систему до тікетів Jira SM (**доступно тільки з встановленим розширенням**).
* Спадкує локалізацію з налаштувань мови браузера та системи (англійська, російська, українська).

## 🛠️ Створено за допомогою

![Qt](https://img.shields.io/badge/Qt%206.11.1-009639?logo=Qt&logoColor=fff)
![C++17](https://img.shields.io/badge/C++%2017-%2300599C.svg?logo=c%2B%2B&logoColor=white)
![Git](https://img.shields.io/badge/Git-F05032?logo=git&logoColor=fff)
![Chrome extension](https://img.shields.io/badge/Google%20Chrome%20Extension-4285F4?logo=GoogleChrome&logoColor=white)
![Edge extension](https://custom-icon-badges.demolab.com/badge/Microsoft%20Edge%20Extension-2771D8?logo=edge-white&logoColor=white)
![Firefox extension](https://img.shields.io/badge/Firefox%20Extension-FF7139?logo=Firefox&logoColor=white)

## ⚙️ Встановлення

### 1. Завантажити останню версію  

👉 [SysInfo Releases on GitHub](https://github.com/1minEpowMinX/SysInfo/releases/latest)

### 2. Вилучити архів  

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

### 3. Додати до автозавантаження

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

### 4. Дозволити користувацьке розширення браузера *(опційно)*

SysInfo приймає запити **лише** від ідентифікаторів розширень браузерів, що **знаходяться у білому списку**. Офіційні ідентифікатори Chrome/Edge та Firefox вже включені, тому це потрібно лише для користувацьких або корпоративних збірок з іншими ідентифікаторами.

Білий список зберігається під ключем `Integration/AllowedExtensionIds`. Якщо цей ключ існує і не є порожнім, він **замінює** вбудовані значення за замовчуванням.

```sh
# Linux: ~/.config/Pivdenny/SysInfo.conf
mkdir -p ~/.config/Pivdenny
cat >> ~/.config/Pivdenny/SysInfo.conf <<'EOF'

[Integration]
AllowedExtensionIds=your-custom-id1, your-custom-id2
EOF

# macOS: ~/Library/Preferences/com.pivdenny.SysInfo.plist
defaults write com.pivdenny.SysInfo Integration.AllowedExtensionIds -array \
    your-custom-id1 \
    your-custom-id2

# Windows: HKCU\Software\Pivdenny\SysInfo\Integration
reg add "HKCU\Software\Pivdenny\SysInfo\Integration" ^
    /v AllowedExtensionIds /t REG_MULTI_SZ ^
    /d "your-custom-id1\0your-custom-id2" /f
```

> ℹ️ Щоб дізнатися ID власного розширення, відкрийте `chrome://extensions` (або `about:addons` у Firefox) з увімкненим режимом розробника.

## 🚀 Використання

Після встановлення SysInfo працює у фоновому режимі через системний трей.

### 🖥️ Швидкий старт

* Запустіть SysInfo вручну або дозвольте йому запускатися автоматично при вході в систему (див. [Додати до автозавантаження](#3-add-to-startup)).
* Знайдіть значок SysInfo в системному треї (області сповіщень).
* Клацніть правою кнопкою миші на значок, щоб відкрити меню дій.

### 🧩 Інтеграція

Якщо розширення браузера встановлено, SysInfo автоматично додає інформацію про систему до тикетів Jira SM.

### ⚠️ Примітки

* Приклади шляхів у цьому посібнику базуються на стандартних шляхах інсталяції. Завжди перевіряйте їх і за необхідності замінюйте (**включно з вихідними файлами**).
* Для інтеграції Jira Service Management потрібне додаткове розширення для браузера.
Можливо, вам доведеться адаптувати його налаштування або вихідний код (наприклад, URL-адреси, зіставлення полів квитків) відповідно до інстанції Jira вашої компанії.

## 📘 Автор

### 👤 Kirill Bitskyi

* Github: [@1minEpowMinX](https://github.com/1minEpowMinX)
* LinkedIn: [@Kirill Bitskyi](https://www.linkedin.com/in/kirill-bitskyi-025672284/)

## 🤝 Внесок

Вітаються внески, питання та пропозиції щодо нових функцій!  
Не соромтеся заглянути на [сторінку питань](https://github.com/1minEpowMinX/InvBinderBot/issues).

## 📝 Ліцензія

Авторські права © 2026 [1minEpowMinX](https://github.com/1minEpowMinX).  
Цей проект ліцензований за ліцензією [GPL-3.0](https://github.com/1minEpowMinX/InvBinderBot/blob/main/LICENSE).
