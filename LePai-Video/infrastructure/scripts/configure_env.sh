#!/bin/bash

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
PROJECT_ROOT="$SCRIPT_DIR/../.."

# 获取本机局域网 IP
get_ip() {
    local ip_addr=""
    
    # ip route
    if [ -z "$ip_addr" ]; then
        ip_addr=$(ip route get 8.8.8.8 2>/dev/null | awk '{for(i=1;i<=NF;i++) if($i=="src") print $(i+1)}')
    fi
    
    # hostname
    if [ -z "$ip_addr" ]; then
        ip_addr=$(hostname -I 2>/dev/null | awk '{print $1}')
    fi
    
    # ip addr
    if [ -z "$ip_addr" ]; then
        ip_addr=$(ip addr show | grep 'inet ' | grep -v '127.0.0.1' | grep -v '172.' | awk '{print $2}' | cut -d/ -f1 | head -n1)
    fi

    echo "$ip_addr"
}

AUTO_IP=$(get_ip)

echo "=========================================="
echo "   乐拍视界 (LePai-Video) 环境配置向导"
echo "=========================================="
echo ""
echo "重要提示："
echo "请务必输入【局域网真实 IP】，不要使用 127.0.0.1。"
echo "本机检测到的 IP 可能是: $AUTO_IP"
echo ""

# PC-1
read -p "请输入 PC-1 (数据库/Redis/MinIO) 的 IP [推荐 $AUTO_IP]: " PC1_IP
PC1_IP=${PC1_IP:-$AUTO_IP}

# PC-2
read -p "请输入 PC-2 (API 服务/网关) 的 IP [推荐 $AUTO_IP]: " PC2_IP
PC2_IP=${PC2_IP:-$AUTO_IP}

# PC-3
read -p "请输入 PC-3 (转码/CDN) 的 IP [推荐 $AUTO_IP]: " PC3_IP
PC3_IP=${PC3_IP:-$AUTO_IP}

echo ""
echo "正在生成配置..."
echo "PC-1: $PC1_IP | PC-2: $PC2_IP | PC-3: $PC3_IP"

generate_config() {
    TEMPLATE_FILE=$1
    OUTPUT_FILE=$2
    
    if [ ! -f "$TEMPLATE_FILE" ]; then
        echo "Error: Template not found: $TEMPLATE_FILE"
        return
    fi

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

# 生成服务端配置
generate_config "$PROJECT_ROOT/infrastructure/config_templates/server_config.json" "$PROJECT_ROOT/server/config/config.json"

# 生成客户端配置
generate_config "$PROJECT_ROOT/infrastructure/config_templates/client_config.json" "$PROJECT_ROOT/client/config/config.json"

# 生成 CDN Nginx 配置 (PC-3)
generate_config "$PROJECT_ROOT/infrastructure/config_templates/nginx_cdn.conf.template" "$PROJECT_ROOT/infrastructure/nginx_cdn/nginx.conf"

# 生成 API Gateway 配置 (PC-2)
# 注意：这里我们使用了 host.docker.internal，但脚本依然会执行替换，只是模板里不再依赖 {{PC2_IP}} 指向后端
generate_config "$PROJECT_ROOT/infrastructure/config_templates/nginx_gateway.conf.template" "$PROJECT_ROOT/infrastructure/nginx_gateway/nginx.conf"

# 生成 Docker Compose 文件
generate_config "$PROJECT_ROOT/infrastructure/config_templates/docker-compose.yml.template" "$PROJECT_ROOT/infrastructure/docker-compose.yml"

echo ""
echo "=========================================="
echo "配置完成！"
echo "下一步：请运行 ./setup_docker.sh "
echo "=========================================="