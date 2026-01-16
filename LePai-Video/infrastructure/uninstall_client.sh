#!/bin/bash

# 字体颜色
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

INSTALL_DIR="/opt/lepai-client"
DESKTOP_FILE="/usr/share/applications/lepai-client.desktop"

echo -e "${RED}==================================================${NC}"
echo -e "${RED}   LePai-Video 客户端卸载工具${NC}"
echo -e "${RED}==================================================${NC}"

# 检查是否以 sudo 运行
if [ "$EUID" -ne 0 ]; then
  echo -e "${RED}错误: 请使用 sudo 运行此脚本。${NC}"
  exit 1
fi

echo -e "${RED}警告：此操作将删除客户端程序及本地配置文件！${NC}"
read -p "确认要执行卸载吗? (输入 yes 确认): " CONFIRM

if [ "$CONFIRM" != "yes" ]; then
    echo "操作已取消。"
    exit 0
fi

# 删除桌面快捷方式
echo ""
echo -e "${YELLOW}[1/2] 移除系统菜单入口...${NC}"
if [ -f "$DESKTOP_FILE" ]; then
    rm "$DESKTOP_FILE"
    echo "已删除: $DESKTOP_FILE"
    
    if command -v update-desktop-database &> /dev/null; then
        update-desktop-database
    fi
else
    echo "快捷方式不存在，跳过。"
fi

# 删除安装目录
echo ""
echo -e "${YELLOW}[2/2] 删除程序文件...${NC}"
if [ -d "$INSTALL_DIR" ]; then
    rm -rf "$INSTALL_DIR"
    echo "已删除安装目录: $INSTALL_DIR"
else
    echo "安装目录不存在，跳过。"
fi

echo ""
echo -e "${GREEN}==================================================${NC}"
echo -e "${GREEN}   卸载完成！客户端已从系统中移除。${NC}"
echo -e "${GREEN}==================================================${NC}"
