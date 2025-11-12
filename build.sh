#!/bin/bash

# Authix C++ Build Script

set -e

echo "======================================"
echo "  Authix C++ Build Script"
echo "======================================"
echo ""

# Check for required tools
echo "Checking for required tools..."

if ! command -v cmake &> /dev/null; then
    echo "Error: CMake is not installed. Please install CMake 3.15 or higher."
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "Error: No C++ compiler found. Please install g++ or clang++."
    exit 1
fi

echo "✓ CMake found: $(cmake --version | head -n1)"
if command -v g++ &> /dev/null; then
    echo "✓ g++ found: $(g++ --version | head -n1)"
elif command -v clang++ &> /dev/null; then
    echo "✓ clang++ found: $(clang++ --version | head -n1)"
fi
echo ""

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure
echo ""
echo "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo ""
echo "Building project..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "======================================"
echo "  Build Complete!"
echo "======================================"
echo ""
echo "To run the bot:"
echo "  cd build"
echo "  ./authix"
echo ""
echo "Make sure to configure config.json first!"
