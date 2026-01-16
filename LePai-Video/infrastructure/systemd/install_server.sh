#!/bin/bash
set -e

# 获取脚本文件所在的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
SERVER_DIR="$SCRIPT_DIR/../../server"

echo "正在编译服务端项目..."
echo "源码路径: $SERVER_DIR"

if [ ! -d "$SERVER_DIR" ]; then
    echo "错误：找不到 server 目录: $SERVER_DIR"
    exit 1
fi

cd "$SERVER_DIR"
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

INSTALL_DIR="/opt/lepai"
echo "正在安装到 $INSTALL_DIR ..."

sudo mkdir -p $INSTALL_DIR/bin
sudo mkdir -p $INSTALL_DIR/config
sudo mkdir -p $INSTALL_DIR/logs

sudo cp bin/api_service $INSTALL_DIR/bin/
sudo cp bin/video_worker $INSTALL_DIR/bin/

if [ -f "../config/config.json" ]; then
    sudo cp ../config/config.json $INSTALL_DIR/config/
else
    echo "警告: 未找到 config.json，请确保先运行了 configure_env.sh"
fi

sudo chmod +x $INSTALL_DIR/bin/*

echo "安装完成！"
