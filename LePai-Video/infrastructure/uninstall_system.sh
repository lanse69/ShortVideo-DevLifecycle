#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
INSTALL_DIR="/opt/lepai"

# 字体颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${RED}==================================================${NC}"
echo -e "${RED}   LePai-Video 服务端系统卸载与清理工具${NC}"
echo -e "${RED}==================================================${NC}"
echo -e "${RED}警告：此操作将删除所有数据（数据库、文件、日志）！${NC}"
read -p "确认要执行卸载吗? (输入 yes 确认): " CONFIRM

if [ "$CONFIRM" != "yes" ]; then
    echo "操作已取消。"
    exit 0
fi

# 停止并移除 Systemd 服务
echo ""
echo -e "${YELLOW}[1/3] 停止并移除后台服务...${NC}"

SERVICES=("lepai-api" "lepai-worker")

for SVC in "${SERVICES[@]}"; do
    if systemctl is-active --quiet $SVC || systemctl is-enabled --quiet $SVC; then
        echo "正在停止 $SVC ..."
        sudo systemctl stop $SVC
        sudo systemctl disable $SVC
        
        if [ -f "/etc/systemd/system/${SVC}.service" ]; then
            echo "删除服务文件 /etc/systemd/system/${SVC}.service"
            sudo rm "/etc/systemd/system/${SVC}.service"
        fi
    else
        echo "$SVC 未安装或未运行，跳过。"
    fi
done

sudo systemctl daemon-reload
echo "Systemd 服务清理完成。"

# 清理 Docker 容器与卷
echo ""
echo -e "${YELLOW}[2/3] 清理 Docker 环境...${NC}"

if [ -f "$SCRIPT_DIR/scripts/cleanup_docker.sh" ]; then
    bash "$SCRIPT_DIR/scripts/cleanup_docker.sh" <<< "yes"
else
    echo -e "${RED}错误：找不到 cleanup_docker.sh 脚本，尝试手动清理...${NC}"
    # 手动清理
    sudo docker rm -f lepai_pg lepai_pg_slave lepai_redis lepai_minio lepai_gateway lepai_cdn 2>/dev/null
    sudo docker volume rm infrastructure_pg_master_data infrastructure_pg_slave_data infrastructure_redis_data infrastructure_minio_data 2>/dev/null
fi

# 删除安装目录 (bin, config, logs)
echo ""
echo -e "${YELLOW}[3/3] 删除安装文件...${NC}"

if [ -d "$INSTALL_DIR" ]; then
    echo "正在删除 $INSTALL_DIR ..."
    sudo rm -rf "$INSTALL_DIR"
    echo "安装目录已删除。"
else
    echo "安装目录 $INSTALL_DIR 不存在，跳过。"
fi

echo ""
echo -e "${GREEN}==================================================${NC}"
echo -e "${GREEN}   卸载完成！系统已恢复干净状态。${NC}"
echo -e "${GREEN}==================================================${NC}"
