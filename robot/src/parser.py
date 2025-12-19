from bs4 import BeautifulSoup
from urllib.parse import urljoin
import logging


class Parser:
    def __init__(self):
        self.logger = logging.getLogger("Parser")

    def parse_listing(self, html_content: str, current_url: str, source_config: dict) -> tuple[list[str], str | None]:
        if not html_content:
            return [], None

        try:
            soup = BeautifulSoup(html_content, 'lxml')

            # 1. Ссылки
            article_urls = []
            selector = source_config.get('item_selector')
            if selector:
                items = soup.select(selector)
                for item in items:
                    href = item.get('href')
                    if href:
                        full_url = urljoin(current_url, href)
                        article_urls.append(full_url)

            next_page_url = None
            pagination_type = source_config.get('pagination_type')

            if pagination_type == 'html_link':
                next_btn_selector = source_config.get('next_btn_selector')
                if next_btn_selector:
                    next_btn = soup.select_one(next_btn_selector)
                    if next_btn and next_btn.get('href'):
                        next_page_url = urljoin(current_url, next_btn.get('href'))

            return article_urls, next_page_url

        except Exception as e:
            self.logger.error(f"Error parsing HTML from {current_url}: {e}")
            return [], None

    def parse_article(self, html_content: str) -> str:
        return html_content