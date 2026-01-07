#!/bin/bash

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

install_ubuntu() {
    echo "正在使用 APT 安装依赖 (需要 sudo 权限)..."
    sudo apt update
    
    # 基础编译工具
    sudo apt install -y build-essential cmake git gdb
    
    # Qt6 全家桶
    sudo apt install -y qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-l10n-tools
    sudo apt install -y qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtmultimedia libqt6networkauth6-dev
    
    # 数据库驱动 (libpqxx)
    sudo apt install -y libpqxx-dev
    
    # 媒体处理 (FFmpeg + OpenCV)
    sudo apt install -y libopencv-dev libavcodec-dev libavformat-dev libavfilter-dev libavdevice-dev libswscale-dev
    
    # SSL
    sudo apt install -y libssl-dev

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
    sudo pacman -S --noconfirm libpqxx
    
    # 媒体处理
    sudo pacman -S --noconfirm opencv ffmpeg
    
    # SSL
    sudo pacman -S --noconfirm openssl

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
        echo "❌ 不支持的发行版: $ID"
        echo "请参考 infrastructure/README.md 手动安装：Qt6, libpqxx, OpenCV, FFmpeg"
        exit 1
        ;;
esac

echo ""
echo "环境准备就绪。下一步："
echo "1. 运行配置脚本: configure_env.sh"
echo "2. 开始编译项目"
