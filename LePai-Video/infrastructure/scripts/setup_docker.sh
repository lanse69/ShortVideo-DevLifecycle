#!/bin/bash
set -e

echo "=========================================="
echo "   LePai-Video Docker 环境配置工具"
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
    "https://mirror.baidubce.com"
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

# [5/5] 启动基础设施
echo ""
echo "[5/5] 准备启动项目基础设施 (Postgres, Redis, MinIO)..."
COMPOSE_FILE="$PROJECT_ROOT/infrastructure/docker-compose.yml"

if [ -f "$COMPOSE_FILE" ]; then
    echo "找到配置文件: $COMPOSE_FILE"
    read -p "是否立即启动基础设施容器? [Y/n] " START_NOW
    START_NOW=${START_NOW:-y}

    if [[ "$START_NOW" =~ ^[Yy]$ ]]; then
        echo "正在启动容器..."
        if command -v docker-compose &> /dev/null; then
             sudo docker-compose -f "$COMPOSE_FILE" up -d
        else
             sudo docker compose -f "$COMPOSE_FILE" up -d
        fi
        
        echo ""
        echo "基础设施已在后台运行！"
        echo "你可以使用 'docker ps' 查看状态。"
    else
        echo "已跳过启动。"
    fi
else
    echo "未找到 docker-compose.yml，请检查项目结构。"
fi

echo ""
echo "=========================================="
echo "配置完成！"
echo "重要提示：如果你是第一次运行此脚本，请记得注销并重新登录系统"
echo "=========================================="
