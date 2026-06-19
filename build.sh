#!/bin/bash

# dicksuck Linux 构建脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_PREFIX="/usr/local"

echo "🔨 dicksuck Linux Build Script"
echo "================================"

# 检查依赖
echo "📦 Checking dependencies..."
required_packages=(
    "libcurl4-openssl-dev"
    "nlohmann-json3-dev"
    "libsqlite3-dev"
    "libixwebsocket-dev"
)

for pkg in "${required_packages[@]}"; do
    if ! dpkg -l | grep -q "^ii  $pkg"; then
        echo "❌ Missing: $pkg"
        echo "   Run: sudo apt install $pkg"
    else
        echo "✓ $pkg"
    fi
done

# 创建构建目录
echo ""
echo "📂 Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake 配置
echo "⚙️  Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wno-unused-parameter -O2"

# 编译
echo ""
echo "🔨 Building..."
make -j$(nproc)

# 输出信息
echo ""
echo "✅ Build successful!"
echo ""
echo "📍 Output: $BUILD_DIR/bin/dicksuck"
echo ""
echo "🚀 To run:"
echo "   cd $SCRIPT_DIR"
echo "   ./build/bin/dicksuck"
echo ""
echo "📥 To install (optional):"
echo "   sudo make -C $BUILD_DIR install"
