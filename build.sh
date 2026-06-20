#!/bin/bash
set -eo pipefail

# ====================== 全局配置 ======================
# 云构建产物输出目录（CNB 自动收集此目录文件作为发布制品）
OUTPUT_DIR="/output"
# IXWebSocket 安装目录标识文件，用于判断是否已编译安装
IXWS_INSTALL_MARK="/usr/local/include/ixwebsocket/IXWebSocket.h"
# 项目路径
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_PREFIX="/usr/local"
BINARY_TARGET_NAME="dicksuck"
BINARY_FULL_PATH="$BUILD_DIR/bin/$BINARY_TARGET_NAME"

echo "============================================="
echo "🔨 qbot-chen CNB Cloud Build Script (Optimized)"
echo "============================================="

# ====================== 1. 替换国内Debian源 ======================
echo -e "\n[1/6] 替换国内阿里云软件源加速"
if [ -f /etc/apt/sources.list ]; then
    sed -i 's/deb.debian.org/mirrors.aliyun.com/g' /etc/apt/sources.list
    sed -i 's/security.debian.org/mirrors.aliyun.com/g' /etc/apt/sources.list
fi

# ====================== 2. 安装系统依赖 ======================
echo -e "\n[2/6] 校验并安装系统原生依赖"
REQUIRED_APT_PACKAGES=(
    "build-essential"
    "cmake"
    "libcurl4-openssl-dev"
    "nlohmann-json3-dev"
    "libsqlite3-dev"
    "git"
)
NEED_APT_INSTALL=0

# 遍历检测缺少的系统包
for pkg in "${REQUIRED_APT_PACKAGES[@]}"; do
    if ! dpkg -l | grep -q "^ii  $pkg"; then
        echo "❌ 缺失系统依赖: $pkg"
        NEED_APT_INSTALL=1
    else
        echo "✓ 已存在: $pkg"
    fi
done

# 仅当存在缺失包时执行安装，缓存环境后无需重复下载
if [ "$NEED_APT_INSTALL" -eq 1 ]; then
    echo "开始批量安装缺失依赖..."
    apt update -y
    apt install -y "${REQUIRED_APT_PACKAGES[@]}"
    # 清理apt缓存，减小构建镜像体积
    rm -rf /var/lib/apt/lists/*
else
    echo "所有系统依赖已就绪，跳过apt安装"
fi

# ====================== 3. 编译安装 IXWebSocket ======================
echo -e "\n[3/6] 校验 IXWebSocket 库"
if [ -f "$IXWS_INSTALL_MARK" ]; then
    echo "✓ IXWebSocket 已预编译安装，跳过源码构建"
else
    echo "IXWebSocket 未安装，源码编译中..."
    # 使用ghproxy镜像加速github克隆，解决国内超时
    git clone https://mirror.ghproxy.com/https://github.com/machinezone/IXWebSocket.git /tmp/ixws
    cd /tmp/ixws
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
    cmake --build build -j$(nproc)
    cmake --install build
    cd "$SCRIPT_DIR"
    rm -rf /tmp/ixws # 删除源码包释放空间
    echo "✓ IXWebSocket 编译安装完成"
fi

# ====================== 4. 项目CMake编译 ======================
echo -e "\n[4/6] 初始化项目编译目录"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[CMake] 执行配置..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wno-unused-parameter -O2"

echo "[Make] 多线程编译项目..."
make -j$(nproc)

# 校验二进制是否生成成功
if [ ! -f "$BINARY_FULL_PATH" ]; then
    echo "ERROR: 编译失败，未生成目标二进制 $BINARY_FULL_PATH"
    exit 1
fi

# ====================== 5. CNB 云构建产物发布 ======================
echo -e "\n[5/6] 导出发布制品到CNB标准输出目录 $OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# 1. 复制主程序
cp "$BINARY_FULL_PATH" "$OUTPUT_DIR/"
# 2. 复制启动脚本、配置文件（按需自行增减）
cp "$SCRIPT_DIR"/*.sh "$OUTPUT_DIR/" 2>/dev/null || true
cp "$SCRIPT_DIR"/config.json "$OUTPUT_DIR/" 2>/dev/null || true

# 输出制品清单，方便流水线查看
echo "📦 已导出发布文件列表:"
ls -lh "$OUTPUT_DIR"

# ====================== 6. 构建完成提示 ======================
echo -e "\n============================================="
echo "✅ 云构建全部完成！"
echo "📍 本地编译产物: $BINARY_FULL_PATH"
echo "📤 CNB发布制品目录: $OUTPUT_DIR"
echo "🚀 运行命令示例:"
echo "   ./$OUTPUT_DIR/$BINARY_TARGET_NAME"
echo "============================================="