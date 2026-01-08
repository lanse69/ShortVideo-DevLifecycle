-- 创建复制专用用户
CREATE ROLE lepai_repl WITH REPLICATION LOGIN PASSWORD 'lepai_repl_pass';

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id VARCHAR(64) PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 视频表
CREATE TABLE IF NOT EXISTS videos (
    id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(64) REFERENCES users(id),
    title VARCHAR(100),
    url VARCHAR(255),
    like_count BIGINT DEFAULT 0,
    status INT DEFAULT 0 -- 0:Processing, 1:Published
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);