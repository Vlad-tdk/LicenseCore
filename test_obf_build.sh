#!/bin/bash

echo "🔨 Testing LicenseCore++ Obfuscated build..."

# Go to obfuscated directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
cd "$REPO_ROOT/obfuscated"

# Try simple build
echo "📦 Attempting simple build..."
make clean
make OBFUSCATION_LEVEL=standard

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📋 Generated files:"
    ls -la *.a 2>/dev/null || echo "No .a files found"
    ls -la *.o 2>/dev/null || echo "No .o files found"
else
    echo "❌ Build failed. Checking errors..."
    echo "📋 Available targets:"
    grep "^[a-zA-Z].*:" Makefile
fi

echo "Done."
