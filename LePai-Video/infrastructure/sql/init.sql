-- 创建复制专用用户
DO $$
BEGIN
  IF NOT EXISTS (SELECT FROM pg_catalog.pg_roles WHERE rolname = 'lepai_repl') THEN
    CREATE ROLE lepai_repl WITH REPLICATION LOGIN PASSWORD 'lepai_repl_pass';
  END IF;
END
$$;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id VARCHAR(64) PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    following_count INT DEFAULT 0,
    follower_count INT DEFAULT 0,
    avatar_url VARCHAR(255) DEFAULT '/public/defaults/default.png', -- 默认头像
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 视频表
CREATE TABLE IF NOT EXISTS videos (
    id VARCHAR(64) PRIMARY KEY,
    user_id VARCHAR(64) REFERENCES users(id),
    title VARCHAR(100),
    url VARCHAR(255) NOT NULL,         -- 视频播放地址
    cover_url VARCHAR(255),            -- 封面图地址
    duration INT DEFAULT 0,            -- 时长(秒)
    like_count BIGINT DEFAULT 0,
    status INT DEFAULT 0,              -- 0:处理中, 1:已发布, 2:失败
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 点赞关联表
CREATE TABLE IF NOT EXISTS video_likes (
    user_id VARCHAR(64) REFERENCES users(id),
    video_id VARCHAR(64) REFERENCES videos(id),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, video_id)
);

-- 关注关系表
CREATE TABLE IF NOT EXISTS user_follows (
    follower_id VARCHAR(64) NOT NULL REFERENCES users(id), -- 谁点的关注 (A)
    following_id VARCHAR(64) NOT NULL REFERENCES users(id), -- 关注了谁 (B)
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (follower_id, following_id)
);

-- 索引
CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE INDEX IF NOT EXISTS idx_videos_created_at ON videos(created_at DESC);
CREATE INDEX IF NOT EXISTS idx_videos_user_id ON videos(user_id);
CREATE INDEX IF NOT EXISTS idx_video_likes_user ON video_likes(user_id);
CREATE INDEX IF NOT EXISTS idx_follows_follower ON user_follows(follower_id);
CREATE INDEX IF NOT EXISTS idx_follows_following ON user_follows(following_id);