#!/bin/bash

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
CLIENT_SRC_DIR="$SCRIPT_DIR/../client"
INSTALL_DIR="/opt/lepai-client"
DESKTOP_FILE="/usr/share/applications/lepai-client.desktop"

# 字体颜色
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}==================================================${NC}"
echo -e "${GREEN}   LePai-Video 客户端一键部署工具${NC}"
echo -e "${GREEN}==================================================${NC}"

# 检查是否以 sudo 运行
if [ "$EUID" -ne 0 ]; then
  echo -e "${RED}错误: 请使用 sudo 运行此脚本 (需要写入 /opt 和系统菜单)${NC}"
  exit 1
fi

# 依赖安装询问
echo ""
read -p "是否需要安装系统编译依赖(Qt6/CMake等)? (默认为 N) [y/N]: " INSTALL_DEP
INSTALL_DEP=${INSTALL_DEP:-N}

if [[ "$INSTALL_DEP" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}正在运行依赖安装脚本...${NC}"
    if [ -f "$SCRIPT_DIR/scripts/install_dependencies.sh" ]; then
        bash "$SCRIPT_DIR/scripts/install_dependencies.sh"
    else
        echo -e "${RED}未找到依赖安装脚本，跳过。${NC}"
    fi
fi

# 环境配置
echo ""
echo -e "${YELLOW}[1/4] 启动环境配置...${NC}"
if [ -f "$SCRIPT_DIR/scripts/configure_env.sh" ]; then
    bash "$SCRIPT_DIR/scripts/configure_env.sh"
else
    echo -e "${RED}错误: 找不到配置脚本 scripts/configure_env.sh${NC}"
    echo "请手动创建配置文件或检查项目结构。"
    exit 1
fi

# 编译客户端
echo ""
echo -e "${YELLOW}[2/4] 正在编译客户端...${NC}"
if [ ! -d "$CLIENT_SRC_DIR" ]; then
    echo -e "${RED}错误: 找不到 client 源码目录 ($CLIENT_SRC_DIR)${NC}"
    exit 1
fi

BUILD_DIR="$CLIENT_SRC_DIR/build"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit

cmake -DCMAKE_BUILD_TYPE=Release ..
if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 配置失败！请检查环境。${NC}"
    exit 1
fi

echo "开始编译..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败！${NC}"
    exit 1
fi

# 安装文件
echo ""
echo -e "${YELLOW}[3/4] 安装文件到 $INSTALL_DIR ...${NC}"

# 创建目录
mkdir -p "$INSTALL_DIR/bin"

# 复制可执行文件
if [ -f "$BUILD_DIR/appLePaiClient" ]; then
    cp "$BUILD_DIR/appLePaiClient" "$INSTALL_DIR/bin/LePaiClient"
else
    echo -e "${RED}错误: 编译产物 appLePaiClient 未找到${NC}"
    exit 1
fi

# 复制配置文件
if [ -f "$BUILD_DIR/config.json" ]; then
    cp "$BUILD_DIR/config.json" "$INSTALL_DIR/bin/"
elif [ -f "$CLIENT_SRC_DIR/config/config.json" ]; then
    cp "$CLIENT_SRC_DIR/config/config.json" "$INSTALL_DIR/bin/"
fi

# 处理图标
if [ -f "$SCRIPT_DIR/../assets_source/leipai_logo.jpg" ]; then
    cp "$SCRIPT_DIR/../assets_source/leipai_logo.jpg" "$INSTALL_DIR/icon.jpg"
else
    echo "下载默认图标..."
    wget -q -O "$INSTALL_DIR/icon.jpg" https://cdn-icons-png.flaticon.com/512/3074/3074767.png
fi

# 设置权限
chmod +x "$INSTALL_DIR/bin/LePaiClient"
chmod 644 "$INSTALL_DIR/bin/config.json" 2>/dev/null

# 创建桌面快捷方式
echo ""
echo -e "${YELLOW}[4/4] 创建系统菜单快捷方式...${NC}"

cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Name=lepai
Name[zh_CN]=乐拍视界
Comment=短视频社交平台客户端
Exec=$INSTALL_DIR/bin/LePaiClient
Icon=$INSTALL_DIR/icon.jpg
Terminal=false
Type=Application
Categories=AudioVideo;Video;Qt;
Keywords=Video;Social;
StartupNotify=true
EOF

chmod 644 "$DESKTOP_FILE"
# 刷新桌面数据库
if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$DESKTOP_FILE"
fi

echo ""
echo -e "${GREEN}==================================================${NC}"
echo -e "${GREEN}   客户端部署成功！${NC}"
echo -e "${GREEN}==================================================${NC}"
echo "1. 你可以在应用菜单中搜索 '乐拍' 或 'LePai' 启动程序。"
echo "2. 安装位置: $INSTALL_DIR"
echo "3. 配置文件: $INSTALL_DIR/bin/config.json"
