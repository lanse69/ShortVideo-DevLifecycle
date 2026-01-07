# 乐拍视界 (LePai Vision) · 实施手册

## 1. 项目总览与核心策略

*   **部署策略**：**私有云物理集群**。不依赖公有云（阿里云等），使用 **3台局域网电脑** 模拟真实的分布式互联网架构。

---

## 2. 物理架构：三台电脑

我们将 3 台电脑处于同一局域网，模拟真实生产环境的**物理隔离**。

### **基础设施与数据中心 (The Vault)**
*   **别名**：PC-1
*   **角色**：负责“存东西”，硬盘空间需大。
*   **部署组件**：
    *   **PostgreSQL (Master)**：主库，负责写入（注册、发布视频、点赞落库）。
    *   **PostgreSQL (Slave)**：从库，负责读取（刷视频列表），配置 Binlog 同步主库。
    *   **Redis**：内存数据库，存 Token、热点视频列表、点赞缓冲池。
    *   **MinIO**：对象存储（替代阿里云 OSS），存视频实体文件、图片。

### **应用服务与网关 (The Brain)**
*   **别名**：PC-2
*   **角色**：负责“处理逻辑”，CPU 响应需快。
*   **部署组件**：
    *   **Nginx (Gateway)**：API 网关，统一入口，反向代理到 8080。
    *   **API Service**：核心后端程序（C++），处理业务逻辑。

### **边缘计算与媒体处理 (The Muscle)**
*   **别名**：PC-3
*   **角色**：负责“干苦力”和“分发”。
*   **部署组件**：
    *   **FFmpeg Worker**：后台进程，监听 Redis 队列，进行视频转码。
    *   **Nginx (CDN)**：模拟 CDN 节点，开启 `proxy_cache`，反向代理 PC-1 的 MinIO。

---

## 3. 代码管理规范

### 3.1 目录结构
```text
LePai-Video/
├── docs/
├── infrastructure/
│   ├── docker-compose.yml
│   ├── nginx_cdn/              # CDN配置
│   ├── nginx_gateway/          # 网关配置
│   ├── config_templates/       # 存放配置文件模板
│   │   ├── server_config.json
│   │   └── client_config.json
│   ├── scripts/                # 存放配置脚本
│   │   ├── install_dependencies.sh
│   │   └── configure_env.sh    # 生成具体的配置
│   └── sql/
│       └── init.sql            # 数据库初始化脚本
├── server/                     # 【后端组】
│   ├── CMakeLists.txt          # 服务端构建配置
│   ├── config/                 # 存放生成的服务端配置
│   │   └── config.json
│   ├── api_service/            # 核心业务代码
│   │   ├── CMakeLists.txt
│   │   └── src/main.cpp
│   ├── video_worker/           # 转码代码
│   │   ├── CMakeLists.txt
│   │   └── src/main.cpp
│   └── common/                 # 公共库
│       ├── CMakeLists.txt
│       ├── src/
│       │   ├── config_manager.cpp
│       │   └── utils.cpp
│       └── include/
│           ├── config_manager.h
│           └── utils.h
└── client/                     # 【客户端组】
    ├── CMakeLists.txt          # 客户端构建配置
    ├── config/
    │   └── config.json
    ├── assets/
    └── src/
        ├── main.cpp
        ├── model/              # C++ Models
        ├── view/               # QML Files
        │   └── Main.qml
        └── components/         # 通用 QML 组件
```

---

## 4. 核心功能实现路径

### 4.1 场景一：用户登录 (Auth Flow)
*   **路径**：客户端 -> PC-2 (API) -> PC-1 (PG主库/Redis)
*   **流程**：
    1.  客户端发 POST 请求给 PC-2。
    2.  PC-2 服务查询 PC-1 的 `users` 表，核对密码。
    3.  若成功，生成 Token（JWT），存入 PC-1 的 Redis（设过期时间）。
    4.  返回 Token 给客户端，客户端存入本地 SQLite。

### 4.2 场景二：视频拍摄与发布 (Upload Pipeline)
分为**红线（数据）**和**蓝线（文件）**。

*   **步骤 1：直传源站**
    *   客户端读取配置，直接将文件 PUT 到 **PC-1 (MinIO)** 的 `temp` 桶。
*   **步骤 2：提交任务**
    *   客户端通知 **PC-2 (API)**：“我传完了”。
    *   PC-2 在 **PC-1 (PG主库)** 写一条状态为“处理中”的记录。
    *   PC-2 往 **PC-1 (Redis)** 的 `video_queue` 队列写一条消息。
*   **步骤 3：异步处理 (PC-3)**
    *   **PC-3 (Worker)** 监听到 Redis 消息。
    *   从 **PC-1 (MinIO)** 下载视频，调用 `ffmpeg` 转码、截图。
    *   将成品传回 **PC-1 (MinIO)** 的 `public` 桶。
    *   回调 PC-2 的接口，修改数据库状态为“已发布”。

### 4.3 场景三：刷视频 (Feed & CDN)
*   **步骤 1：获取列表**
    *   客户端请求 **PC-2**。
    *   PC-2 优先读 **PC-1 (Redis)** 的列表，读不到读 **PC-1 (PG从库)**。
    *   **关键**：返回的视频 URL 必须指向 **PC-3** (模拟 CDN)，如 `http://192.168.1.12/video/abc.mp4`。
*   **步骤 2：MVD 渲染**
    *   客户端 **QML (View)** 检测滑动，**C++ (Model)** 加载数据。
    *   **Delegate** 重复利用，只创建 3 个播放器实例。
*   **步骤 3：CDN 播放**
    *   客户端请求 **PC-3**。
    *   PC-3 Nginx 检查本地缓存 -> 有则直接返回（极速）。
    *   无则去 **PC-1 (MinIO)** 拉取并缓存（回源）。
