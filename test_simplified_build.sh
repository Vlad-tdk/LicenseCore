#!/bin/bash

echo "🔨 Testing simplified LicenseCore++ Obfuscated build for macOS..."

cd /Users/vlad/Coding/C++/LicenseCore/obfuscated

echo "📦 Trying simplified build without problematic flags..."

# Упрощенные флаги для macOS
g++ -std=c++17 -Os -DNDEBUG -fvisibility=hidden \
    -Wall -Wextra -mmacosx-version-min=10.14 \
    -DSTANDARD_OBFUSCATION \
    -Iinclude -Isrc \
    -c src/license_core_obfuscated.cpp -o src/license_core_obfuscated.o

if [ $? -eq 0 ]; then
    echo "✅ license_core_obfuscated.cpp compiled successfully"
    
    g++ -std=c++17 -Os -DNDEBUG -fvisibility=hidden \
        -Wall -Wextra -mmacosx-version-min=10.14 \
        -DSTANDARD_OBFUSCATION \
        -Iinclude -Isrc \
        -c src/crypto_utils.cpp -o src/crypto_utils.o
    
    if [ $? -eq 0 ]; then
        echo "✅ crypto_utils.cpp compiled successfully"
        
        g++ -std=c++17 -Os -DNDEBUG -fvisibility=hidden \
            -Wall -Wextra -mmacosx-version-min=10.14 \
            -DSTANDARD_OBFUSCATION \
            -Iinclude -Isrc \
            -c src/string_obfuscator.cpp -o src/string_obfuscator.o
        
        if [ $? -eq 0 ]; then
            echo "✅ string_obfuscator.cpp compiled successfully"
            
            echo "📦 Trying to compile hardware_detector.cpp..."
            g++ -std=c++17 -Os -DNDEBUG -fvisibility=hidden \
                -Wall -Wextra -mmacosx-version-min=10.14 \
                -DSTANDARD_OBFUSCATION \
                -Iinclude -Isrc \
                -c src/hardware_detector.cpp -o src/hardware_detector.o
            
            if [ $? -eq 0 ]; then
                echo "✅ All sources compiled successfully!"
                
                echo "📚 Creating static library..."
                ar rcs liblicense_core_obf.a src/*.o
                
                if [ $? -eq 0 ]; then
                    echo "✅ Static library created successfully!"
                    echo "📋 Library info:"
                    ls -la liblicense_core_obf.a
                    nm -g liblicense_core_obf.a | head -20
                else
                    echo "❌ Failed to create static library"
                fi
            else
                echo "❌ hardware_detector.cpp compilation failed"
            fi
        else
            echo "❌ string_obfuscator.cpp compilation failed"
        fi
    else
        echo "❌ crypto_utils.cpp compilation failed"
    fi
else
    echo "❌ license_core_obfuscated.cpp compilation failed"
fi

echo "Done."
