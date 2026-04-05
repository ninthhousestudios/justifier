#!/bin/bash
set -e

# Build justifier_audio dynamic library for macOS
cd "${PROJECT_DIR}/../native"

# Create build directory
BUILD_DIR="${PROJECT_DIR}/../build/macos/Build/Products/${CONFIGURATION}"
mkdir -p "${BUILD_DIR}"

# Check if library already exists and is up to date
if [ -f "${BUILD_DIR}/libjustifier_audio.dylib" ] && [ "${BUILD_DIR}/libjustifier_audio.dylib" -nt "${CMAKELists.txt}" ]; then
    echo "Library is up to date"
    exit 0
fi

# Build with CMake
cmake -B "${PROJECT_DIR}/../build/native" -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_BUILD_TYPE=Release
cmake --build "${PROJECT_DIR}/../build/native" --target justifier_audio --config Release

# Copy to Flutter bundle
cp "${PROJECT_DIR}/../build/native/Release/libjustifier_audio.dylib" "${BUILD_DIR}/libjustifier_audio.dylib"
echo "Copied libjustifier_audio.dylib to ${BUILD_DIR}"
