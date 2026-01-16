#!/bin/bash
set -e

echo "正在编译服务端项目..."
cd ../../server
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
