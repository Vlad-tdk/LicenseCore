#!/bin/bash
# Простая сборка примера для текущей платформы

echo "🔨 LicenseCore++ Example Builder"
echo "==============================="

# Определяем платформу
case "$(uname -s)" in
    Darwin)
        echo "🍎 macOS detected"
        if [ -f "../lib/liblicense_core_macos_universal.a" ]; then
            LIB="../lib/liblicense_core_macos_universal.a"
            echo "📚 Using universal library"
        elif [ "$(uname -m)" = "arm64" ] && [ -f "../lib/liblicense_core_macos_arm64.a" ]; then
            LIB="../lib/liblicense_core_macos_arm64.a"
            echo "📚 Using ARM64 library"
        elif [ -f "../lib/liblicense_core_macos_x64.a" ]; then
            LIB="../lib/liblicense_core_macos_x64.a"
            echo "📚 Using x64 library"
        else
            echo "❌ No macOS library found!"
            exit 1
        fi
        FRAMEWORKS="-framework IOKit -framework CoreFoundation -framework Security"
        gcc -std=c99 -I../include basic_usage.c $LIB $FRAMEWORKS -o basic_usage
        ;;
    Linux)
        echo "🐧 Linux detected"
        LIB="../lib/liblicense_core_linux_x64.a"
        if [ ! -f "$LIB" ]; then
            echo "❌ Linux library not found: $LIB"
            exit 1
        fi
        gcc -std=c99 -I../include basic_usage.c $LIB -lpthread -o basic_usage
        ;;
    *)
        echo "🪟 Windows/Other detected"
        LIB="../lib/liblicense_core_windows_x64.a"
        if [ ! -f "$LIB" ]; then
            echo "❌ Windows library not found: $LIB"
            exit 1
        fi
        gcc -std=c99 -I../include basic_usage.c $LIB -liphlpapi -lole32 -loleaut32 -ladvapi32 -o basic_usage.exe
        ;;
esac

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "🚀 Run with: ./basic_usage"
else
    echo "❌ Build failed!"
    exit 1
fi
