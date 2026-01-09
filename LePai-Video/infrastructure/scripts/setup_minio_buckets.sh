#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."

echo "只需在基础设施的机器上执行本脚本"
echo "正在配置 MinIO 存储桶策略..."

sudo docker run --rm --entrypoint /bin/sh minio/mc -c "
# 设置别名 (连接到宿主机的 9000 端口)
mc alias set myminio http://host.docker.internal:9000 lepai_minio lepai_minio_pass;

# 创建 buckets (如果不存在)
mc mb myminio/public --ignore-existing;
mc mb myminio/temp --ignore-existing;

# 设置 public 桶为公开下载 (Download)
mc anonymous set download myminio/public;

# 设置 temp 桶为私有
mc anonymous set none myminio/temp;

echo 'MinIO 策略配置完成！'
" --add-host=host.docker.internal:host-gateway

echo "脚本执行完毕。"