#!/bin/bash

echo "🚀 === LicenseCore++ Installation Script ==="
echo ""

# Detect platform
PLATFORM=$(uname)
echo "📱 Platform: $PLATFORM"

install_dependencies() {
    case $PLATFORM in
        "Darwin")
            echo "🍺 Installing dependencies on macOS..."
            if command -v brew >/dev/null 2>&1; then
                echo "Installing OpenSSL..."
                brew install openssl
                echo "Installing CMake..."
                brew install cmake
            else
                echo "❌ Homebrew not found. Please install from https://brew.sh/"
                exit 1
            fi
            ;;
        "Linux")
            echo "🐧 Installing dependencies on Linux..."
            if command -v apt-get >/dev/null 2>&1; then
                echo "Installing build tools and OpenSSL..."
                sudo apt-get update
                sudo apt-get install -y build-essential libssl-dev cmake pkg-config
            elif command -v yum >/dev/null 2>&1; then
                echo "Installing build tools and OpenSSL..."
                sudo yum install -y gcc-c++ openssl-devel cmake3 pkgconfig
            elif command -v dnf >/dev/null 2>&1; then
                echo "Installing build tools and OpenSSL..."
                sudo dnf install -y gcc-c++ openssl-devel cmake pkgconfig
            else
                echo "❌ Package manager not found. Please install dependencies manually:"
                echo "   - C++ compiler (gcc/clang)"
                echo "   - OpenSSL development headers"
                echo "   - CMake (optional)"
                exit 1
            fi
            ;;
        *)
            echo "❌ Unsupported platform: $PLATFORM"
            exit 1
            ;;
    esac
}

build_project() {
    echo ""
    echo "🔨 Building LicenseCore++..."
    
    # Try CMake first
    if command -v cmake >/dev/null 2>&1; then
        echo "Using CMake build system..."
        mkdir -p build
        cd build
        cmake ..
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        if [ $? -eq 0 ]; then
            echo "✅ CMake build successful"
            cd ..
            return 0
        else
            echo "⚠️  CMake build failed, trying Makefile..."
            cd ..
        fi
    fi
    
    # Fallback to Makefile
    echo "Using Makefile build system..."
    make test_full
    if [ $? -eq 0 ]; then
        echo "✅ Makefile build successful"
        return 0
    else
        echo "❌ Build failed"
        return 1
    fi
}

run_tests() {
    echo ""
    echo "🧪 Running tests..."
    
    # Run simple test first
    if [ -f "./test_simple" ]; then
        echo "Running simple test..."
        ./test_simple
    fi
    
    # Run full test
    if [ -f "./build/examples/simple_example" ]; then
        echo "Running CMake example..."
        ./build/examples/simple_example
    elif [ -f "./test_full" ]; then
        echo "Running Makefile test..."
        ./test_full
    fi
    
    echo ""
    echo "🎯 Available tools:"
    find . -name "simple_example" -o -name "hwid_tool" -o -name "license_generator" 2>/dev/null
}

show_usage() {
    echo ""
    echo "📚 === Usage Instructions ==="
    echo ""
    echo "Integration:"
    echo "  #include <license_core/license_manager.hpp>"
    echo "  // Link: -llicensecore -lssl -lcrypto"
    echo ""
    echo "API Example:"
    echo "  LicenseManager manager(\"secret-key\");"
    echo "  auto info = manager.load_and_validate(license_json);"
    echo "  if (info.valid && manager.has_feature(\"premium\")) {"
    echo "      // Premium features unlocked"
    echo "  }"
    echo ""
    echo "Tools:"
    echo "  ./hwid_tool              - Get hardware fingerprint"
    echo "  ./license_generator      - Generate licenses"
    echo "  ./simple_example         - API demonstration"
    echo ""
}

# Main installation flow
echo "1️⃣ Installing dependencies..."
install_dependencies

echo ""
echo "2️⃣ Building project..."
if build_project; then
    echo ""
    echo "3️⃣ Running tests..."
    run_tests
    
    echo ""
    echo "✅ === Installation Complete! ==="
    show_usage
    
    echo "🎉 LicenseCore++ is ready to use!"
else
    echo ""
    echo "❌ === Installation Failed ==="
    echo ""
    echo "🔧 Manual troubleshooting:"
    echo "  1. Check that OpenSSL is installed"
    echo "  2. Run: chmod +x test_build.sh && ./test_build.sh"
    echo "  3. Check build output for specific errors"
    echo ""
    echo "💬 For support, check README.md or QUICKSTART.md"
    exit 1
fi
