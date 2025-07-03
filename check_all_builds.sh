#!/bin/bash

echo "🎯 АВТОМАТИЧЕСКАЯ ПРОВЕРКА ВСЕХ СБОРОК LICENSECORE++"
echo "=================================================="

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функция для красивого вывода
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ $2${NC}"
    else
        echo -e "${RED}❌ $2${NC}"
    fi
}

print_header() {
    echo -e "\n${BLUE}$1${NC}"
    echo "=========================="
}

# Проверка зависимостей
print_header "🔍 ПРОВЕРКА ЗАВИСИМОСТЕЙ"

cmake --version > /dev/null 2>&1
print_status $? "CMake доступен"

make --version > /dev/null 2>&1  
print_status $? "Make доступен"

gcc --version > /dev/null 2>&1 || clang --version > /dev/null 2>&1
print_status $? "Компилятор доступен (GCC/Clang)"

pkg-config --exists openssl 2>/dev/null || brew list openssl > /dev/null 2>&1
print_status $? "OpenSSL доступен"

# Переменные для подсчёта
main_result=1
embedded_result=1
obfuscated_result=1

# 1. ОСНОВНАЯ БИБЛИОТЕКА
print_header "🏗️ СБОРКА 1: ОСНОВНАЯ БИБЛИОТЕКА"

echo "📋 Запуск автоматической сборки..."
if ./build.sh > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Сборка основной библиотеки успешна${NC}"
    
    # Проверка тестов
    cd build
    if make test > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Все тесты прошли${NC}"
        main_result=0
    else
        echo -e "${RED}❌ Некоторые тесты упали${NC}"
        echo "💡 Запустите: cd build && make test"
    fi
    cd ..
    
    # Проверка примеров
    if [ -f "build/examples/simple_example" ]; then
        echo -e "${GREEN}✅ Примеры собраны${NC}"
    else
        echo -e "${YELLOW}⚠️ Примеры не найдены${NC}"
    fi
    
else
    echo -e "${RED}❌ Ошибка сборки основной библиотеки${NC}"
    echo "💡 Запустите: ./build.sh"
fi

# 2. EMBEDDED ВЕРСИЯ  
print_header "🔧 СБОРКА 2: EMBEDDED ВЕРСИЯ"

cd embedded
echo "📋 Сборка embedded версии..."

if make clean > /dev/null 2>&1 && make all > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Embedded библиотека собрана${NC}"
    
    # Тест embedded
    if make test > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Embedded тест прошёл${NC}"
        embedded_result=0
    else
        echo -e "${RED}❌ Embedded тест упал${NC}"
        echo "💡 Запустите: cd embedded && make test"
    fi
    
    # Проверка файлов
    if [ -f "liblicense_core.a" ]; then
        echo -e "${GREEN}✅ Embedded библиотека создана ($(du -h liblicense_core.a | cut -f1))${NC}"
    fi
    
else
    echo -e "${RED}❌ Ошибка сборки embedded версии${NC}"
    echo "💡 Запустите: cd embedded && make clean && make all"
fi

cd ..

# 3. OBFUSCATED ВЕРСИЯ
print_header "🔒 СБОРКА 3: OBFUSCATED ВЕРСИЯ"

cd obfuscated  
echo "📋 Сборка obfuscated версии..."

if make clean > /dev/null 2>&1 && make obfuscated > /dev/null 2>&1; then
    echo -e "${GREEN}✅ Obfuscated библиотека собрана${NC}"
    
    # Тест obfuscated
    if make test > /dev/null 2>&1; then
        echo -e "${GREEN}✅ Obfuscated тест прошёл${NC}"
        obfuscated_result=0
    else
        echo -e "${RED}❌ Obfuscated тест упал${NC}"
        echo "💡 Запустите: cd obfuscated && make test"
    fi
    
    # Проверка обфускации
    if [ -f "liblicense_core.a" ]; then
        echo -e "${GREEN}✅ Obfuscated библиотека создана ($(du -h liblicense_core.a | cut -f1))${NC}"
        
        # Проверка скрытых символов
        symbols=$(nm --defined-only liblicense_core.a 2>/dev/null | grep -E " T " | wc -l)
        if [ $symbols -lt 5 ]; then
            echo -e "${GREEN}✅ Символы успешно скрыты ($symbols экспортированных)${NC}"
        else
            echo -e "${YELLOW}⚠️ Найдено $symbols экспортированных символов${NC}"
        fi
    fi
    
else
    echo -e "${RED}❌ Ошибка сборки obfuscated версии${NC}"
    echo "💡 Запустите: cd obfuscated && make clean && make obfuscated"
fi

cd ..

# ФИНАЛЬНЫЙ ОТЧЁТ
print_header "📊 ИТОГОВЫЙ ОТЧЁТ"

total_success=$((3 - main_result - embedded_result - obfuscated_result))

echo "📈 Статистика сборок:"
print_status $main_result "Основная библиотека (Google Test интеграция)"
print_status $embedded_result "Embedded версия (C API интеграция)"  
print_status $obfuscated_result "Obfuscated версия (Enterprise защита)"

echo ""
echo "🎯 Общий результат: $total_success/3 сборки успешны"

if [ $total_success -eq 3 ]; then
    echo -e "${GREEN}🎉 ВСЕ СБОРКИ ГОТОВЫ К ПРОДАКШЕНУ!${NC}"
    echo ""
    echo "📦 Готовые файлы:"
    [ -f "build/liblicensecore.a" ] && echo "  ✅ build/liblicensecore.a (основная)"
    [ -f "embedded/liblicense_core.a" ] && echo "  ✅ embedded/liblicense_core.a (C API)"
    [ -f "obfuscated/liblicense_core.a" ] && echo "  ✅ obfuscated/liblicense_core.a (защищённая)"
    
    echo ""
    echo "🚀 Следующие шаги:"
    echo "  • Создайте пакеты: cd embedded && make package"
    echo "  • Создайте enterprise пакет: cd obfuscated && make package"
    echo "  • Интегрируйте в ваши проекты"
    
elif [ $total_success -eq 2 ]; then
    echo -e "${YELLOW}⚠️ Большинство сборок готово, но есть проблемы${NC}"
elif [ $total_success -eq 1 ]; then
    echo -e "${YELLOW}⚠️ Только одна сборка работает корректно${NC}"
else
    echo -e "${RED}❌ Все сборки имеют проблемы${NC}"
fi

echo ""
echo "📚 Документация:"
echo "  • Полная инструкция: PRODUCTION_BUILD_GUIDE.md"
echo "  • Быстрая шпаргалка: QUICK_BUILD_CHEAT_SHEET.md"

echo ""
echo "✅ Проверка завершена"

# Возвращаем код ошибки если не все сборки успешны
if [ $total_success -ne 3 ]; then
    exit 1
fi
