#!/bin/bash
# Force usage of NVIDIA GPU using Prime Render Offload
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia

echo "Checking for in-source build artifacts..."
# Clean in-source build if present (safety)
if [ -f "CMakeCache.txt" ]; then
    echo "Detected in-source build artifacts. Cleaning..."
    rm CMakeCache.txt Makefile cmake_install.cmake
    rm -rf CMakeFiles
fi
if [ -f "Basic" ]; then
    rm Basic
fi

# Ensure build dir exists
mkdir -p build
cd build

# Run cmake if needed
if [ ! -f "Makefile" ]; then
    echo "Configuring with CMake..."
    cmake ..
fi

# Build
echo "Building..."
if make; then
    echo "Build successful. Launching..."
    ./Basic
else
    echo "Build failed."
    exit 1
fi
