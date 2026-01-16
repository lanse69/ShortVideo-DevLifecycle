# 乐拍视界 (LePai Vision) - 分布式短视频系统

本项目是一个基于 C++ (Drogon, Qt) 构建的分布式短视频平台，包含 API 网关、转码服务、CDN 分发、数据库主从复制等组件。

## 服务端快速启动 (Quick Start)

本项目服务端采用**一键式部署脚本**，支持单机全量部署（开发测试）和三机分布式部署（生产模拟）。

### 1. 准备工作
*   操作系统：推荐 Ubuntu 20.04+ 或 Manjaro。
*   确保 `assets_source` 文件夹位于项目根目录，且其中包含 `default.png` 和 `failed.jpeg`（用于初始化对象存储）。
*   确保当前用户拥有 `sudo` 权限。

### 2. 执行服务端部署脚本
服务端部署的核心入口脚本位于 `infrastructure/` 目录下。

```bash
# 1. 进入基础设施目录
cd infrastructure

# 2. 赋予脚本执行权限
chmod +x deploy_system.sh uninstall_system.sh scripts/*.sh

# 3. 运行一键部署工具
./deploy_system.sh

# 查看 API 日志
tail -f /opt/lepai/logs/api.log

# 查看 Worker 日志
tail -f /opt/lepai/logs/worker.log

# 查看 错误 日志 (如果有报错看这里)
tail -f /opt/lepai/logs/api_error.log
```

### 3. 按照向导操作
脚本运行后，请按照提示进行操作：

1.  **安装依赖**：首次运行请选择 `Y`，脚本将自动安装 Qt6, Drogon, FFmpeg, Docker 等系统依赖。
2.  **配置环境**：脚本会自动检测本机 IP，并生成 `config.json` 和 Nginx 配置。
3.  **选择角色**：
    *   如果是**单机测试**，请输入 **`4` (全部启动)**。
    *   如果是**分布式部署**，请根据当前机器的角色选择 `1` (数据库/MinIO)、`2` (API服务) 或 `3` (转码Worker)。

> **注意**：脚本会自动完成编译、Docker 容器启动、数据库初始化、Systemd 服务注册等所有流程。

### 4. 启动客户端
服务端部署完成后，即可配置环境并编译和运行客户端：
```bash
# 如果客户端没有和某个服务端在一台机器上运行
# 则客户端需要自己运行一遍环境配置向导
cd infrastructure/scripts
./configure_env.sh
```


### 5. 卸载与清理
如果需要重置环境或清空数据：
```bash
cd infrastructure
./uninstall_system.sh
```

---

## 目录结构与文件详解

### 1. 根目录
*   **`assets_source/`**: 存放初始化资源（默认头像、加载失败图）。`setup_minio_buckets.sh` 会将这些文件上传到 MinIO。
*   **`client/`**: Qt/QML 客户端源码。
*   **`server/`**: C++ 服务端源码。
*   **`infrastructure/`**: 所有的运维、部署、配置、数据库脚本都在这里。

### 2. Infrastructure (基础设施) 详解

#### 根文件
*   **`deploy_system.sh`**: **[核心]** 部署总入口。负责调用其他子脚本，执行编译、安装、Docker 启动和服务注册。
*   **`uninstall_system.sh`**: **[核心]** 卸载入口。停止服务、删除 Docker 容器、清理安装目录和数据卷。
*   **`docker-compose.yml`**: Docker 编排文件（由模板生成），定义了 Postgres、Redis、MinIO、Nginx 等容器的运行参数。

#### `scripts/` (执行脚本库)
被 `deploy_system.sh` 调用的底层功能脚本。
*   **`install_dependencies.sh`**: 检测系统版本（Ubuntu/Manjaro），安装 C++ 编译环境、Qt、FFmpeg、Docker 等依赖。
*   **`configure_env.sh`**: 交互式脚本。获取局域网 IP，将 `config_templates` 中的模板替换为真实配置（如 server_config.json）。
*   **`setup_docker.sh`**: 根据选择的角色（PC1/2/3），启动 `docker-compose.yml` 中对应的容器组（Profile）。
*   **`setup_minio_buckets.sh`**: MinIO 启动后的初始化脚本。创建 `public` 和 `temp` 存储桶，设置读写权限，并上传默认图片。
*   **`cleanup_docker.sh`**: 强制停止并删除相关容器和数据卷。

#### `config_templates/` (配置模板)
包含占位符（如 `{{PC1_IP}}`）的模板文件，用于生成最终配置。
*   **`server_config.json`**: 服务端配置模板（数据库地址、Redis地址等）。
*   **`client_config.json`**: 客户端配置模板（API地址、CDN地址）。
*   **`docker-compose.yml.template`**: Docker 编排模板。
*   **`nginx_gateway.conf.template`**: API 网关（PC-2）的 Nginx 配置，负责反向代理 80 端口到 8080 服务。
*   **`nginx_cdn.conf.template`**: 边缘节点（PC-3）的 Nginx 配置，负责缓存 MinIO 的视频流，模拟 CDN。

#### `sql/` (数据库初始化)
*   **`init.sql`**: 数据库建表脚本。定义了 `users` (用户), `videos` (视频), `video_likes` (点赞), `user_follows` (关注) 等表结构及索引。
*   **`00_allow_repl.sh`**: PostgreSQL 初始化钩子。在数据库容器首次启动时执行，修改 `pg_hba.conf` 以允许从库进行数据复制 (Replication)。

#### `systemd/` (后台服务管理)
用于将编译好的 C++ 程序注册为 Linux 系统服务，实现开机自启和崩溃重启。
*   **`lepai-api.service`**: API 服务的 Systemd 配置文件。
*   **`lepai-worker.service`**: 转码 Worker 的 Systemd 配置文件。
*   **`install_server.sh`**: 服务端安装脚本。

#### `nginx_gateway/` & `nginx_cdn/` & `nginx_cache/`
*   这些目录用于存放生成的 Nginx 配置文件，以及挂载 Nginx 的本地缓存文件（CDN 节点使用）。

---

### 3. Server (服务端) 详解
*   **`common/`**: 公共库。包含配置管理器 (`ConfigManager`)、MinIO 客户端封装、UUID生成等工具类。
*   **`api_service/`**: **核心业务服务**。
    *   基于 Drogon 框架。
    *   处理用户注册登录、视频流拉取、点赞关注等 HTTP 请求。
    *   负责读写分离逻辑（写主库，读从库/Redis）。
*   **`video_worker/`**: **后台转码服务**。
    *   监听 Redis 队列 (`video_queue`)。
    *   使用 FFmpeg 将上传的视频转码为 HLS (m3u8+ts) 切片。
    *   生成封面图并上传回 MinIO。

---

## 常见问题

**Q: 启动后无法登录？**
A: 请检查 Docker 容器状态 `docker ps`，确保 PostgreSQL 和 Redis 运行正常。如果是第一次部署，请注销账户重新登录并确认 `setup_minio_buckets.sh` 是否执行成功（需要等待 MinIO 完全启动）。

**Q: 视频无法播放？**
A: 检查 PC-3 的 Worker 服务日志 `/opt/lepai/logs/worker.log`，确认转码是否成功。同时检查客户端配置中的 CDN 地址是否指向了 PC-3 的 IP。
