#!/bin/bash
# Кроссплатформенная сборка LicenseCore++

echo "🌍 LicenseCore++ Multi-Platform Builder"
echo "======================================"

PLATFORMS=("linux-x64" "windows-x64" "macos-x64" "macos-arm64")
OUTPUT_DIR="release_multiplatform"

mkdir -p $OUTPUT_DIR

for platform in "${PLATFORMS[@]}"; do
    echo ""
    echo "🔨 Building for $platform..."
    
    case $platform in
        "linux-x64")
            echo "🐧 Linux x64 build..."
            docker run --rm -v $(pwd):/workspace ubuntu:20.04 bash -c "
                apt update && apt install -y gcc libc6-dev
                cd /workspace/obfuscated
                gcc -std=c99 -Os -DNDEBUG -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD -Iinclude -Isrc -c src/license_core_pure_c.c -o src/license_core_pure_c_linux.o
                ar rcs liblicense_core_linux_x64.a src/license_core_pure_c_linux.o
            "
            cp obfuscated/liblicense_core_linux_x64.a $OUTPUT_DIR/
            ;;
            
        "windows-x64")
            echo "🪟 Windows x64 build..."
            if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
                cd obfuscated
                x86_64-w64-mingw32-gcc -std=c99 -Os -DNDEBUG -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD -Iinclude -Isrc -c src/license_core_pure_c.c -o src/license_core_pure_c_win.o
                x86_64-w64-mingw32-ar rcs liblicense_core_windows_x64.a src/license_core_pure_c_win.o
                cp liblicense_core_windows_x64.a ../$OUTPUT_DIR/
                cd ..
            else
                echo "⚠️  MinGW не установлен. Установите: brew install mingw-w64"
            fi
            ;;
            
        "macos-x64")
            echo "🍎 macOS x64 build..."
            cd obfuscated
            gcc -arch x86_64 -std=c99 -Os -DNDEBUG -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD -mmacosx-version-min=10.14 -Iinclude -Isrc -c src/license_core_pure_c.c -o src/license_core_pure_c_macos_x64.o
            ar rcs liblicense_core_macos_x64.a src/license_core_pure_c_macos_x64.o
            cp liblicense_core_macos_x64.a ../$OUTPUT_DIR/
            cd ..
            ;;
            
        "macos-arm64")
            echo "🍎 macOS ARM64 build..."
            cd obfuscated
            gcc -arch arm64 -std=c99 -Os -DNDEBUG -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD -mmacosx-version-min=11.0 -Iinclude -Isrc -c src/license_core_pure_c.c -o src/license_core_pure_c_macos_arm64.o
            ar rcs liblicense_core_macos_arm64.a src/license_core_pure_c_macos_arm64.o
            cp liblicense_core_macos_arm64.a ../$OUTPUT_DIR/
            cd ..
            ;;
    esac
done

echo ""
echo "📦 Создание мультиплатформенного релиза..."
cd $OUTPUT_DIR
cp ../obfuscated/include/license_core_pure_c.h .

cat > README_MULTIPLATFORM.md << 'EOF'
# LicenseCore++ Multi-Platform Release

## Platforms Included:
- `liblicense_core_linux_x64.a` - Linux x86_64
- `liblicense_core_windows_x64.a` - Windows x86_64 (MinGW)
- `liblicense_core_macos_x64.a` - macOS x86_64 (Intel Mac)
- `liblicense_core_macos_arm64.a` - macOS ARM64 (Apple Silicon)

## Usage:
```c
#include "license_core_pure_c.h"

int main() {
    if (lc_validate_embedded()) {
        printf("License valid!\n");
    }
    return 0;
}
```

## Linking:
```bash
# Linux
gcc app.c -L. -llicense_core_linux_x64

# Windows (MinGW)
gcc app.c -L. -llicense_core_windows_x64

# macOS Intel
gcc app.c -L. -llicense_core_macos_x64

# macOS Apple Silicon  
gcc app.c -L. -llicense_core_macos_arm64
```
EOF

tar -czf licensecore-multiplatform-$(date +%Y%m%d).tar.gz *.a *.h *.md

echo "✅ Мультиплатформенный релиз готов!"
echo "📁 Файлы:"
ls -la
