#!/bin/bash
set -e

echo "=========================================="
echo "   LePai-Video Docker 环境配置工具"
echo "=========================================="

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# 启动 Docker 服务
echo "[1/4] 正在启动 Docker 服务..."
sudo systemctl start docker
sudo systemctl enable docker
echo "Docker 服务已启动并设置为开机自启。"

# 配置当前用户免 sudo 使用 Docker
echo ""
echo "[2/4] 配置用户权限..."
if groups $USER | grep &>/dev/null '\bdocker\b'; then
    echo "当前用户 $USER 已经在 docker 组中。"
else
    sudo usermod -aG docker $USER
    echo "已将 $USER 加入 docker 用户组。"
    echo "注意：你必须【注销并重新登录】(Log out) 才能使权限生效！"
    echo "或者临时运行 'newgrp docker' 来在当前终端生效。"
fi

# 配置国内镜像加速
echo ""
echo "[3/4] 检查 Docker 镜像加速..."
DAEMON_JSON="/etc/docker/daemon.json"
if [ ! -f "$DAEMON_JSON" ]; then
    read -p "是否配置国内镜像加速源? [y/N] " USE_MIRROR
    if [[ "$USE_MIRROR" =~ ^[Yy]$ ]]; then
        echo "正在写入配置到 $DAEMON_JSON ..."
        sudo mkdir -p /etc/docker
        sudo bash -c 'cat <<EOF > /etc/docker/daemon.json
		{
		  "registry-mirrors": [
		    "https://docker.m.daocloud.io",
		    "https://huecker.io",
		    "https://dockerhub.timeweb.cloud",
		    "https://noohub.ru"
		    "https://hub-mirror.c.163.com",
		    "https://mirror.baidubce.com"
		  ]
		}
		EOF'
        sudo systemctl restart docker
        echo "镜像加速配置已完成。"
    fi
else
    echo "检测到 $DAEMON_JSON 已存在，跳过配置。"
fi

# 尝试启动项目基础设施
echo ""
echo "[4/4] 准备启动项目基础设施 (Postgres, Redis, MinIO)..."
COMPOSE_FILE="$PROJECT_ROOT/infrastructure/docker-compose.yml"

if [ -f "$COMPOSE_FILE" ]; then
    echo "找到配置文件: $COMPOSE_FILE"
    read -p "是否立即启动基础设施容器? [y/N] " START_NOW
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
    fi
else
    echo "未找到 docker-compose.yml，请检查项目结构。"
fi

echo ""
echo "=========================================="
echo "配置完成！"
echo "重要提示：如果你是第一次运行此脚本，请记得注销并重新登录系统"
echo "=========================================="
