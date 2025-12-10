#!/bin/bash
set -e

echo "Installing build dependencies for s3al OS simulator..."

# Detect the OS
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
else
    echo "Cannot detect OS. /etc/os-release not found."
    exit 1
fi

case "$OS" in
    ubuntu|debian)
        echo "Detected Ubuntu/Debian-based system"
        sudo apt-get update
        sudo apt-get install -y --no-install-recommends \
            build-essential \
            cmake \
            ninja-build \
            g++ \
            libncurses-dev \
            lua5.4 \
            liblua5.4-dev
        ;;
    fedora|rhel|centos)
        echo "Detected Fedora/RHEL/CentOS-based system"
        sudo dnf install -y \
            gcc \
            gcc-c++ \
            cmake \
            ninja-build \
            make \
            ncurses-devel \
            lua \
            lua-devel
        ;;
    arch|manjaro)
        echo "Detected Arch-based system"
        sudo pacman -Sy --noconfirm \
            base-devel \
            cmake \
            ninja \
            ncurses \
            lua
        ;;
    *)
        echo "Unsupported OS: $OS"
        echo "Please manually install: build-essential/gcc, cmake, ninja-build, ncurses-dev"
        exit 1
        ;;
esac

echo ""
echo "Build dependencies installed successfully!"
echo ""

# Ask if user wants to build the project now
read -p "Would you like to build the project now? (y/n) " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Building s3al OS simulator..."
    cmake -B build -G Ninja
    cmake --build build
    echo ""
    echo "Build complete! Binary is at: ./build/s3al_sim"
    echo "Run with: ./build/s3al_sim"
else
    echo "Skipping build. To build later, run:"
    echo "  cmake -B build -G Ninja"
    echo "  cmake --build build"
fi
