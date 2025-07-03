#!/bin/bash

# LicenseCore++ WASM Build Script
set -e

# Автоматически подгружаем emsdk окружение
if [ -f "$HOME/emsdk/emsdk_env.sh" ]; then
    source "$HOME/emsdk/emsdk_env.sh"
else
    echo "❌ Не найден $HOME/emsdk/emsdk_env.sh"
    exit 1
fi
echo "🔧 Building LicenseCore++ WASM..."

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "❌ Emscripten not found!"
    echo ""
    echo "Install Emscripten:"
    echo "1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "2. cd emsdk"
    echo "3. ./emsdk install latest"
    echo "4. ./emsdk activate latest"
    echo "5. source ./emsdk_env.sh"
    echo ""
    exit 1
fi

echo "✅ Emscripten found: $(emcc --version | head -1)"

# Create output directory
mkdir -p wasm/build

# Build WASM module
echo "🔨 Compiling C++ to WASM..."

emcc \
    -std=c++17 \
    -O3 \
    --bind \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="LicenseCoreModule" \
    -s ENVIRONMENT=web \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "_hmac_ready", "_hmac_error"]' \
    -s EXPORTED_FUNCTIONS='["_hmac_ready", "_hmac_error", "_main"]' \
    -s NO_EXIT_RUNTIME=1 \
    -s ASSERTIONS=0 \
    -s ASYNCIFY=1 \
    -s ASYNCIFY_WHITELIST='["compute_hmac_sha256"]' \
    license_core_wasm.cpp \
    -o build/license_core.js

if [ $? -eq 0 ]; then
    echo "✅ WASM build successful!"
    echo ""
    echo "📁 Generated files:"
    echo "   wasm/build/license_core.js   - JavaScript loader"
    echo "   wasm/build/license_core.wasm - WebAssembly binary"
    echo ""
    echo "📦 File sizes:"
    ls -lh wasm/build/
    echo ""
    echo "🔧 Integration:"
    echo "   Copy files to docs/ directory"
    echo "   Update demo.js to use WASM module"
    echo ""
else
    echo "❌ WASM build failed!"
    exit 1
fi

# Copy to docs directory for demo
if [ -d "docs" ]; then
    echo "📋 Copying WASM files to docs/..."
    cp wasm/build/license_core.js docs/
    cp wasm/build/license_core.wasm docs/
    echo "✅ Files copied to docs/"
fi

echo ""
echo "🚀 WASM module ready for integration!"
echo ""
echo "Usage in JavaScript:"
echo "============================"
cat << 'EOF'
import LicenseCoreModule from './license_core.js';

LicenseCoreModule().then(Module => {
    const manager = new Module.LicenseCoreWasm("secret-key");

    const hwid = manager.getCurrentHwid();
    console.log("Hardware ID:", hwid);

    const features = new Module.VectorString();
    features.push_back("basic");
    features.push_back("premium");

    const license = manager.generateLicense("user-123", features, 365);
    console.log("License:", license);

    const result = manager.validateLicense(license);
    console.log("Valid:", result.valid);
    console.log("User:", result.user_id);
});
EOF

echo ""
echo "🎯 Next: Update demo.js to use real WASM instead of simulation!"