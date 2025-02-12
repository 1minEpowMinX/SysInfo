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
    """Следит за изменением IP через регулярный опрос и обновляет трей."""
    # Сохраняем начальную информацию о системе
    system_info = get_system_info()

    while True:
        new_ip = get_current_ip()

        if new_ip != system_info.ip_address:
            system_info = system_info._replace(ip_address=new_ip)  # Обновляем только IP
            icon.title = format_system_info(
                system_info
            )  # Обновляем только заголовок с новой информацией

        # Ожидание перед следующей проверкой для снижения нагрузки ЦП
        sleep(1)
