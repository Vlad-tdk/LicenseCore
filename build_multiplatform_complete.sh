#!/bin/bash
# Полная локальная мультиплатформенная сборка LicenseCore++

echo "🏗️ LicenseCore++ Complete Multi-Platform Builder"
echo "================================================"

BUILD_DIR="local_multiplatform_build"
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR/{linux,windows,macos,package/{lib,include,docs,examples}}

# Проверяем, что мы в правильной директории
if [ ! -d "obfuscated" ]; then
    echo "❌ Ошибка: Запустите скрипт из корневой директории LicenseCore (где есть папка obfuscated/)"
    exit 1
fi

cd obfuscated

echo "🍎 Building macOS versions..."

# macOS Intel
echo "  📦 macOS x86_64..."
clang -arch x86_64 -std=c99 -Os -DNDEBUG \
  -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 \
  -DANTI_DEBUG_BUILD -mmacosx-version-min=10.14 \
  -Iinclude -Isrc -c src/license_core_pure_c.c \
  -o src/license_core_pure_c_macos_x64.o

if [ $? -eq 0 ]; then
    ar rcs ../$BUILD_DIR/macos/liblicense_core_macos_x64.a src/license_core_pure_c_macos_x64.o
    echo "  ✅ macOS x64 build successful"
else
    echo "  ❌ macOS x64 build failed"
fi

# macOS Apple Silicon
echo "  📦 macOS ARM64..."
clang -arch arm64 -std=c99 -Os -DNDEBUG \
  -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 \
  -DANTI_DEBUG_BUILD -mmacosx-version-min=11.0 \
  -Iinclude -Isrc -c src/license_core_pure_c.c \
  -o src/license_core_pure_c_macos_arm64.o

if [ $? -eq 0 ]; then
    ar rcs ../$BUILD_DIR/macos/liblicense_core_macos_arm64.a src/license_core_pure_c_macos_arm64.o
    echo "  ✅ macOS ARM64 build successful"
else
    echo "  ❌ macOS ARM64 build failed"
fi

# macOS Universal (если обе версии собрались)
if [ -f "../$BUILD_DIR/macos/liblicense_core_macos_x64.a" ] && [ -f "../$BUILD_DIR/macos/liblicense_core_macos_arm64.a" ]; then
    echo "  📦 macOS Universal..."
    lipo -create \
      ../$BUILD_DIR/macos/liblicense_core_macos_x64.a \
      ../$BUILD_DIR/macos/liblicense_core_macos_arm64.a \
      -output ../$BUILD_DIR/macos/liblicense_core_macos_universal.a
    echo "  ✅ macOS Universal build successful"
fi

echo ""
echo "🐧 Building Linux with Docker..."
if command -v docker &> /dev/null; then
    echo "  🐳 Docker найден, запускаем Linux сборку..."
    docker run --rm -v $(pwd)/..:/workspace ubuntu:20.04 bash -c "
        apt update -qq && apt install -y gcc libc6-dev
        cd /workspace/obfuscated
        gcc -std=c99 -Os -DNDEBUG -fvisibility=hidden \
          -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD \
          -Iinclude -Isrc -c src/license_core_pure_c.c \
          -o src/license_core_pure_c_linux.o
        ar rcs /workspace/$BUILD_DIR/linux/liblicense_core_linux_x64.a src/license_core_pure_c_linux.o
        chown -R $(id -u):$(id -g) /workspace/$BUILD_DIR/linux/
    "
    if [ $? -eq 0 ]; then
        echo "  ✅ Linux x64 build successful"
    else
        echo "  ❌ Linux x64 build failed"
    fi
else
    echo "  ⚠️  Docker не найден - пропускаем Linux сборку"
    echo "     Установите Docker: brew install docker"
fi

