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

echo "正在执行 MinIO 配置..."

# 使用 docker 运行 mc 客户端
# -v 挂载本地资源目录到容器内的 /assets
# --network lepai_net 确保能连接到 MinIO 容器
sudo docker run --rm \
    -v "$ASSETS_DIR":/assets \
    --network lepai_net \
    --entrypoint /bin/sh \
    minio/mc -c "

echo '[1/4] 连接 MinIO 服务...'
mc alias set myminio http://lepai_minio:9000 lepai_minio lepai_minio_pass;

echo '[2/4] 创建存储桶...'
mc mb myminio/public --ignore-existing;
mc mb myminio/temp --ignore-existing;

echo '[3/4] 设置访问策略...'
# public 桶允许匿名下载
mc anonymous set download myminio/public;
# temp 桶私有
mc anonymous set none myminio/temp;

echo '[4/4] 上传默认资源...'
# 上传 default.png
mc cp /assets/default.png myminio/public/defaults/default.png;
# 上传 failed.jpeg
mc cp /assets/failed.jpeg myminio/public/defaults/failed.jpeg;

echo 'MinIO 初始化全部完成！'

" 

echo "脚本执行完毕。"