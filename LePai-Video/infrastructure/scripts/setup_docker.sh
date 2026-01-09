#!/bin/bash
set -e

echo "=========================================="
echo "   LePai-Video Docker 启动工具"
echo "=========================================="

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# [1/5] 启动 Docker 服务
echo "[1/5] 正在启动 Docker 服务..."
if ! systemctl is-active --quiet docker; then
    sudo systemctl start docker
    sudo systemctl enable docker
    echo "Docker 服务已启动并设置为开机自启。"
else
    echo "Docker 服务正在运行。"
fi

# [2/5] 配置用户权限
echo ""
echo "[2/5] 配置用户权限..."
if groups $USER | grep &>/dev/null '\bdocker\b'; then
    echo "当前用户 $USER 已经在 docker 组中。"
else
    sudo usermod -aG docker $USER
    echo "已将 $USER 加入 docker 用户组。"
    echo "注意：你必须【注销并重新登录】(Log out) 才能使权限生效！"
    echo "或者临时运行 'newgrp docker' 来在当前终端生效。"
fi

# [3/5] 配置国内镜像加速
echo ""
echo "[3/5] 检查 Docker 镜像加速..."
DAEMON_JSON="/etc/docker/daemon.json"
if [ ! -f "$DAEMON_JSON" ]; then
    read -p "是否配置国内镜像加速源? [Y/n] " USE_MIRROR
    USE_MIRROR=${USE_MIRROR:-y} # 如果输入为空，默认为 y

    if [[ "$USE_MIRROR" =~ ^[Yy]$ ]]; then
        echo "正在写入配置到 $DAEMON_JSON ..."
        sudo mkdir -p /etc/docker
        sudo bash -c 'cat <<EOF > /etc/docker/daemon.json
{
  "registry-mirrors": [
    "https://docker.m.daocloud.io",
    "https://huecker.io",
    "https://dockerhub.timeweb.cloud",
    "https://noohub.ru",
    "https://hub-mirror.c.163.com",
    "https://mirror.baidubce.com",
    "https://docker.mirrors.ustc.edu.cn"
  ]
}
EOF'
        sudo systemctl restart docker
        echo "镜像加速配置已完成。"
    else
        echo "跳过镜像加速配置。"
    fi
else
    echo "检测到 $DAEMON_JSON 已存在，跳过配置。"
fi

# [4/5] 注入 PostgreSQL 复制权限脚本
echo ""
echo "[4/5] 正在检查数据库复制权限配置..."
SQL_DIR="$PROJECT_ROOT/infrastructure/sql"
REPL_SCRIPT="$SQL_DIR/00_allow_repl.sh"

# 确保 sql 目录存在
if [ -d "$SQL_DIR" ]; then
    if [ -f "$REPL_SCRIPT" ]; then
        echo "检测到 $REPL_SCRIPT 已存在，跳过生成。"
    else
        echo "正在生成 $REPL_SCRIPT ..."
        cat <<'EOF' > "$REPL_SCRIPT"
#!/bin/bash
set -e
echo "host replication lepai_repl all md5" >> "$PGDATA/pg_hba.conf"
echo "已添加复制权限规则到 pg_hba.conf"
EOF
        chmod +x "$REPL_SCRIPT"
        echo "权限脚本注入完成。"
    fi
else
    echo "警告：未找到 infrastructure/sql 目录，跳过脚本注入。"
fi

# [5/5] 启动容器
echo ""
echo "[5/5] 准备启动容器..."
COMPOSE_FILE="$PROJECT_ROOT/infrastructure/docker-compose.yml"

if [ ! -f "$COMPOSE_FILE" ]; then
    echo "错误：未找到 $COMPOSE_FILE"
    echo "请先运行 configure_env.sh 生成配置。"
    exit 1
fi

DC_CMD=""
if command -v docker-compose &> /dev/null; then
    DC_CMD="docker-compose"
elif docker compose version &> /dev/null; then
    DC_CMD="docker compose"
else
    echo "错误：未找到 docker-compose 或 docker compose 指令"
    exit 1
fi

echo "请选择当前机器的角色："
echo "1) PC-1 (基础设施: DB, Redis, MinIO)"
echo "2) PC-2 (应用网关: API Gateway)"
echo "3) PC-3 (边缘节点: CDN Nginx)"
echo "4) 全部启动"
echo "5) 退出"

read -p "请输入选项 [1-4]: " ROLE_OPT

case $ROLE_OPT in
    1)
        echo "正在启动基础设施 (Profile: infra)..."
        echo "请运行 setup_minio_buckets.sh 脚本来初始化 MinIO 桶!"
        # --profile infra 只启动带有 profiles: ["infra"] 的服务
        sudo $DC_CMD -f "$COMPOSE_FILE" --profile infra up -d
        ;;
    2)
        echo "正在启动 API 网关 (Profile: gateway)..."
        # --profile gateway 只启动带有 profiles: ["gateway"] 的服务
        sudo $DC_CMD -f "$COMPOSE_FILE" --profile gateway up -d
        ;;
    3)
        echo "正在启动边缘节点 (Profile: cdn)..."
        # --profile cdn 只启动带有 profiles: ["cdn"] 的服务
        sudo $DC_CMD -f "$COMPOSE_FILE" --profile cdn up -d
        ;;
    4)
        echo "正在启动所有服务..."
        echo "请运行 setup_minio_buckets.sh 脚本来初始化 MinIO 桶!"
        # --profile all
        sudo $DC_CMD -f "$COMPOSE_FILE" --profile infra --profile gateway --profile cdn up -d
        ;;
    5)
        echo "已退出。"
        exit 0
        ;;
    *)
        echo "无效选项。"
        exit 1
        ;;
esac

echo ""
echo "=========================================="
echo "容器启动命令已执行！"
echo "使用 'docker ps' 查看运行状态。"
echo "重要提示：如果你是第一次运行此脚本，请记得注销并重新登录系统"
echo "=========================================="
