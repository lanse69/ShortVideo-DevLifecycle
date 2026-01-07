#!/bin/bash
set -e

# 向 pg_hba.conf 追加允许复制的规则
# host: TCP/IP 连接
# replication: 数据库名为复制协议
# lepai_repl: 用户名
# all: 允许所有 IP (0.0.0.0/0)
# md5: 使用密码验证
echo "host replication lepai_repl all md5" >> "$PGDATA/pg_hba.conf"

echo "已添加复制权限规则到 pg_hba.conf"
