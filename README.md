# ShadowRenderer Engine

This project is a custom C++ graphics and physics engine utilizing OpenGL and GTK3. It features a custom physics engine with semi-implicit Euler integration, AABB collision detection, and a planar projection shadow system using stencil buffers.

## Prerequisites

Before building the project, ensure you have the necessary development libraries and tools installed on your Linux system.

### Required Packages

For Debian/Ubuntu-based systems, you will need to install the following packages:
- cmake (version 3.10 or higher)
- make
- g++ or clang++
- pkg-config
- libgtk-3-dev
- libgl1-mesa-dev

You can install these by running:
```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libgtk-3-dev libgl1-mesa-dev
```

## How to Run This Project (Standard)

To build and run the project using your default graphics processor, follow these steps:

1. Clone or navigate to the project root directory.
2. Create a dedicated build directory to avoid polluting the source tree:
   ```bash
   mkdir build
   cd build
   ```
3. Generate the build files using CMake:
   ```bash
   cmake ..
   ```
4. Compile the project:
   ```bash
   make
   ```
5. Run the generated executable:
   ```bash
   ./Basic
   ```

## How to Run This Project Using NVIDIA GPU

If you are on a laptop with hybrid graphics (NVIDIA Optimus) or a system where you specifically want to force the application to run on the dedicated NVIDIA GPU, you can use Prime Render Offload.

A helper script `run_with_nvidia.sh` is provided in the root directory to automate this process.

1. Ensure the script has execution permissions:
   ```bash
   chmod +x run_with_nvidia.sh
   ```
2. Execute the script from the root directory:
   ```bash
   ./run_with_nvidia.sh
   ```

### What the NVIDIA Script Does

Under the hood, the script exports specific environment variables before launching the executable to force the NVIDIA driver to handle the OpenGL rendering:
- `__NV_PRIME_RENDER_OFFLOAD=1`
- `__GLX_VENDOR_LIBRARY_NAME=nvidia`

It also performs a safety check to clean any accidental in-source build artifacts (such as a stray `CMakeCache.txt` or `Makefile` in the root directory) before building the project in a proper `build` directory.

## Troubleshooting:

Below are common issues you might encounter while building or running the engine, along with their solutions.

### 1. CMake Configuration Fails or Build Fails Repeatedly
**Symptom:** You receive errors from CMake about cached variables, or `make` fails with strange missing file errors, often after you previously tried to build the project.
**Cause:** You likely performed an "in-source build" previously by running `cmake .` in the root directory. This creates artifacts that conflict with the `build` directory setup.
**Fix:** Delete all CMake generated files from the root directory. Run the following commands in the project root:
```bash
rm -f CMakeCache.txt Makefile cmake_install.cmake
rm -rf CMakeFiles
```
After cleaning, navigate back into your `build` directory, remove its contents, and run `cmake ..` again.

### 2. Missing GTK3 or OpenGL Dependencies
**Symptom:** CMake fails with an error stating `pkg_check_modules` could not find `gtk+-3.0`, or `find_package` could not find `OpenGL`.
**Cause:** The required development headers for GTK3 or OpenGL are missing from your system.
**Fix:** Install the missing libraries. On Ubuntu/Debian, run:
```bash
sudo apt install libgtk-3-dev libgl1-mesa-dev pkg-config
```

### 3. NVIDIA Script Fails to Use Dedicated GPU
**Symptom:** Running the script still uses the integrated Intel/AMD graphics, or the program crashes immediately upon launch with the NVIDIA script.
**Cause:** Your proprietary NVIDIA drivers are either not installed, not configured for Prime Render Offload, or your user does not have the necessary permissions.
**Fix:** 
- Verify your drivers are installed by running `nvidia-smi` in the terminal. If the command is not found, you must install the proprietary NVIDIA drivers from your distribution's driver manager.
- Ensure your system supports Prime Offloading (requires NVIDIA driver version 435.17 or newer).

### 4. Permission Denied When Executing the Script
**Symptom:** Output reads `bash: ./run_with_nvidia.sh: Permission denied`.
**Cause:** The script lacks the executable bit.
**Fix:** Add the executable permission by running:
```bash
chmod +x run_with_nvidia.sh
```

### 5. Display Server Issues (Wayland vs X11)
**Symptom:** The application window appears completely black, fails to render, or crashes complaining about the display server.
**Cause:** GTK3 and OpenGL applications can sometimes experience issues when running natively on Wayland, especially when forced through proprietary NVIDIA drivers.
**Fix:** Force the application to use the X11 backend (Xwayland) by prepending the `GDK_BACKEND` environment variable. Modify your run command to:
```bash
export GDK_BACKEND=x11
./run_with_nvidia.sh
```
Or for the standard run:
```bash
export GDK_BACKEND=x11
./Basic
```
