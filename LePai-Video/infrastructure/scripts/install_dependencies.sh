#!/bin/bash

set -e

echo "=========================================="
echo "   LePai-Video 开发环境自动化安装向导"
echo "=========================================="

# 检测操作系统
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    echo "检测到操作系统: $OS"
else
    echo "无法检测操作系统，请手动安装依赖。"
    exit 1
fi

install_drogon_source() {
    echo "------------------------------------------"
    echo "开始编译安装 Drogon..."
    echo "------------------------------------------"

    # 创建临时构建目录
    BUILD_DIR="/tmp/drogon_build"
    rm -rf $BUILD_DIR
    mkdir -p $BUILD_DIR
    cd $BUILD_DIR

    # 克隆 Drogon
    echo "正在克隆 Drogon 仓库..."
    git clone https://github.com/drogonframework/drogon
    cd drogon
    git checkout v1.9.7
    git submodule update --init

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    sudo make install

    echo "Drogon 安装完成！清理临时文件..."
    rm -rf $BUILD_DIR
}

install_ubuntu() {
    echo "正在使用 APT 安装依赖 (需要 sudo 权限)..."
    sudo apt update
    
    # 基础编译工具
    sudo apt install -y build-essential cmake git gdb
    
    # Drogon 依赖 (Jsoncpp, UUID, Zlib)
    sudo apt install -y libjsoncpp-dev uuid-dev zlib1g-dev
    
    # Qt6
    sudo apt install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-l10n-tools
    sudo apt install -y qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtmultimedia libqt6networkauth6-dev libqt6sql6-psql
    
    # 数据库驱动 (libpqxx)
    sudo apt install -y libpqxx-dev libpq-dev postgresql-server-dev-all postgresql-client
    
    # 媒体处理 (FFmpeg + OpenCV)
    sudo apt install -y libopencv-dev libavcodec-dev libavformat-dev libavfilter-dev libavdevice-dev libswscale-dev
    
    # SSL
    sudo apt install -y libssl-dev
    
    # Docker & Docker Compose
    sudo apt install -y docker.io docker-compose
    
    install_drogon_source

    echo "依赖安装完成！"
}

install_manjaro() {
    echo "正在使用 Pacman 安装依赖 (需要 sudo 权限)..."
    
    # 同步数据库
    sudo pacman -Syu --noconfirm
    
    # 基础工具
    sudo pacman -S --noconfirm base-devel cmake git gdb
    
    # Qt6
    sudo pacman -S --noconfirm qt6-base qt6-declarative qt6-tools qt6-multimedia qt6-svg
    
    # 数据库驱动
    sudo pacman -S --noconfirm libpqxx postgresql-libs
    
    # 媒体处理
    sudo pacman -S --noconfirm opencv ffmpeg
    
    # SSL
    sudo pacman -S --noconfirm openssl

    # Docker & Docker Compose
    sudo pacman -S --noconfirm docker docker-compose
    
    sudo pacman -S --noconfirm jsoncpp libutil-linux zlib
    
    install_drogon_source

    echo "依赖安装完成！"
}

# 执行安装分支
case $ID in
    ubuntu|debian)
        install_ubuntu
        ;;
    manjaro|arch)
        install_manjaro
        ;;
    *)
        echo "不支持的发行版: $ID"
        exit 1
        ;;
esac

echo ""
echo "环境准备就绪。下一步："
echo "1. 运行配置脚本: configure_env.sh"
echo "2. 运行 setup_docker.sh 配置 Docker 环境"
echo "3. 开始编译项目"
