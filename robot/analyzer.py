import yaml
import logging
from bs4 import BeautifulSoup
import psycopg2
import os

CONFIG_PATH = "config.yaml"
OUTPUT_FILE = "analysis_report.txt"


def load_config(path):
    with open(path, 'r', encoding='utf-8') as f:
        return yaml.safe_load(f)


def clean_html(html_content):
    """Выделяет чистый текст из HTML."""
    if not html_content:
        return ""
    soup = BeautifulSoup(html_content, 'lxml')
    for script in soup(["script", "style", "header", "footer", "nav"]):
        script.extract()
    return soup.get_text(separator=' ', strip=True)


def analyze():
    config = load_config(CONFIG_PATH)
    db_conf = config['db']

    conn = psycopg2.connect(
        host=db_conf['host'],
        port=db_conf['port'],
        user=db_conf['user'],
        password=db_conf['password'],
        dbname=db_conf['dbname']
    )

    print("Подключение к БД успешно. Начинаю анализ...")

    stats = {
        'total_docs': 0,
        'consultant_docs': 0,
        'business_docs': 0,
        'total_raw_bytes': 0,
        'total_text_bytes': 0,
        'docs_lengths_raw': [],
        'docs_lengths_text': []
    }

    examples = {}  

    with conn.cursor() as cur:
        cur.execute("SELECT source_name, raw_html, url FROM documents")

        row_count = 0
        while True:
            rows = cur.fetchmany(100)
            if not rows:
                break

            for source, raw_html, url in rows:
                row_count += 1
                if row_count % 100 == 0:
                    print(f"Обработано {row_count} документов...", end='\r')

                if 'consultant' in source:
                    stats['consultant_docs'] += 1
                else:
                    stats['business_docs'] += 1

                stats['total_docs'] += 1

                raw_len = len(raw_html.encode('utf-8'))
                stats['total_raw_bytes'] += raw_len
                stats['docs_lengths_raw'].append(raw_len)

                clean_text = clean_html(raw_html)
                text_len = len(clean_text.encode('utf-8'))
                stats['total_text_bytes'] += text_len
                stats['docs_lengths_text'].append(text_len)

                if source not in examples:
                    examples[source] = {
                        'url': url,
                        'raw_snippet': raw_html[:1000], 
                        'clean_snippet': clean_text[:1000]  
                    }

    conn.close()

    avg_raw = stats['total_raw_bytes'] / stats['total_docs'] if stats['total_docs'] else 0
    avg_text = stats['total_text_bytes'] / stats['total_docs'] if stats['total_docs'] else 0

    report = []
    report.append("=== СТАТИСТИКА КОРПУСА ===")
    report.append(f"Всего документов: {stats['total_docs']}")
    report.append(f"Consultant.ru: {stats['consultant_docs']}")
    report.append(f"Business.ru: {stats['business_docs']}")
    report.append("-" * 20)
    report.append(f"Общий размер сырых данных (HTML): {stats['total_raw_bytes'] / 1024 / 1024:.2f} MB")
    report.append(f"Общий размер чистого текста: {stats['total_text_bytes'] / 1024 / 1024:.2f} MB")
    report.append("-" * 20)
    report.append(f"Средний размер HTML документа: {avg_raw / 1024:.2f} KB")
    report.append(f"Средний размер текста в документе: {avg_text / 1024:.2f} KB")
    report.append(f"Ratio (Текст/HTML): {avg_text / avg_raw:.2%}")
    report.append("\n=== ПРИМЕРЫ ДАННЫХ ===")

    for source, data in examples.items():
        report.append(f"\nИсточник: {source}")
        report.append(f"URL: {data['url']}")
        report.append(f"--- Raw HTML (snippet) ---\n{data['raw_snippet']}...")
        report.append(f"--- Clean Text (snippet) ---\n{data['clean_snippet']}...")

    with open(OUTPUT_FILE, 'w', encoding='utf-8') as f:
        f.write('\n'.join(report))

    


if __name__ == "__main__":
    analyze()