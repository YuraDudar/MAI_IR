import psycopg2
import yaml
from pathlib import Path
from bs4 import BeautifulSoup
from bs4.element import Tag
import re

CONFIG_PATH = "config.yaml"  
OUTPUT_DIR = Path("corpus_txt")  


def parse_consultant_html(html_content: str) -> str | None:
    try:
        soup = BeautifulSoup(html_content, 'lxml')
        content_div = soup.find('div', class_='news-page__text')
        if not content_div:
            return None

        title_tag = soup.find('h1', class_='news-page__title')
        title = title_tag.get_text(strip=True) if title_tag else ""

        text_blocks = [title]

        for element in content_div.children:
            if isinstance(element, Tag):
                if element.name in ['h2', 'p']:
                    clean_text = element.get_text(' ', strip=True)
                    if clean_text:
                        text_blocks.append(clean_text)
                elif element.name == 'ul':
                    for li in element.find_all('li'):
                        text_blocks.append(li.get_text(' ', strip=True))

        return "\n".join(text_blocks)
    except Exception:
        return None


def parse_business_html(html_content: str) -> str | None:
    try:
        soup = BeautifulSoup(html_content, 'lxml')
        content_div = soup.find('div', itemprop='articleBody', class_='contentSchema')
        if not content_div:
            return None

        title_tag = soup.find('h1', class_='page__title')
        title = title_tag.get_text(strip=True) if title_tag else ""

        text_blocks = [title]

        all_paragraphs = content_div.find_all('p')
        for p in all_paragraphs:
            if p.find_parent('noindex'):
                continue
            clean_text = p.get_text(' ', strip=True)
            if clean_text:
                text_blocks.append(clean_text)

        return "\n".join(text_blocks)
    except Exception:
        return None


def load_config(path: str) -> dict:
    with open(path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)


def main():
    config = load_config(CONFIG_PATH)
    db_conf = config['db']

    conn = psycopg2.connect(
        host=db_conf['host'],
        port=db_conf['port'],
        user=db_conf['user'],
        password=db_conf['password'],
        dbname=db_conf['dbname']
    )

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    print("Начинаем экспорт документов из БД...")

    with conn.cursor() as cursor:
        cursor.execute("SELECT id, source_name, raw_html FROM documents")

        count = 0
        skipped = 0

        while True:
            rows = cursor.fetchmany(100)
            if not rows:
                break

            for doc_id, source, html in rows:
                parsed_text = None

                if source == 'consultant':
                    parsed_text = parse_consultant_html(html)
                elif source == 'business_ru':
                    parsed_text = parse_business_html(html)

                if parsed_text:
                    file_path = OUTPUT_DIR / f"doc_{doc_id}.txt"
                    file_path.write_text(parsed_text, encoding='utf-8')
                    count += 1
                else:
                    skipped += 1

            print(f"\rОбработано: {count} (Пропущено: {skipped})", end="")

    print(f"\nГотово! Файлы сохранены в {OUTPUT_DIR.resolve()}")
    conn.close()


if __name__ == "__main__":
    main()