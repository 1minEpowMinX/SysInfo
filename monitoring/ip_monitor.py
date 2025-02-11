from ctypes import windll, c_ulong, byref
from socket import gethostbyname, gethostname, gaierror
from time import sleep


def get_current_ip():
    """Получает текущий IP-адрес с обработкой ошибок."""
    try:
        return gethostbyname(gethostname())
    except gaierror as e:  # Обработка ошибки в случае проблем получения IP
        sleep(5)
        return get_current_ip()  # Повторная попытка получения IP


def monitor_ip_change(icon, get_system_info, format_system_info):
    """Следит за изменением IP через Windows API и обновляет трей."""
    iphlpapi = windll.iphlpapi
    event = c_ulong()

    iphlpapi.NotifyAddrChange(byref(event), None)

    # Сохраняем начальную информацию о системе
    system_info = get_system_info()

    while True:
        windll.kernel32.WaitForSingleObject(event, -1)  # Ожидание изменения IP
        new_ip = get_current_ip()

        if new_ip != system_info.ip_address:
            system_info = system_info._replace(ip_address=new_ip)  # Обновляем только IP
            icon.title = format_system_info(
                system_info
            )  # Обновляем только заголовок с новой информацией

        # Ожидание перед следующей проверкой для снижения нагрузки ЦП
        sleep(1)
