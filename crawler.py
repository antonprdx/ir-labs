import json
import os
import re
import time
from collections import deque
from urllib.parse import unquote, urlparse

import requests

# Конфигурация
API_URL = "https://ru.wikipedia.org/w/api.php"
USER_AGENT = "MyWikiCrawler/1.0 (your_email@example.com)"  # Укажите свой email
OUTPUT_FOLDER = "wiki_articles"  # Папка для сохранения статей
CATEGORIES_FILE = "sites.txt"  # Файл со списком категорий
RECURSIVE_DEPTH = (
    2  # Глубина обхода подкатегорий (0 - только указанные, 3 - три уровня вглубь)
)
REQUEST_DELAY = 0.5  # Задержка между запросами (сек)
MAX_RETRIES = 3  # Попыток при ошибке
SAVE_PROGRESS = True  # Сохранять прогресс в JSON файл
PROGRESS_FILE = "crawler_progress.json"  # Файл для сохранения прогресса

# Создаём выходную папку, если её нет
os.makedirs(OUTPUT_FOLDER, exist_ok=True)


def load_categories(file_path):
    """Читает файл со списком категорий (по одной на строку) и возвращает список названий."""
    categories = []
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            # Если это полный URL, извлекаем название категории
            if line.startswith("http"):
                parsed = urlparse(line)
                path = unquote(parsed.path)
                # Ожидаем что-то вроде /wiki/Категория:Имя
                if "/wiki/" in path:
                    title = path.split("/wiki/")[-1]
                else:
                    title = path
            else:
                title = line
            # Убедимся, что есть префикс "Категория:"
            if not title.startswith("Категория:"):
                title = "Категория:" + title
            categories.append(title)
    return categories


def make_api_request(params, retries=MAX_RETRIES):
    """Выполняет API-запрос с повторными попытками."""
    headers = {"User-Agent": USER_AGENT}
    for attempt in range(retries):
        try:
            response = requests.get(API_URL, params=params, headers=headers, timeout=30)
            response.raise_for_status()
            return response.json()
        except requests.exceptions.RequestException as e:
            print(f"Ошибка запроса: {e}. Попытка {attempt + 1}/{retries}")
            if attempt < retries - 1:
                time.sleep(REQUEST_DELAY * 2)
    return None


def get_category_members_recursive(start_categories, max_depth=RECURSIVE_DEPTH):
    """
    Рекурсивно обходит категории до указанной глубины.
    Возвращает множество названий статей и словарь обработанных категорий с их глубиной.
    """
    all_pages = set()
    processed_categories = {}  # {название_категории: глубина}

    # Очередь для BFS обхода: (название_категории, текущая_глубина)
    queue = deque([(cat, 0) for cat in start_categories])

    while queue:
        category, depth = queue.popleft()

        # Пропускаем, если уже обработали эту категорию
        if category in processed_categories:
            continue

        print(f"\nОбработка категории: {category} (глубина {depth})")
        processed_categories[category] = depth

        # Получаем страницы и подкатегории
        pages, subcategories = get_category_contents(category)

        # Добавляем страницы в общий список
        all_pages.update(pages)
        print(f"  Найдено статей: {len(pages)} (всего: {len(all_pages)})")

        # Если не достигли максимальной глубины, добавляем подкатегории в очередь
        if depth < max_depth:
            for subcat in subcategories:
                if subcat not in processed_categories:
                    queue.append((subcat, depth + 1))
                    print(f"  Добавлена подкатегория в очередь: {subcat}")

        time.sleep(REQUEST_DELAY)

    return all_pages, processed_categories


def get_category_contents(category):
    """
    Получает все страницы и подкатегории из одной категории.
    Возвращает (множество_страниц, множество_подкатегорий)
    """
    pages = set()
    subcategories = set()
    cmcontinue = None

    while True:
        params = {
            "action": "query",
            "list": "categorymembers",
            "cmtitle": category,
            "cmlimit": "max",
            "cmtype": "page|subcat",  # Получаем и страницы, и подкатегории
            "format": "json",
        }
        if cmcontinue:
            params["cmcontinue"] = cmcontinue

        data = make_api_request(params)
        if not data:
            break

        members = data.get("query", {}).get("categorymembers", [])
        for member in members:
            ns = member.get("ns")
            title = member.get("title")

            if ns == 0:  # Основное пространство имён (статьи)
                pages.add(title)
            elif ns == 14:  # Пространство имён "Категория"
                subcategories.add(title)

        if "continue" in data and "cmcontinue" in data["continue"]:
            cmcontinue = data["continue"]["cmcontinue"]
        else:
            break

    return pages, subcategories


def get_page_text(title):
    """Получает чистый текст статьи по её названию."""
    params = {
        "action": "query",
        "prop": "extracts",
        "titles": title,
        "explaintext": True,
        "format": "json",
    }
    data = make_api_request(params)
    if not data:
        return None

    pages = data.get("query", {}).get("pages", {})
    for page_id, page_info in pages.items():
        if page_id == "-1":
            print(f"Страница '{title}' не найдена.")
            return None
        return page_info.get("extract", "")
    return None


def sanitize_filename(filename):
    """Заменяет недопустимые символы в имени файла на подчёркивание."""
    return re.sub(r'[\\/*?:"<>|]', "_", filename)