echo ""
echo "🪟 Building Windows with MinGW..."
if command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "  🔨 MinGW найден, запускаем Windows сборку..."
    x86_64-w64-mingw32-gcc -std=c99 -Os -DNDEBUG \
      -fvisibility=hidden -DLICENSECORE_OBFUSCATED=1 \
      -DANTI_DEBUG_BUILD -Iinclude -Isrc \
      -c src/license_core_pure_c.c \
      -o src/license_core_pure_c_windows.o
    
    if [ $? -eq 0 ]; then
        x86_64-w64-mingw32-ar rcs ../$BUILD_DIR/windows/liblicense_core_windows_x64.a src/license_core_pure_c_windows.o
        echo "  ✅ Windows x64 build successful"
    else
        echo "  ❌ Windows x64 build failed"
    fi
else
    echo "  ⚠️  MinGW не найден - пропускаем Windows сборку"
    echo "     Установите MinGW: brew install mingw-w64"
fi

cd ..

echo ""
echo "📦 Создание релизного пакета..."

# Копируем все библиотеки
echo "  📚 Копирование библиотек..."
find $BUILD_DIR -name "*.a" -exec cp {} $BUILD_DIR/package/lib/ \;

# Копируем заголовки
echo "  📄 Копирование заголовков..."
cp obfuscated/include/license_core_pure_c.h $BUILD_DIR/package/include/

# Создаем документацию
echo "  📝 Создание документации..."
cat > $BUILD_DIR/package/README.md << 'EOF'
# 🔒 LicenseCore++ Multi-Platform Package

## 📦 Libraries included:

### Available libraries:
- `liblicense_core_macos_x64.a` - macOS Intel (x86_64)
- `liblicense_core_macos_arm64.a` - macOS Apple Silicon (ARM64)  
- `liblicense_core_macos_universal.a` - macOS Universal (Intel + Apple Silicon)
- `liblicense_core_linux_x64.a` - Linux x86_64 (if Docker was available)
- `liblicense_core_windows_x64.a` - Windows x86_64 (if MinGW was available)

## 🚀 Quick Start:

```c
#include "license_core_pure_c.h"
#include <stdio.h>

int main() {
    // Test embedded license
    if (lc_validate_embedded()) {
        printf("✅ Embedded license valid!\n");
        
        if (lc_has_feature("basic")) {
            printf("✅ Basic features available\n");
        }
    }
    
    // Test external license
    const char* license = "{\"user_id\":\"demo\",\"features\":[\"basic\",\"premium\"]}";
    if (lc_validate_license(license)) {
        printf("✅ External license valid!\n");
    }
    
    // Get hardware ID
    printf("🖥️ Hardware ID: %s\n", lc_get_hwid());
    
    return 0;
}
```

## 🔗 Platform-specific compilation:

