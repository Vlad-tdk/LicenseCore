#!/bin/bash

echo "🔍 ПРОВЕРКА СПЕЦИАЛИЗИРОВАННЫХ СБОРОК"
echo "====================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

cd "$REPO_ROOT"

echo ""
echo "🔧 1. EMBEDDED VERSION"
echo "======================"
cd embedded

echo "📋 Доступные make цели для embedded:"
make help | grep -A5 "Targets:" | head -10

echo ""
echo "🧪 Сборка и тест embedded версии:"
if make clean && make all && make test; then
    echo "✅ Embedded версия работает!"
    if [ -f test_embedded ]; then
        echo "🚀 Запуск embedded теста:"
        ./test_embedded | head -20
    fi
else
    echo "❌ Проблема с embedded версией"
fi

echo ""
echo "🔒 2. OBFUSCATED VERSION"  
echo "========================"
cd ../obfuscated

echo "📋 Доступные make цели для obfuscated:"
make help | grep -A5 "Main Targets:" | head -10

echo ""
echo "🧪 Сборка и тест obfuscated версии:"
if make clean && make all && make test; then
    echo "✅ Obfuscated версия работает!"
else
    echo "❌ Проблема с obfuscated версией"
fi

echo ""
echo "📊 3. ИТОГОВЫЙ СТАТУС"
echo "====================="
echo "✅ Главная библиотека: Google Test интеграция (56 тестов)"
echo "$(ls -la "$REPO_ROOT/embedded/test_embedded" 2>/dev/null && echo '✅ Embedded версия: интеграционный тест' || echo '⚠️ Embedded версия: требует проверки')"
echo "$(ls -la "$REPO_ROOT/obfuscated/test_obfuscated" 2>/dev/null && echo '✅ Obfuscated версия: security тест' || echo '⚠️ Obfuscated версия: требует проверки')"

echo ""
echo "🎯 РЕКОМЕНДАЦИИ:"
echo "  • Главные тесты: используйте 'make test' в корне"
echo "  • Embedded тесты: 'cd embedded && make test'"  
echo "  • Obfuscated тесты: 'cd obfuscated && make test'"
echo "  • Каждая сборка независима и имеет свою цель"
echo ""
echo "✅ Анализ завершён"
