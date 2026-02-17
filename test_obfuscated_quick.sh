#!/bin/bash

echo "🔨 БЫСТРАЯ ПРОВЕРКА OBFUSCATED СБОРКИ"
echo "===================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
cd "$REPO_ROOT/obfuscated"

# Clean build
echo "🧹 Очистка предыдущей сборки..."
make clean

# Build
echo "🔧 Сборка obfuscated версии..."
make all

if [ $? -eq 0 ]; then
    echo "✅ Сборка успешна!"
    
    # Check symbols
    echo ""
    echo "🔍 Проверка символов..."
    if [ -f "check_symbols.sh" ]; then
        chmod +x check_symbols.sh
        ./check_symbols.sh liblicense_core.a
    else
        echo "📊 Количество экспортированных символов:"
        nm liblicense_core.a 2>/dev/null | grep ' T ' | wc -l | xargs echo "   Экспортированных функций:"
        echo "📋 Первые 10 экспортированных символов:"
        nm liblicense_core.a 2>/dev/null | grep ' T ' | head -10
    fi
    
    echo ""
    echo "✅ Тест завершён успешно!"
else
    echo "❌ Сборка провалена!"
    exit 1
fi
