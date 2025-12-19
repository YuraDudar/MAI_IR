CREATE TABLE IF NOT EXISTS documents (
    id SERIAL PRIMARY KEY,
    url TEXT NOT NULL,
    url_hash VARCHAR(64) UNIQUE NOT NULL,
    source_name VARCHAR(50),
    raw_html TEXT,
    crawled_at BIGINT
);

CREATE INDEX IF NOT EXISTS idx_url_hash ON documents(url_hash);