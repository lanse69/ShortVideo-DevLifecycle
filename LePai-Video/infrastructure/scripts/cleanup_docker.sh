#!/bin/bash

# 定义颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 获取脚本所在目录的绝对路径，定位项目根目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."
INFRA_DIR="$PROJECT_ROOT/infrastructure"

echo -e "${RED}============================================================${NC}"
echo -e "${RED}   警告：这将执行清理！${NC}"
echo -e "${RED}   所有 数据库数据(PostgreSQL)、Redis缓存、MinIO文件 将被永久删除。${NC}"
echo -e "${RED}============================================================${NC}"
read -p "确认要继续吗? (输入 yes 确认): " CONFIRM

if [ "$CONFIRM" != "yes" ]; then
    echo "操作已取消。"
    exit 0
fi

echo ""
echo -e "${YELLOW}[1/4] 正在停止并移除容器...${NC}"

# 检测 docker-compose 命令
DC_CMD=""
if command -v docker-compose &> /dev/null; then
    DC_CMD="docker-compose"
elif docker compose version &> /dev/null; then
    DC_CMD="docker compose"
else
    echo -e "${RED}错误：未找到 docker-compose 或 docker compose 指令${NC}"
    exit 1
fi

# 使用 down -v 同时删除容器和挂载的卷(Volumes)
# --remove-orphans 清除未定义但关联的孤儿容器
cd "$INFRA_DIR" || exit
sudo $DC_CMD down -v --remove-orphans

echo ""
echo -e "${YELLOW}[2/4] 强制清理残留容器 (以防万一)...${NC}"
# 显式删除指定名称的容器，防止 docker-compose 没删干净
sudo docker rm -f lepai_pg lepai_pg_slave lepai_redis lepai_minio lepai_gateway lepai_cdn 2>/dev/null || true

echo ""
echo -e "${YELLOW}[3/4] 强制删除数据卷...${NC}"
# 显式删除卷
sudo docker volume rm infrastructure_pg_master_data infrastructure_pg_slave_data infrastructure_redis_data infrastructure_minio_data 2>/dev/null || true
sudo docker volume prune -f

echo ""
echo -e "${YELLOW}[4/4] 清理本地 Nginx 缓存文件...${NC}"
if [ -d "$INFRA_DIR/nginx_cache" ]; then
    sudo rm -rf "$INFRA_DIR/nginx_cache/*"
    echo "本地 Nginx 缓存目录已清空。"
else
    echo "未发现 Nginx 缓存目录，跳过。"
fi

echo ""
echo -e "${GREEN}============================================================${NC}"
echo -e "${GREEN}   清理完成！环境已重置。${NC}"
echo -e "${GREEN}   现在的状态相当于第一次运行。${NC}"
echo -e "${GREEN}   请运行 ./setup_docker.sh 重新启动服务。${NC}"
echo -e "${GREEN}============================================================${NC}"