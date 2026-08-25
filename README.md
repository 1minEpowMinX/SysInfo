<h1 align="center">Welcome to SysInfo 👋</h1>

<p align="center">
  🌐 <b>Available languages:</b><br>
  <a href="README.md"><img src="https://img.shields.io/badge/English-blue?style=flat-square"></a>
  <a href="docs/README.ru.md"><img src="https://img.shields.io/badge/Русский-green?style=flat-square"></a>
  <a href="docs/README.uk.md"><img src="https://img.shields.io/badge/Українська-yellow?style=flat-square"></a>
</p>

> SysInfo is a cross-platform utility that provides users with key system information in a convenient format. The program runs in the system tray, allowing you to retrieve data with a single click and copy it to the clipboard. It can also be integrated with Jira SM using a browser extension.

## ✨ Features

* Collects and displays system information (device name, user, IP address, last boot time).
* Operates silently in the background through the system tray.
* Provides quick access via right-click menu on the tray icon.
* Allows copying current system information to the clipboard.
* Offers a minimal, lightweight, and responsive design for everyday diagnostics.
* Adds integration with Jira SM via a browser extension.
* Automatically attaches system information to Jira SM tickets (**available only with the extension installed**).
* Inherits localization from browser and system language settings (English, Russian, Ukrainian).

## 🛠️ Built With

![Qt](https://img.shields.io/badge/Qt%206.11.1-009639?logo=Qt&logoColor=fff)
![C++17](https://img.shields.io/badge/C++%2017-%2300599C.svg?logo=c%2B%2B&logoColor=white)
![Git](https://img.shields.io/badge/Git-F05032?logo=git&logoColor=fff)
![Chrome extension](https://img.shields.io/badge/Google%20Chrome%20Extension-4285F4?logo=GoogleChrome&logoColor=white)
![Edge extension](https://custom-icon-badges.demolab.com/badge/Microsoft%20Edge%20Extension-2771D8?logo=edge-white&logoColor=white)
![Firefox extension](https://img.shields.io/badge/Firefox%20Extension-FF7139?logo=Firefox&logoColor=white)

## ⚙️ Install

### 1. Download the latest release  

👉 [SysInfo Releases on GitHub](https://github.com/1minEpowMinX/SysInfo/releases/latest)

### 2. Extract the archive  

```sh
# Linux
sudo apt install p7zip-full
sudo 7z x SysInfo-3.1.0-linux-x64.7z -o/usr/bin/

# MacOS:
brew install p7zip
sudo 7z x SysInfo-3.1.0-macos-x64.7z -o/usr/local/bin/

# Windows:
New-EventLog -LogName "SysInfo" -Source "SysInfo"
Limit-EventLog -LogName "SysInfo" -MaximumSize 5MB
"C:\Program Files\7-Zip\7z.exe" x SysInfo-3.1.0-windows-x64.7z -o"C:\Program Files\"
```

### 3. Add to startup

```sh
# Linux
curl -L -o /etc/xdg/autostart/sysinfo.desktop https://raw.githubusercontent.com/1minEpowMinX/SysInfo/refs/heads/dev/SysInfo/resources/sysinfo.desktop

# MacOS:
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

### 4. Allow a custom browser extension *(optional)*

SysInfo accepts requests **only** from **whitelisted** browser extension IDs. Official Chrome/Edge and Firefox IDs are already included, so this is only needed for custom or corporate builds with different IDs.

The whitelist lives under the `Integration/AllowedExtensionIds` key. When the key is present and non-empty it **replaces** the built-in defaults.

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

> ℹ️ To find the ID of your own extension, open `chrome://extensions` (or `about:addons` in Firefox) with developer mode enabled.

## 🚀 Usage

After installation, SysInfo runs silently in the background via the system tray.

### 🖥️ Quick Start

* Launch SysInfo manually or let it start automatically at login (see [Add to startup](#3-add-to-startup)).
* Look for the SysInfo icon in the system tray (notification area).
* Right-click the icon to open the action menu.

### 🧩 Integration

If the browser extension is installed, SysInfo automatically attaches system details to Jira SM tickets.

### ⚠️ Notes

* The path examples in this guide assume default installation paths. Always check them and replace if necessary (**Including source files**).
* The Jira Service Management integration requires the optional browser extension.
You may need to adapt its settings or source code (e.g. URLs, ticket field mapping) to match your company’s Jira instance.

## 📘 Author

### 👤 Kyrylo Bitskyi

* Github: [@1minEpowMinX](https://github.com/1minEpowMinX)
* LinkedIn: [@Kyrylo Bitskyi](https://www.linkedin.com/in/kyrylo-bitskyi-025672284/)

## 🤝 Contributing

Contributions, issues and feature requests are welcome!  
Feel free to check [issues page](https://github.com/1minEpowMinX/SysInfo/issues).

## 📝 License

Copyright © 2026 [1minEpowMinX](https://github.com/1minEpowMinX).  
This project is [GPL-3.0](https://github.com/1minEpowMinX/SysInfo/blob/main/LICENSE) licensed.
