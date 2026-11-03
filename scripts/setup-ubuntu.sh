#!/usr/bin/env bash
# setup-ubuntu.sh —— 在 WSL 的 Ubuntu 里安装 C++ 开发工具链
# 用法（在 Ubuntu 终端里）：
#   bash "/mnt/c/Users/dym54/Desktop/c++项目/scripts/setup-ubuntu.sh"
set -euo pipefail

echo "==> apt update"
sudo apt update

echo "==> 安装编译工具链 / CMake / Ninja / clangd / 调试器"
sudo apt install -y \
  build-essential cmake ninja-build \
  clang clangd lldb gdb \
  git curl zip unzip pkg-config

echo
echo "==> 版本检查"
g++ --version | head -n 1
clang++ --version | head -n 1
cmake --version | head -n 1
ninja --version
clangd --version | head -n 1
gdb --version | head -n 1

echo
echo "==> 完成。下一步：回到 Windows 的 VS Code，安装 WSL 扩展后打开项目文件夹。"