def save_article(title, text, folder):
    """Сохраняет текст статьи в файл .txt."""
    if not text:
        return False
    safe_title = sanitize_filename(title)
    filepath = os.path.join(folder, f"{safe_title}.txt")

    # Проверяем, не сохранена ли уже статья
    if os.path.exists(filepath):
        print(f"Статья уже существует: {title}")
        return True

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(text)
    print(f"Сохранено: {title}")
    return True


def save_progress(processed_categories, saved_articles, categories_file):
    """Сохраняет прогресс в JSON файл."""
    progress = {
        "processed_categories": processed_categories,
        "saved_articles": list(saved_articles),
        "categories_file": categories_file,
        "timestamp": time.time(),
    }
    with open(PROGRESS_FILE, "w", encoding="utf-8") as f:
        json.dump(progress, f, ensure_ascii=False, indent=2)
    print(f"\nПрогресс сохранён в {PROGRESS_FILE}")


def load_progress():
    """Загружает прогресс из JSON файла, если он существует."""
    if os.path.exists(PROGRESS_FILE):
        try:
            with open(PROGRESS_FILE, "r", encoding="utf-8") as f:
                progress = json.load(f)
            print(f"Загружен прогресс из {PROGRESS_FILE}")
            return progress
        except:
            print("Не удалось загрузить прогресс, начинаем сначала")
    return None


def get_existing_articles():
    """Возвращает множество названий уже сохранённых статей."""
    existing = set()
    if os.path.exists(OUTPUT_FOLDER):
        for filename in os.listdir(OUTPUT_FOLDER):
            if filename.endswith(".txt"):
                # Убираем расширение .txt и восстанавливаем оригинальное название
                title = filename[:-4]
                # Здесь можно добавить обратное преобразование sanitize_filename,
                # но для простоты будем считать, что имена файлов уникальны
                existing.add(title)
    return existing


def main():
    print("Загрузка списка категорий...")
    categories = load_categories(CATEGORIES_FILE)
    print(f"Найдено категорий: {len(categories)}")

    # Проверяем существующие статьи
    existing_articles = get_existing_articles()
    print(f"Уже сохранено статей: {len(existing_articles)}")

    # Загружаем прогресс, если нужно
    processed_categories = {}
    saved_articles = set(existing_articles)

    if SAVE_PROGRESS:
        progress = load_progress()
        if progress and progress.get("categories_file") == CATEGORIES_FILE:
            processed_categories = progress.get("processed_categories", {})
            saved_articles.update(progress.get("saved_articles", []))
            print(f"Загружено обработанных категорий: {len(processed_categories)}")

    # Получаем все статьи из категорий с учётом глубины
    print(f"\nНачинаем обход категорий с глубиной {RECURSIVE_DEPTH}...")

    # Фильтруем уже обработанные категории
    categories_to_process = [
        cat for cat in categories if cat not in processed_categories
    ]

    if categories_to_process:
        new_pages, new_processed = get_category_members_recursive(categories_to_process)

        # Объединяем результаты
        all_articles = set(new_pages)

        # Обновляем обработанные категории
        processed_categories.update(new_processed)
    else:
        print("Все категории уже обработаны")
        all_articles = set()

    # Добавляем уже сохранённые статьи, чтобы не загружать их повторно
    articles_to_download = all_articles - saved_articles

    print(f"\nВсего уникальных статей для загрузки: {len(articles_to_download)}")
    print(f"Всего уникальных статей в категориях: {len(all_articles)}")

    if not articles_to_download:
        print("Новых статей для загрузки нет.")
        return

    # Загрузка и сохранение каждой статьи
    for i, title in enumerate(sorted(articles_to_download), 1):
        print(f"[{i}/{len(articles_to_download)}] Загружаем: {title}")
        text = get_page_text(title)
        if text:
            if save_article(title, text, OUTPUT_FOLDER):
                saved_articles.add(title)
        else:
            print(f"  Не удалось получить текст статьи '{title}'")

        # Сохраняем прогресс каждые 10 статей
        if SAVE_PROGRESS and i % 10 == 0:
            save_progress(processed_categories, saved_articles, CATEGORIES_FILE)

        time.sleep(REQUEST_DELAY)

    # Финальное сохранение прогресса
    if SAVE_PROGRESS:
        save_progress(processed_categories, saved_articles, CATEGORIES_FILE)

    print(f"\nГотово! Сохранено статей: {len(saved_articles)}")
    print(f"Обработано категорий: {len(processed_categories)}")


def print_category_tree(start_categories, max_depth=RECURSIVE_DEPTH):
    """Вспомогательная функция для просмотра дерева категорий без загрузки статей."""
    print(f"\nДерево категорий (глубина {max_depth}):")

    queue = deque([(cat, 0) for cat in start_categories])
    visited = set()

    while queue:
        category, depth = queue.popleft()

        if category in visited:
            continue

        visited.add(category)
        indent = "  " * depth
        print(f"{indent}📁 {category}")

        if depth < max_depth:
            _, subcategories = get_category_contents(category)
            for subcat in sorted(subcategories)[
                :5
            ]:  # Показываем только первые 5 подкатегорий
                if subcat not in visited:
                    queue.append((subcat, depth + 1))

            if len(subcategories) > 5:
                print(f"{indent}  ... и ещё {len(subcategories) - 5} подкатегорий")

        time.sleep(REQUEST_DELAY)


if __name__ == "__main__":
    import sys

    # Если передан аргумент --tree, показываем только дерево категорий
    if len(sys.argv) > 1 and sys.argv[1] == "--tree":
        categories = load_categories(CATEGORIES_FILE)
        print_category_tree(categories)
    else:
        main()