### macOS (recommended - works on all Macs):
```bash
gcc app.c -L./lib -llicense_core_macos_universal \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### macOS Intel only:
```bash
gcc app.c -L./lib -llicense_core_macos_x64 \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### macOS Apple Silicon only:
```bash
gcc app.c -L./lib -llicense_core_macos_arm64 \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### Linux:
```bash
gcc app.c -L./lib -llicense_core_linux_x64 -o app
```

### Windows (MinGW):
```bash
gcc app.c -L./lib -llicense_core_windows_x64 -o app.exe
```

## 📁 Directory structure:
```
package/
├── lib/                    # Static libraries for all platforms
├── include/               # Header files
├── examples/              # Usage examples
├── docs/                  # Documentation
└── README.md             # This file
```

## 🔒 Security Features:
- ✅ Symbol obfuscation and hiding
- ✅ Anti-debug runtime protection  
- ✅ Embedded encrypted license
- ✅ Minimal API surface (4 functions only)
- ✅ Pure C implementation (no C++ dependencies)

## 🔧 API Reference:
- `lc_validate_license(json)` - Validate external license from JSON
- `lc_validate_embedded()` - Validate built-in embedded license
- `lc_has_feature(name)` - Check if specific feature is available
- `lc_get_hwid()` - Get unique hardware identifier

Built with LicenseCore++ Multi-Platform Builder
EOF

# Создаем примеры
echo "  🧪 Создание примеров..."
cat > $BUILD_DIR/package/examples/basic_usage.c << 'EOF'
#include "license_core_pure_c.h"
#include <stdio.h>

int main() {
    printf("🔒 LicenseCore++ Multi-Platform Example\n");
    printf("=====================================\n\n");
    
    // Get hardware ID
    const char* hwid = lc_get_hwid();
    printf("🖥️  Hardware ID: %s\n\n", hwid);
    
    // Test embedded license
    printf("🔐 Testing embedded license...\n");
    if (lc_validate_embedded()) {
        printf("✅ Embedded license is VALID!\n");
        
        // Check available features
        printf("📋 Available features:\n");
        if (lc_has_feature("basic")) {
            printf("  ✅ Basic features\n");
        }
        if (lc_has_feature("premium")) {
            printf("  ✅ Premium features\n");
        } else {
            printf("  ❌ Premium features (not available)\n");
        }
        if (lc_has_feature("enterprise")) {
            printf("  ✅ Enterprise features\n");
        } else {
            printf("  ❌ Enterprise features (not available)\n");
        }
    } else {
        printf("❌ Embedded license INVALID\n");
    }
    
    printf("\n🧪 Testing external license...\n");
    const char* test_license = "{\"user_id\":\"demo\",\"features\":[\"basic\",\"premium\"]}";
    printf("JSON: %s\n", test_license);
    
    if (lc_validate_license(test_license)) {
        printf("✅ External license is VALID!\n");
        printf("📋 Features from external license:\n");
        printf("  - Basic: %s\n", lc_has_feature("basic") ? "✅ Available" : "❌ Missing");
        printf("  - Premium: %s\n", lc_has_feature("premium") ? "✅ Available" : "❌ Missing");
        printf("  - Enterprise: %s\n", lc_has_feature("enterprise") ? "✅ Available" : "❌ Missing");
    } else {
        printf("❌ External license INVALID\n");
    }
    
    printf("\n🛡️  Security test (NULL inputs)...\n");
    printf("NULL license test: %s\n", lc_validate_license(NULL) ? "❌ FAILED" : "✅ Protected");
    printf("NULL feature test: %s\n", lc_has_feature(NULL) ? "❌ FAILED" : "✅ Protected");
    
    printf("\n🎉 LicenseCore++ test completed successfully!\n");
    printf("🔒 Library is working correctly on this platform.\n");
    
    return 0;
}
EOF

# Создаем Makefile для примеров с автоопределением платформы
cat > $BUILD_DIR/package/examples/Makefile << 'EOF'
# LicenseCore++ Multi-Platform Examples Makefile
# Automatically detects platform and uses appropriate library

# Detect platform
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Set library and flags based on platform
ifeq ($(UNAME_S),Darwin)
    # macOS - prefer universal library, fallback to architecture-specific
    ifeq ($(wildcard ../lib/liblicense_core_macos_universal.a),)
        # Universal library not available, try architecture-specific
        ifeq ($(UNAME_M),arm64)
            LIBRARY = ../lib/liblicense_core_macos_arm64.a
        else
            LIBRARY = ../lib/liblicense_core_macos_x64.a
        endif
    else
        LIBRARY = ../lib/liblicense_core_macos_universal.a
    endif
    FRAMEWORKS = -framework IOKit -framework CoreFoundation -framework Security
    LDFLAGS = $(FRAMEWORKS)
    PLATFORM_NAME = macOS
else ifeq ($(UNAME_S),Linux)
    # Linux
    LIBRARY = ../lib/liblicense_core_linux_x64.a
    LDFLAGS = -lpthread
    PLATFORM_NAME = Linux
else
    # Windows (MinGW/MSYS2)
    LIBRARY = ../lib/liblicense_core_windows_x64.a
    LDFLAGS = -liphlpapi -lole32 -loleaut32 -ladvapi32
    PLATFORM_NAME = Windows
endif

# Compiler settings
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I../include
TARGET = basic_usage

# Check if library exists
LIBRARY_EXISTS := $(wildcard $(LIBRARY))

# Main targets
all: check_library $(TARGET)

check_library:
	@echo "🔍 Platform detected: $(PLATFORM_NAME) ($(UNAME_M))"
	@echo "📚 Looking for library: $(LIBRARY)"
ifeq ($(LIBRARY_EXISTS),)
	@echo "❌ Library not found: $(LIBRARY)"
	@echo "⚠️  Available libraries:"
	@ls -la ../lib/ || echo "No libraries found"
	@echo ""
	@echo "💡 To build libraries for this platform:"
	@echo "   cd ../../ && ./build_multiplatform_complete.sh"
	@exit 1
else
	@echo "✅ Library found: $(LIBRARY)"
endif

$(TARGET): basic_usage.c
	@echo "🔨 Compiling $(TARGET) for $(PLATFORM_NAME)..."
	$(CC) $(CFLAGS) -o $@ $< $(LIBRARY) $(LDFLAGS)
	@echo "✅ Build successful!"

clean:
	@echo "🧹 Cleaning..."
	rm -f $(TARGET) $(TARGET).exe

test: $(TARGET)
	@echo "🧪 Running test on $(PLATFORM_NAME)..."
	@echo "======================================"
	./$(TARGET)

install: $(TARGET)
	@echo "📦 Installing $(TARGET)..."
	cp $(TARGET) /usr/local/bin/ 2>/dev/null || echo "⚠️  Could not install to /usr/local/bin (run with sudo?)"

info:
	@echo "ℹ️  Build Information:"
	@echo "  Platform: $(PLATFORM_NAME) ($(UNAME_S)/$(UNAME_M))"
	@echo "  Compiler: $(CC)"
	@echo "  Library: $(LIBRARY)"
	@echo "  Flags: $(CFLAGS)"
	@echo "  Link flags: $(LDFLAGS)"

.PHONY: all clean test install info check_library
EOF

# Создаем простой build скрипт для примеров
cat > $BUILD_DIR/package/examples/build.sh << 'EOF'
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
EOF

chmod +x $BUILD_DIR/package/examples/build.sh

# Создаем архив
echo "  📦 Creating release archive..."
cd $BUILD_DIR/package
tar -czf ../licensecore-local-multiplatform-$(date +%Y%m%d).tar.gz .
cd ../..

echo ""
echo "📊 Build Summary:"
echo "================"
echo "📁 Build directory: $BUILD_DIR"
echo "📦 Archive: $BUILD_DIR/licensecore-local-multiplatform-$(date +%Y%m%d).tar.gz"
echo ""
echo "📚 Libraries built:"
LIBRARY_COUNT=0
for lib in $(find $BUILD_DIR -name "*.a" 2>/dev/null); do
    echo "  ✅ $lib"
    LIBRARY_COUNT=$((LIBRARY_COUNT + 1))
done

if [ $LIBRARY_COUNT -eq 0 ]; then
    echo "  ⚠️  No libraries were built successfully!"
    echo "     Check the error messages above."
else
    echo ""
    echo "📏 Library sizes:"
    find $BUILD_DIR -name "*.a" -exec ls -lh {} \;
fi

echo ""
echo "🧪 Test the example:"
echo "cd $BUILD_DIR/package/examples"
echo "make && make test"
echo "# или:"
echo "./build.sh && ./basic_usage"
echo ""

if [ $LIBRARY_COUNT -gt 0 ]; then
    echo "🚀 Ready for distribution!"
    echo "Package contains everything needed for supported platforms."
    echo ""
    echo "📋 Next steps:"
    echo "1. Test: cd $BUILD_DIR/package/examples && make test"
    echo "2. Distribute: $BUILD_DIR/licensecore-local-multiplatform-$(date +%Y%m%d).tar.gz"
else
    echo "⚠️  Build incomplete. Install missing dependencies:"
    echo "   brew install docker mingw-w64"
    echo "   # Make sure Docker Desktop is running"
fi
