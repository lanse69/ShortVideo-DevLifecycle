#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."

echo "=========================================="
echo "   乐拍视界 (LePai-Video) 环境配置向导"
echo "=========================================="
echo ""

read -p "请输入 PC-1 (数据库/Redis/MinIO) 的 IP [默认 127.0.0.1]: " PC1_IP
PC1_IP=${PC1_IP:-127.0.0.1}

read -p "请输入 PC-2 (API 服务) 的 IP [默认 127.0.0.1]: " PC2_IP
PC2_IP=${PC2_IP:-127.0.0.1}

read -p "请输入 PC-3 (转码/CDN) 的 IP [默认 127.0.0.1]: " PC3_IP
PC3_IP=${PC3_IP:-127.0.0.1}

echo ""
echo "正在生成配置..."
echo "PC-1: $PC1_IP | PC-2: $PC2_IP | PC-3: $PC3_IP"

generate_config() {
    TEMPLATE_FILE=$1
    OUTPUT_FILE=$2
    
    # 读取模板
    CONTENT=$(cat "$TEMPLATE_FILE")
    
    # 替换变量
    CONTENT=${CONTENT//\{\{PC1_IP\}\}/$PC1_IP}
    CONTENT=${CONTENT//\{\{PC2_IP\}\}/$PC2_IP}
    CONTENT=${CONTENT//\{\{PC3_IP\}\}/$PC3_IP}

    # 确保输出目录存在
    mkdir -p "$(dirname "$OUTPUT_FILE")"
    
    # 写入文件
    echo "$CONTENT" > "$OUTPUT_FILE"
    echo "已生成: $OUTPUT_FILE"
}

# 生成 Server 端配置
generate_config "$PROJECT_ROOT/infrastructure/config_templates/server_config.json" "$PROJECT_ROOT/server/config/config.json"

# 生成 Client 端配置
generate_config "$PROJECT_ROOT/infrastructure/config_templates/client_config.json" "$PROJECT_ROOT/client/config/config.json"

echo ""
echo "=========================================="
echo "配置完成！"
echo "请重新编译项目。"
echo "=========================================="
