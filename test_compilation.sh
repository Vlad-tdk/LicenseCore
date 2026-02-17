#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"
cd "$REPO_ROOT/obfuscated"

echo "🔧 Тестирование компиляции исправленного файла..."
echo "Текущая директория: $(pwd)"

# Упрощенная команда компиляции для теста
gcc -std=c99 -Os -DNDEBUG \
    -fvisibility=hidden \
    -DLICENSECORE_OBFUSCATED=1 \
    -DLICENSECORE_NO_STRINGS=1 \
    -Wall -Wextra \
    -mmacosx-version-min=10.14 \
    -DANTI_DEBUG_BUILD \
    -Iinclude -Isrc \
    -c src/license_core_pure_c.c \
    -o src/license_core_pure_c.o

if [ $? -eq 0 ]; then
    echo "✅ Компиляция успешна!"
    echo "📊 Информация о объектном файле:"
    ls -la src/license_core_pure_c.o
    echo ""
    echo "🔍 Символы в объектном файле:"
    nm src/license_core_pure_c.o | head -10
else
    echo "❌ Ошибка компиляции"
fi
