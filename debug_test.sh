#!/bin/bash
cd /Users/vlad/Coding/C++/LicenseCore/obfuscated

echo "🔧 Сборка DEBUG версии для диагностики..."

# Компилируем с DEBUG флагами
gcc -std=c99 -Os -DNDEBUG \
    -fvisibility=hidden \
    -DLICENSECORE_OBFUSCATED=1 \
    -DLICENSECORE_NO_STRINGS=1 \
    -DANTI_DEBUG_BUILD \
    -DDEBUG_JSON_PARSER \
    -Iinclude -Isrc \
    -c src/license_core_pure_c.c \
    -o src/license_core_pure_c_debug.o

if [ $? -eq 0 ]; then
    echo "✅ DEBUG компиляция успешна!"
    
    # Создаем DEBUG библиотеку
    ar rcs liblicense_core_debug.a src/license_core_pure_c_debug.o
    
    # Компилируем тест с DEBUG библиотекой
    gcc -std=c99 -Os -DNDEBUG \
        -fvisibility=hidden \
        -DLICENSECORE_OBFUSCATED=1 \
        -DDEBUG_JSON_PARSER \
        -Iinclude -Isrc \
        -o test_debug \
        test/test_obfuscated.c \
        liblicense_core_debug.a \
        -framework IOKit -framework CoreFoundation -framework Security
    
    if [ $? -eq 0 ]; then
        echo "✅ DEBUG тест скомпилирован!"
        echo ""
        echo "🚀 Запуск DEBUG теста..."
        ./test_debug
    else
        echo "❌ Ошибка компиляции теста"
    fi
else
    echo "❌ Ошибка DEBUG компиляции"
fi
