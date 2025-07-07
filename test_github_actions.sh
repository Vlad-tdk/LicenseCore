#!/bin/bash
# Локальное тестирование GitHub Actions с помощью act
# Установка: brew install act

echo "🧪 Тестирование GitHub Actions локально"
echo "======================================"

if ! command -v act &> /dev/null; then
    echo "⚠️  'act' не установлен. Установите его:"
    echo "brew install act"
    echo ""
    echo "🔄 Альтернативно - запустите тест вручную:"
    echo "./test_local_build.sh"
    exit 1
fi

echo "🎭 Тестируем workflow локально..."

# Тестируем только Linux сборку (быстрее)
act -j build \
    --matrix platform:linux-x64 \
    --verbose \
    --pull=false

echo ""
echo "📊 Результаты теста:"
if [ -d ".act_output" ]; then
    echo "✅ Локальный тест завершен"
    echo "📁 Проверьте .act_output/ для артефактов"
else
    echo "ℹ️  Запустите полный тест на GitHub"
fi

echo ""
echo "🚀 Для запуска на GitHub:"
echo "1. git add ."
echo "2. git commit -m 'Add CI/CD workflows'"
echo "3. git push"
echo "4. Создайте тег: git tag v1.0.0 && git push --tags"
