#!/bin/bash

# LicenseCore Build Script

set -e

BUILD_TYPE=${1:-Release}
BUILD_EXAMPLES=${2:-OFF}
BUILD_TESTS=${3:-OFF}
BUILD_GTESTS=${4:-OFF}

echo "=== Building LicenseCore ==="
echo "Build Type: $BUILD_TYPE"
echo "Examples: $BUILD_EXAMPLES"
echo "Tests: $BUILD_TESTS"
echo "Google Tests: $BUILD_GTESTS"
echo ""

# Create build directory
mkdir -p build
cd build

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DLICENSECORE_BUILD_EXAMPLES=$BUILD_EXAMPLES \
    -DLICENSECORE_BUILD_TESTS=$BUILD_TESTS \
    -DLICENSECORE_BUILD_GTESTS=$BUILD_GTESTS

# Build
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "=== Build Complete ==="

if [ "$BUILD_TESTS" = "ON" ]; then
    echo "Running tests..."
    ctest --output-on-failure
fi

if [ "$BUILD_EXAMPLES" = "ON" ]; then
    echo ""
    echo "=== Example Executables ==="
    echo "Simple Example: ./examples/simple_example"
    echo "License Generator: ./examples/license_generator"
    echo "Hardware ID Tool: ./examples/hwid_tool"
fi

echo ""
echo "Library built: liblicensecore.a"
echo "Installation: make install"
