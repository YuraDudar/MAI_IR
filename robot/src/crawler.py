import time
import logging
import random
import requests
from urllib.parse import urlparse
from collections import defaultdict

from .parser import Parser
from .database import Database
from .utils import normalize_url, get_url_hash

CAPTCHA_MARKERS = [
    "captcha",
    "recaptcha",
    "g-recaptcha",
    "доступ ограничен",
    "security check",
    "подтвердите, что вы не робот",
    "403 forbidden",
    "waf",
]


class CrawlerError(Exception):
    pass


class CaptchaDetectedError(CrawlerError):
    pass


class Crawler:
    def __init__(self, config: dict, db: Database):
        self.config = config
        self.db = db
        self.parser = Parser()
        self.logger = logging.getLogger("Crawler")

        logic_conf = config.get('logic', {})
        self.base_delay = logic_conf.get('delay', 2.0)
        self.user_agent = logic_conf.get('user_agent', "Mozilla/5.0")

        self.stats = defaultdict(int)

        self.session = requests.Session()
        self._update_headers()

    def _update_headers(self, referer: str = None):
        headers = {
            'User-Agent': self.user_agent,
            'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8',
            'Accept-Language': 'ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7',
            'Connection': 'keep-alive',
            'Upgrade-Insecure-Requests': '1',
            'Cache-Control': 'max-age=0',
        }
        if referer:
            headers['Referer'] = referer
        self.session.headers.update(headers)

    def _sleep(self, factor=1.0):
        jitter = random.uniform(0.1, 0.5)
        sleep_time = (self.base_delay * factor) + jitter
        time.sleep(sleep_time)

    def _check_for_captcha(self, html: str, status_code: int):
        lower_html = html.lower()
        if status_code == 403:
            raise CaptchaDetectedError(f"Status 403 Forbidden. Content len: {len(html)}")
        for marker in CAPTCHA_MARKERS:
            if marker in lower_html and len(html) < 5000:
                raise CaptchaDetectedError(f"Captcha marker found: '{marker}'")

    def _fetch(self, url: str) -> str | None:
        attempts = 3
        for attempt in range(1, attempts + 1):
            try:
                response = self.session.get(url, timeout=15)
                self._check_for_captcha(response.text, response.status_code)

                if response.status_code == 200:
                    return response.text
                elif response.status_code == 404:
                    self.logger.warning(f" [404] Page not found: {url}")
                    return None
                else:
                    self.logger.warning(f" [Status {response.status_code}] fetching {url}")
                    return None

            except CaptchaDetectedError as e:
                self.logger.critical(f" [!!!] CAPTCHA DETECTED on {url}: {e}")
                self.logger.warning("Pausing for 60 seconds...")
                time.sleep(60)
                if attempt == attempts:
                    self.stats['captcha_hits'] += 1
                    return None

            except requests.RequestException as e:
                self.logger.error(f" [Network Error] Attempt {attempt}/{attempts}: {e}")
                self._sleep(factor=2 * attempt)

            except Exception as e:
                self.logger.exception(f" [Critical] Error fetching {url}: {e}")
                return None

        self.stats['errors'] += 1
        return None

    def run(self):
        sources = self.config.get('sources', [])
        print("\n" + "=" * 60)
        self.logger.info("CRAWLER STARTED")
        print("=" * 60 + "\n")

        try:
            for source_conf in sources:
                self._process_source(source_conf)
        except KeyboardInterrupt:
            self.logger.warning("Crawler stopped by user.")
        finally:
            self._print_stats()

    def _process_source(self, source_conf: dict):
        source_name = source_conf.get('name')
        current_url = source_conf.get('start_url')
        logic_conf = self.config.get('logic', {})
        max_pages = logic_conf.get('max_pages', 100)

        self.logger.info(f"--> Starting source: {source_name.upper()}")

        for page_num in range(1, max_pages + 1):
            self.logger.info(f"Processing listing page {page_num}: {current_url}")

            html = self._fetch(current_url)
            if not html:
                self.logger.error(f"Failed to fetch listing page. Stopping source.")
                break

            article_urls, next_page_url = self.parser.parse_listing(html, current_url, source_conf)

            if next_page_url:
                self.logger.info(f"Next page: {next_page_url}")
            else:
                self.logger.info("Next page: None (End of list)")

            if not article_urls:
                self.logger.warning(f"No articles found on page {page_num}.")

            saved_count = 0
            skipped_count = 0

            for url in article_urls:
                norm_url = normalize_url(url)
                url_hash = get_url_hash(norm_url)

                if self.db.document_exists(url_hash):
                    skipped_count += 1
                    self.stats['skipped'] += 1
                    continue

                self._sleep()
                article_html = self._fetch(norm_url)

                if article_html:
                    self.db.save_document(norm_url, url_hash, source_name, article_html)
                    saved_count += 1
                    self.stats['downloaded'] += 1
                    self.logger.info(f"Saved: {norm_url}")

            self.stats['pages_scanned'] += 1

            print(f"--- Page {page_num} finished: {saved_count} saved, {skipped_count} skipped ---")

            if next_page_url and next_page_url != current_url:
                self._update_headers(referer=current_url)
                current_url = next_page_url
                self._sleep()
            else:
                break

    def _print_stats(self):
        print("\n" + "=" * 30)
        print("   CRAWLER SESSION STATS   ")
        print("=" * 30)
        print(f" Pages Scanned:   {self.stats['pages_scanned']}")
        print(f" Docs Downloaded: {self.stats['downloaded']}")
        print(f" Docs Skipped:    {self.stats['skipped']}")
        print(f" Errors:          {self.stats['errors']}")
        print(f" Captcha Hits:    {self.stats['captcha_hits']}")
        print("=" * 30 + "\n")