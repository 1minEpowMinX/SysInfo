import sys
import ctypes
from tray.tray_icon import create_tray_icon
from config.config import is_first_run, set_first_run_complete
from messages.first_run import show_first_run_message


def main():
    """Основная функция программы."""
    mutex_name = "SysInfoMutex"
    handle = ctypes.windll.kernel32.CreateMutexW(None, False, mutex_name)
    last_error = ctypes.windll.kernel32.GetLastError()

    if last_error == 183:  # ERROR_ALREADY_EXISTS
        sys.exit()

    if is_first_run():
        show_first_run_message()
        set_first_run_complete()

    icon = create_tray_icon()
    icon.run()

    ctypes.windll.kernel32.ReleaseMutex(handle)


if __name__ == "__main__":
    main()
