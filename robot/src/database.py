import psycopg2
import logging
from datetime import datetime


class Database:
    def __init__(self, config: dict):
        self.conn = psycopg2.connect(
            host=config['host'],
            port=config['port'],
            user=config['user'],
            password=config['password'],
            dbname=config['dbname']
        )
        self.conn.autocommit = True 
        self.logger = logging.getLogger("Database")
        self.logger.info("Connected to PostgreSQL")

    def document_exists(self, url_hash: str) -> bool:
        """Проверяет наличие документа по хешу URL."""
        with self.conn.cursor() as cursor:
            cursor.execute("SELECT 1 FROM documents WHERE url_hash = %s", (url_hash,))
            return cursor.fetchone() is not None

    def save_document(self, url: str, url_hash: str, source_name: str, raw_html: str):
        """Сохраняет документ в базу."""
        query = """
            INSERT INTO documents (url, url_hash, source_name, raw_html, crawled_at)
            VALUES (%s, %s, %s, %s, %s)
            ON CONFLICT (url_hash) DO NOTHING;
        """

        timestamp = int(datetime.now().timestamp())

        try:
            with self.conn.cursor() as cursor:
                cursor.execute(query, (url, url_hash, source_name, raw_html, timestamp))
        except Exception as e:
            self.logger.error(f"Error saving document {url}: {e}")

    def close(self):
        if self.conn:
            self.conn.close()
            self.logger.info("Database connection closed")