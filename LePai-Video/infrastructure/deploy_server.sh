#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
INSTALL_DIR="/opt/lepai"

# 字体颜色
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}==================================================${NC}"
echo -e "${GREEN}   LePai-Video 服务端一键部署工具${NC}"
echo -e "${GREEN}==================================================${NC}"

# 依赖安装询问
echo ""
read -p "是否需要安装/更新系统依赖? (默认为 Y) [Y/n]: " INSTALL_DEP
INSTALL_DEP=${INSTALL_DEP:-Y}

if [[ "$INSTALL_DEP" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}正在运行依赖安装脚本...${NC}"
    bash "$SCRIPT_DIR/scripts/install_dependencies.sh"
else
    echo "跳过依赖安装。"
fi

# 环境配置检查
if [ -f "$SCRIPT_DIR/scripts/configure_env.sh" ]; then
    echo -e "${YELLOW}正在运行配置向导...${NC}"
    bash "$SCRIPT_DIR/scripts/configure_env.sh"
else
    echo -e "${YELLOW}未找到 configure_env.sh${NC}"
fi

# 编译并安装到 /opt/lepai
# 检查 install_release.sh 是否存在，如果不存在则内联执行编译逻辑
if [ -f "$SCRIPT_DIR/systemd/install_server.sh" ]; then
    echo -e "${YELLOW}正在编译并安装服务端二进制文件...${NC}"
    bash "$SCRIPT_DIR/systemd/install_server.sh"
else
    echo -e "${YELLOW}未找到 install_server.sh，执行内联编译安装...${NC}"
    
    cd "$SCRIPT_DIR/../server" || exit
    mkdir -p build && cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)

    echo "正在安装到 $INSTALL_DIR ..."
    sudo mkdir -p $INSTALL_DIR/bin
    sudo mkdir -p $INSTALL_DIR/config
    sudo mkdir -p $INSTALL_DIR/logs

    # 复制文件
    if [ -f "bin/api_service" ]; then sudo cp bin/api_service $INSTALL_DIR/bin/; fi
    if [ -f "bin/video_worker" ]; then sudo cp bin/video_worker $INSTALL_DIR/bin/; fi
    
    # 复制配置
    if [ -f "$SCRIPT_DIR/../server/config/config.json" ]; then
        sudo cp "$SCRIPT_DIR/../server/config/config.json" $INSTALL_DIR/config/
    fi
    
    sudo chmod +x $INSTALL_DIR/bin/*
    echo "服务端编译安装完成。"
fi

# 生成 Systemd 服务文件
create_service_file() {
    local SERVICE_NAME=$1
    local EXEC_PATH=$2
    local DESC=$3
    local LOG_PREFIX=$4

    echo "正在生成 $SERVICE_NAME 服务文件..."
    sudo bash -c "cat <<EOF > /etc/systemd/system/${SERVICE_NAME}.service
[Unit]
Description=${DESC}
After=network.target docker.service

[Service]
Type=simple
WorkingDirectory=${INSTALL_DIR}/bin
ExecStart=${EXEC_PATH}
Restart=always
RestartSec=5
User=root
StandardOutput=append:${INSTALL_DIR}/logs/${LOG_PREFIX}.log
StandardError=append:${INSTALL_DIR}/logs/${LOG_PREFIX}_error.log

[Install]
WantedBy=multi-user.target
EOF"
    
    sudo systemctl daemon-reload
    sudo systemctl enable ${SERVICE_NAME}
    sudo systemctl restart ${SERVICE_NAME}
    echo -e "${GREEN}服务 ${SERVICE_NAME} 已启动！${NC}"
}

# 角色选择与服务启动
echo ""
echo "请选择当前服务器的角色："
echo "1) PC-1 (基础设施: DB, Redis, MinIO)"
echo "2) PC-2 (API 服务)"
echo "3) PC-3 (转码 Worker + CDN)"
echo "4) 全部启动 (单机部署 All-in-One)"
read -p "请输入选项 [1-4]: " ROLE

case $ROLE in
    1)
        # PC-1
        echo -e "${YELLOW}正在启动基础设施容器...${NC}"
        # 传递 "1" 给 setup_docker.sh
        bash "$SCRIPT_DIR/scripts/setup_docker.sh" <<< "1"
        
        echo "等待数据库和MinIO启动 (10秒)..."
        sleep 10
        bash "$SCRIPT_DIR/scripts/setup_minio_buckets.sh"
        echo -e "${GREEN}PC-1 部署完成！${NC}"
        ;;
        
    2)
        # PC-2
        echo -e "${YELLOW}正在启动 API 网关容器...${NC}"
        bash "$SCRIPT_DIR/scripts/setup_docker.sh" <<< "2"
        
        create_service_file "lepai-api" "${INSTALL_DIR}/bin/api_service" "LePai API Service" "api"
        echo -e "${GREEN}PC-2 部署完成！${NC}"
        ;;
        
    3)
        # PC-3
        echo -e "${YELLOW}正在启动 CDN 容器...${NC}"
        bash "$SCRIPT_DIR/scripts/setup_docker.sh" <<< "3"
        
        create_service_file "lepai-worker" "${INSTALL_DIR}/bin/video_worker" "LePai Video Worker" "worker"
        echo -e "${GREEN}PC-3 部署完成！${NC}"
        ;;
        
    4)
        # All-in-One
        echo -e "${YELLOW}正在启动所有 Docker 容器...${NC}"
        bash "$SCRIPT_DIR/scripts/setup_docker.sh" <<< "4"
        
        echo "等待数据库和MinIO启动 (10秒)..."
        sleep 10
        bash "$SCRIPT_DIR/scripts/setup_minio_buckets.sh"
        
        echo -e "${YELLOW}正在启动所有后台服务...${NC}"
        create_service_file "lepai-api" "${INSTALL_DIR}/bin/api_service" "LePai API Service" "api"
        create_service_file "lepai-worker" "${INSTALL_DIR}/bin/video_worker" "LePai Video Worker" "worker"
        
        echo -e "${GREEN}单机全量部署完成！${NC}"
        ;;
    *)
        echo "无效选项"
        exit 1
        ;;
esac
