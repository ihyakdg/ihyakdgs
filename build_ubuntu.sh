#!/bin/bash
# ========================================================================
# Automatic Ubuntu / Linux Build Script for GTPS (NopySource)
# ========================================================================
set -e

echo "[1/4] Installing required dependencies..."
if command -v apt-get &> /dev/null; then
    sudo apt-get update
    sudo apt-get install -y build-essential cmake g++ libenet-dev libcurl4-openssl-dev libssl-dev libmysqlclient-dev libjsoncpp-dev libpoco-dev
elif command -v yum &> /dev/null; then
    sudo yum install -y gcc-c++ cmake enet-devel libcurl-devel openssl-devel mariadb-devel jsoncpp-devel
fi

echo "[2/4] Preparing build directory..."
mkdir -p build
cd build

echo "[3/4] Running CMake configuration..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "[4/4] Compiling NopySource..."
make -j$(nproc)

echo ""
echo "========================================================================"
echo " SUCCESS: NopySource binary compiled at build/NopySource"
echo " You can run it with: ./build/NopySource"
echo "========================================================================"
