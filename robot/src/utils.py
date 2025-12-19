import hashlib
from urllib.parse import urlparse, urlunparse


def normalize_url(url: str) -> str:
    """
    Приводит URL к каноническому виду
    - Нижний регистр схемы и хоста
    - Удаление фрагментов (#anchor)
    - Удаление лишних query параметров (utm_*, etc)
    - Удаление слеша в конце (для единообразия)
    """
    parsed = urlparse(url)

    scheme = parsed.scheme.lower()
    netloc = parsed.netloc.lower()
    path = parsed.path

    if path != '/' and path.endswith('/'):
        path = path[:-1]

    clean_url = urlunparse((scheme, netloc, path, parsed.params, parsed.query, ""))

    return clean_url


def get_url_hash(url: str) -> str:
    """Возвращает MD5 хеш от нормализованного URL."""
    return hashlib.md5(url.encode('utf-8')).hexdigest()