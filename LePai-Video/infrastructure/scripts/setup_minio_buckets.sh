#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."
ASSETS_DIR="$PROJECT_ROOT/assets_source"

echo "=================================================="
echo "   MinIO 存储桶及默认资源初始化"
echo "=================================================="

# 检查资源文件是否存在
if [ ! -d "$ASSETS_DIR" ]; then
    echo "错误: 未找到资源目录 $ASSETS_DIR"
    echo "请在项目根目录创建 assets_source 文件夹并放入默认资源"
    exit 1
fi

if [ ! -f "$ASSETS_DIR/default.png" ] || [ ! -f "$ASSETS_DIR/failed.jpeg" ]; then
    echo "错误: 资源目录中缺少 default.png 或 failed.jpeg"
    exit 1
fi

# 自动检测 Docker 网络名称
NETWORK_NAME=$(docker network ls --filter name=lepai_net --format "{{.Name}}" | head -n 1)
if [ -z "$NETWORK_NAME" ]; then
    NETWORK_NAME="lepai_net" # 降级默认值
fi
echo "已检测到 Docker 网络: $NETWORK_NAME"

# 获取 MinIO 容器的真实 IP 地址
MINIO_IP=$(docker inspect -f "{{.NetworkSettings.Networks.$NETWORK_NAME.IPAddress}}" lepai_minio 2>/dev/null)

if [ -z "$MINIO_IP" ]; then
    echo "警告: 无法获取 lepai_minio 的 IP，尝试使用主机名（可能会报错）..."
    MINIO_HOST="lepai_minio"
else
    echo "已获取 MinIO 内部 IP: $MINIO_IP"
    MINIO_HOST="$MINIO_IP"
fi

echo "正在执行 MinIO 配置..."

# 使用 docker 运行 mc 客户端
sudo docker run --rm \
    -v "$ASSETS_DIR":/assets \
    --network "$NETWORK_NAME" \
    -e TARGET_HOST="$MINIO_HOST" \
    --entrypoint /bin/sh \
    minio/mc -c "

echo '[1/4] 连接 MinIO 服务...'
# 使用 IP 地址连接
mc alias set myminio http://\${TARGET_HOST}:9000 lepai_minio lepai_minio_pass;

echo '[2/4] 创建存储桶...'
mc mb myminio/public --ignore-existing;
mc mb myminio/temp --ignore-existing;

echo '[3/4] 设置访问策略...'
mc anonymous set download myminio/public;
mc anonymous set upload myminio/temp
mc anonymous set download myminio/temp

echo '[4/4] 上传默认资源...'
mc cp /assets/default.png myminio/public/defaults/default.png;
mc cp /assets/failed.jpeg myminio/public/defaults/failed.jpeg;

echo 'MinIO 初始化全部完成！'

" 

echo "脚本执行完毕。"