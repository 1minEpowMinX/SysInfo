from json import load, dump
from os import path

CONFIG_FILE = "sysinfo_config.json"


def is_first_run():
    """Проверяет, первый ли запуск программы."""
    if not path.exists(CONFIG_FILE):
        return True
    with open(CONFIG_FILE, "r") as f:
        data = load(f)
        return data.get("first_run", True)


def set_first_run_complete():
    """Помечает первый запуск завершённым."""
    with open(CONFIG_FILE, "w") as f:
        dump({"first_run": False}, f)
