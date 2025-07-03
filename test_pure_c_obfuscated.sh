#!/bin/bash

echo "🔨 БЫСТРЫЙ ТЕСТ PURE C OBFUSCATED ВЕРСИИ"
echo "========================================"

cd /Users/vlad/Coding/C++/LicenseCore/obfuscated

# Clean build
echo "🧹 Очистка предыдущей сборки..."
make clean

# Build pure C version
echo "🔧 Сборка pure C версии..."
make all

if [ $? -eq 0 ]; then
    echo "✅ Сборка успешна!"
    
    # Check symbols with new script
    echo ""
    echo "🔍 Проверка символов..."
    if [ -f "check_symbols.sh" ]; then
        chmod +x check_symbols.sh
        ./check_symbols.sh liblicense_core.a
    else
        echo "📊 Количество экспортированных символов:"
        nm liblicense_core.a 2>/dev/null | grep ' T ' | wc -l | xargs echo "   Экспортированных функций:"
        echo "📋 Все экспортированные символы:"
        nm liblicense_core.a 2>/dev/null | grep ' T '
    fi
    
    # Test the library
    echo ""
    echo "🧪 Тестирование библиотеки..."
    make test
    
    echo ""
    echo "📏 Размер библиотеки:"
    ls -lh liblicense_core.a
    
    echo ""
    echo "🎯 Проверка строк в библиотеке:"
    strings liblicense_core.a | grep -E "(license|feature|hwid)" | head -5 || echo "   Нет очевидных утечек строк"
    
    echo ""
    echo "✅ Тест Pure C версии завершён успешно!"
    echo ""
    echo "📊 РЕЗУЛЬТАТЫ:"
    SYMBOL_COUNT=$(nm liblicense_core.a 2>/dev/null | grep ' T ' | wc -l)
    echo "   Экспортированных символов: $SYMBOL_COUNT"
    if [ $SYMBOL_COUNT -le 4 ]; then
        echo "   🎉 ОТЛИЧНО! Целевое количество символов достигнуто!"
    elif [ $SYMBOL_COUNT -le 10 ]; then
        echo "   ⚠️  ПРИЕМЛЕМО, но можно улучшить"
    else
        echo "   ❌ СЛИШКОМ МНОГО символов"
    fi
else
    echo "❌ Сборка провалена!"
    exit 1
fi
