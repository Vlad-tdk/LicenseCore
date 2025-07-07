#!/bin/bash
# Исправленная версия скрипта сборки для тестирования

echo "🔧 Попытка простой сборки C файла..."

# Переходим в правильную директорию
cd /Users/vlad/Coding/C++/LicenseCore/obfuscated

echo "Текущая директория: $(pwd)"
echo "Содержимое:"
ls -la

# Создаем простую команду компиляции без проблемных флагов
echo ""
echo "🔨 Компиляция с минимальными флагами..."

gcc -std=c99 -Os -DNDEBUG \
    -fvisibility=hidden \
    -DLICENSECORE_OBFUSCATED=1 \
    -DLICENSECORE_NO_STRINGS=1 \
    -DANTI_DEBUG_BUILD \
    -Iinclude \
    -Isrc \
    -c src/license_core_pure_c.c \
    -o src/license_core_pure_c.o

if [ $? -eq 0 ]; then
    echo "✅ Успешная компиляция!"
    
    # Создание библиотеки
    echo "📚 Создание статической библиотеки..."
    ar rcs liblicense_core_obf.a src/license_core_pure_c.o
    
    if [ $? -eq 0 ]; then
        echo "✅ Библиотека создана: liblicense_core_obf.a"
        echo "📊 Размер библиотеки:"
        ls -lh liblicense_core_obf.a
        
        echo ""
        echo "🔍 Экспортируемые символы:"
        nm liblicense_core_obf.a | grep " T " | head -10
        
    else
        echo "❌ Ошибка создания библиотеки"
    fi
else
    echo "❌ Ошибка компиляции"
fi
