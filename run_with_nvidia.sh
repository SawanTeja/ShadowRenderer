#!/bin/bash
# Force usage of NVIDIA GPU using Prime Render Offload
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia

# Check if build/Basic exists
if [ -f "./build/Basic" ]; then
    echo "Running with NVIDIA GPU..."
    ./build/Basic
else
    echo "Executable ./build/Basic not found. Building..."
    mkdir -p build
    cd build
    cmake ..
    if make; then
        echo "Build successful. Running..."
        cd ..
        ./build/Basic
    else
        echo "Build failed."
        exit 1
    fi
fi
