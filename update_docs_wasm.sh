#!/bin/bash

echo "🔨 ОБНОВЛЕНИЕ WASM ДЛЯ САЙТА DOCS"
echo "================================"

cd /Users/vlad/Coding/C++/LicenseCore

# Check if we have emsdk
if [ ! -d "$HOME/emsdk" ]; then
    echo "❌ Emscripten SDK не найден в $HOME/emsdk"
    echo ""
    echo "Установите Emscripten:"
    echo "cd ~"
    echo "git clone https://github.com/emscripten-core/emsdk.git"
    echo "cd emsdk"
    echo "./emsdk install latest"
    echo "./emsdk activate latest"
    echo "source ./emsdk_env.sh"
    exit 1
fi

# Activate emsdk
echo "🔧 Активация Emscripten SDK..."
source $HOME/emsdk/emsdk_env.sh

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "❌ emcc не найден после активации emsdk"
    exit 1
fi

echo "✅ Emscripten готов: $(emcc --version | head -1)"

# Build WASM
echo ""
echo "🔨 Сборка WASM модуля..."
cd wasm

# Run the existing build script
chmod +x build_wasm.sh
./build_wasm.sh

if [ $? -ne 0 ]; then
    echo "❌ Сборка WASM провалена!"
    exit 1
fi

# Check if files were generated
if [ ! -f "build/license_core.js" ] || [ ! -f "build/license_core.wasm" ]; then
    echo "❌ WASM файлы не найдены после сборки"
    exit 1
fi

cd ..

# Copy fresh WASM files to docs
echo ""
echo "📋 Копирование WASM файлов в docs/..."
cp wasm/build/license_core.js docs/
cp wasm/build/license_core.wasm docs/

echo "✅ WASM файлы обновлены в docs/"

# Show file sizes
echo ""
echo "📊 Размеры файлов:"
ls -lh docs/license_core.*

echo ""
echo "🌐 Тестирование сайта:"
echo "cd docs && python3 -m http.server 8000"
echo "Открыть: http://localhost:8000"

echo ""
echo "✅ WASM интеграция готова!"
echo ""
echo "🔍 Проверьте:"
echo "  • Откройте сайт в браузере"
echo "  • Проверьте Console (F12) на наличие 'WASM module loaded successfully!'"
echo "  • Генерируйте лицензию и убедитесь что используется C++ WASM"
echo "  • Попробуйте Performance Test в Demo Scenarios"
