import yaml
import logging
import sys
from src.database import Database
from src.crawler import Crawler

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("crawler.log"),
        logging.StreamHandler(sys.stdout)
    ]
)
logger = logging.getLogger("Main")


def load_config(path: str) -> dict:
    try:
        with open(path, 'r', encoding='utf-8') as f:
            return yaml.safe_load(f)
    except Exception as e:
        logger.error(f"Failed to load config: {e}")
        sys.exit(1)


def main():
    config_path = "config.yaml"
    config = load_config(config_path)

    logger.info("Initializing search engine crawler...")

    try:
        db = Database(config['db'])
    except Exception as e:
        logger.critical(f"Could not connect to database: {e}")
        sys.exit(1)

    try:
        crawler = Crawler(config, db)
        crawler.run()
    except KeyboardInterrupt:
        logger.info("Crawler stopped by user.")
    except Exception as e:
        logger.exception(f"Unexpected error: {e}")
    finally:
        db.close()
        logger.info("Exiting.")


if __name__ == "__main__":
    main()