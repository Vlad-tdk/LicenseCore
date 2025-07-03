# LicenseCore++ Embedded Edition - Полное руководство

## 📦 Обзор

LicenseCore++ Embedded Edition - это легковесная, статическая библиотека для интеграции лицензирования в приложения без внешних зависимостей. Идеально подходит для:

- Встраиваемых систем и IoT устройств
- Настольных приложений с минимальными зависимостями  
- B2B решений с простым лицензированием
- Быстрого прототипирования лицензионной системы

## 🏗️ Архитектура

### Компоненты

```
embedded/
├── liblicense_core.a      # Статическая библиотека
├── include/
│   └── license_core_stub.h # C API заголовок
├── src/                   # Исходные коды (опционально)
├── test/                  # Тестовые примеры
└── example_integration.cpp # Пример интеграции
```

### Диаграмма архитектуры

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Your App      │───▶│ license_core.a  │───▶│  Hardware ID    │
│                 │    │                 │    │                 │
│ lc_validate()   │    │ • HMAC-SHA256   │    │ • CPU ID        │
│ lc_has_feature()│    │ • JSON parsing  │    │ • MAC address   │
│ lc_get_hwid()   │    │ • HW binding    │    │ • Volume serial │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🔧 API Reference

### Основные функции

#### `int lc_validate_license(const char* license_json)`

Проверяет валидность лицензии из JSON строки.

**Параметры:**
- `license_json` - JSON строка с лицензионными данными

**Возвращает:**
- `1` - лицензия валидна
- `0` - лицензия невалидна или ошибка

**Пример:**
```c
const char* license = R"({
    "user": "Company Inc",
    "product": "MyApp",
    "version": "1.0",
    "expiry": "2025-12-31",
    "features": ["basic", "premium"],
    "hwid": "ABC123DEF456",
    "signature": "sha256_hmac_signature"
})";

if (lc_validate_license(license)) {
    printf("License is valid\n");
} else {
    printf("License validation failed\n");
}
```

#### `int lc_has_feature(const char* feature_name)`

Проверяет наличие конкретной фичи в загруженной лицензии.

**Параметры:**
- `feature_name` - название фичи для проверки

**Возвращает:**
- `1` - фича доступна
- `0` - фича недоступна

**Пример:**
```c
if (lc_has_feature("premium")) {
    enable_premium_features();
}

if (lc_has_feature("advanced_reporting")) {
    show_reports_menu();
}
```

#### `const char* lc_get_hwid(void)`

Возвращает уникальный Hardware ID текущей машины.

**Возвращает:**
- Строку с Hardware ID (не освобождать память)
- `NULL` в случае ошибки

**Пример:**
```c
const char* hwid = lc_get_hwid();
if (hwid) {
    printf("Hardware ID: %s\n", hwid);
}
```

## 📥 Установка и интеграция

### Шаг 1: Подготовка файлов

```bash
# Скопируйте необходимые файлы в ваш проект
cp embedded/liblicense_core.a your_project/lib/
cp embedded/include/license_core_stub.h your_project/include/
```

### Шаг 2: CMake интеграция

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(YourApp)

# Добавляем путь к заголовкам
include_directories(include)

# Добавляем библиотеку
add_library(license_core STATIC IMPORTED)
set_target_properties(license_core PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/liblicense_core.a
)

# Создаем исполняемый файл
add_executable(your_app main.cpp)

# Линкуем с библиотекой
target_link_libraries(your_app license_core)
```

### Шаг 3: Makefile интеграция

```makefile
# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Iinclude
LDFLAGS = -Llib -llicense_core

SOURCES = main.cpp
TARGET = your_app

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
```

### Шаг 4: Прямая компиляция

```bash
# GCC/Clang
g++ -std=c++17 -O2 -Iinclude main.cpp -Llib -llicense_core -o your_app

# MSVC
cl /std:c++17 /O2 /Iinclude main.cpp /link lib/license_core.lib
```

## 💻 Примеры использования

### Базовая интеграция

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include "license_core_stub.h"

int main() {
    // Загружаем лицензию из файла
    std::ifstream file("license.json");
    std::string license_content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
    
    // Проверяем лицензию
    if (!lc_validate_license(license_content.c_str())) {
        std::cerr << "Invalid license!" << std::endl;
        return 1;
    }
    
    std::cout << "License valid!" << std::endl;
    
    // Проверяем доступные фичи
    if (lc_has_feature("premium")) {
        std::cout << "Premium features enabled" << std::endl;
    }
    
    if (lc_has_feature("export")) {
        std::cout << "Export functionality available" << std::endl;
    }
    
    // Получаем Hardware ID
    const char* hwid = lc_get_hwid();
    if (hwid) {
        std::cout << "Hardware ID: " << hwid << std::endl;
    }
    
    return 0;
}
```

### Интеграция с классом приложения

```cpp
class LicensedApplication {
private:
    bool license_valid_;
    std::vector<std::string> available_features_;
    
public:
    bool initialize(const std::string& license_path) {
        // Загружаем лицензию
        std::ifstream file(license_path);
        if (!file.is_open()) {
            return false;
        }
        
        std::string license_content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        
        // Проверяем лицензию
        license_valid_ = lc_validate_license(license_content.c_str());
        
        if (license_valid_) {
            // Определяем доступные фичи
            const char* features[] = {"basic", "premium", "export", "import", "api"};
            for (const char* feature : features) {
                if (lc_has_feature(feature)) {
                    available_features_.push_back(feature);
                }
            }
        }
        
        return license_valid_;
    }
    
    bool is_feature_enabled(const std::string& feature) const {
        return std::find(available_features_.begin(), 
                        available_features_.end(), 
                        feature) != available_features_.end();
    }
    
    void run() {
        if (!license_valid_) {
            std::cout << "Application requires valid license" << std::endl;
            return;
        }
        
        std::cout << "Starting licensed application..." << std::endl;
        
        if (is_feature_enabled("premium")) {
            enable_premium_ui();
        }
        
        if (is_feature_enabled("export")) {
            enable_export_menu();
        }
        
        main_loop();
    }
};
```

### Встроенная лицензия

```cpp
// Встраиваем лицензию прямо в код
const char* embedded_license = R"({
    "user": "OEM Partner",
    "product": "EmbeddedApp",
    "version": "1.0",
    "expiry": "2025-12-31",
    "features": ["basic", "embedded"],
    "hwid": "*",
    "signature": "embedded_signature_here"
})";

int main() {
    // Проверяем встроенную лицензию
    if (lc_validate_license(embedded_license)) {
        std::cout << "Embedded license is valid" << std::endl;
        
        // Запускаем приложение
        run_application();
    } else {
        std::cout << "Embedded license validation failed" << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔒 Безопасность

### Уровни защиты

#### Базовая защита (включена по умолчанию):
- ✅ HMAC-SHA256 подпись лицензий
- ✅ Hardware binding по CPU ID, MAC адресу, серийному номеру диска
- ✅ Скрытие символов (`-fvisibility=hidden`)
- ✅ Удаление отладочной информации

#### Дополнительная защита (рекомендуется):
- 🔧 Обфускация строк с ключами
- 🔧 Переименование API функций
- 🔧 UPX компрессия
- 🔧 Wrapper с anti-debug защитой

### Рекомендации по безопасности

#### 1. Обфускация secret key

```cpp
// Плохо - ключ виден в дизассемблере
const char* secret_key = "my_secret_key_123";

// Хорошо - простая XOR обфускация
const char* get_secret_key() {
    static char key[] = {0x6D^0xAA, 0x79^0xAA, 0x5F^0xAA, 0x73^0xAA, 0x65^0xAA, 
                        0x63^0xAA, 0x72^0xAA, 0x65^0xAA, 0x74^0xAA, 0x5F^0xAA,
                        0x6B^0xAA, 0x65^0xAA, 0x79^0xAA, 0x5F^0xAA, 0x31^0xAA,
                        0x32^0xAA, 0x33^0xAA, 0x00^0xAA};
    
    static bool decrypted = false;
    if (!decrypted) {
        for (int i = 0; key[i]; i++) {
            key[i] ^= 0xAA;
        }
        decrypted = true;
    }
    return key;
}
```

#### 2. Переименование API функций

```cpp
// Добавляем алиасы с менее очевидными именами
extern "C" {
    int app_check_auth(const char* data) {
        return lc_validate_license(data);
    }
    
    int app_has_capability(const char* cap) {
        return lc_has_feature(cap);
    }
    
    const char* app_get_machine_id(void) {
        return lc_get_hwid();
    }
}
```

#### 3. Защита от отладки

```cpp
bool is_debugger_present() {
#ifdef _WIN32
    return IsDebuggerPresent();
#elif defined(__linux__)
    char buf[4096];
    const int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1) return false;
    
    const ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
    close(status_fd);
    
    if (num_read <= 0) return false;
    buf[num_read] = '\0';
    
    return strstr(buf, "TracerPid:\t0") == nullptr;
#else
    return false;
#endif
}

int main() {
    if (is_debugger_present()) {
        std::cout << "Debugging not allowed" << std::endl;
        return 1;
    }
    
    // Остальной код...
}
```

## 🚀 Производительность

### Характеристики

| Метрика | Значение |
|---------|----------|
| Размер библиотеки | ~150KB |
| Время валидации | <1ms |
| Использование памяти | <10KB |
| Зависимости | Только C++17 stdlib |
| Thread safety | ❌ (добавить mutex при необходимости) |

### Оптимизация размера

```bash
# Удаление отладочной информации
strip --strip-unneeded liblicense_core.a

# Удаление неиспользуемых секций
g++ -ffunction-sections -fdata-sections -Wl,--gc-sections

# UPX компрессия (для готового приложения)
upx --best your_app
```

### Thread Safety

Библиотека не является thread-safe по умолчанию. Для многопоточного использования добавьте синхронизацию:

```cpp
#include <mutex>

class ThreadSafeLicense {
private:
    mutable std::mutex mutex_;
    
public:
    bool validate_license(const char* license_json) {
        std::lock_guard<std::mutex> lock(mutex_);
        return lc_validate_license(license_json);
    }
    
    bool has_feature(const char* feature_name) {
        std::lock_guard<std::mutex> lock(mutex_);
        return lc_has_feature(feature_name);
    }
    
    std::string get_hwid() {
        std::lock_guard<std::mutex> lock(mutex_);
        const char* hwid = lc_get_hwid();
        return hwid ? std::string(hwid) : std::string();
    }
};
```

## 🔧 Сборка из исходников

### Требования

- **Компилятор:** GCC 9+, Clang 10+, MSVC 2019+
- **Стандарт:** C++17
- **Система сборки:** CMake 3.16+ или Make

### Процесс сборки

```bash
# Переходим в папку embedded
cd embedded

# Сборка через Make
make clean
make

# Или через CMake
mkdir build && cd build
cmake ..
make

# Проверка сборки
./test/test_embedded
```

### Настройки компиляции

```makefile
# Makefile настройки
CXX = g++
CXXFLAGS = -std=c++17 -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections -static-libgcc -static-libstdc++

# Для максимальной безопасности
SECURITY_FLAGS = -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security
```

## 🧪 Тестирование

### Запуск тестов

```bash
cd embedded/test
./run_tests.sh
```

### Пример тестов

```cpp
#include <cassert>
#include "license_core_stub.h"

void test_valid_license() {
    const char* valid_license = R"({
        "user": "Test User",
        "product": "TestApp",
        "version": "1.0",
        "expiry": "2025-12-31",
        "features": ["basic", "test"],
        "hwid": "*",
        "signature": "valid_signature"
    })";
    
    assert(lc_validate_license(valid_license) == 1);
    assert(lc_has_feature("basic") == 1);
    assert(lc_has_feature("test") == 1);
    assert(lc_has_feature("nonexistent") == 0);
}

void test_hwid() {
    const char* hwid = lc_get_hwid();
    assert(hwid != nullptr);
    assert(strlen(hwid) > 0);
}

int main() {
    test_valid_license();
    test_hwid();
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
```

## 🌍 Поддержка платформ

### Windows
- **Компиляторы:** MSVC 2019+, MinGW
- **Архитектуры:** x86, x64
- **Особенности:** Поддержка WMI для Hardware ID

### macOS
- **Компиляторы:** Xcode 12+, Clang
- **Архитектуры:** Intel x64, Apple Silicon (ARM64)
- **Особенности:** Универсальные бинарники

### Linux
- **Компиляторы:** GCC 9+, Clang 10+
- **Архитектуры:** x86, x64, ARM, ARM64
- **Дистрибутивы:** Ubuntu 20.04+, CentOS 8+, Alpine Linux

### Встраиваемые системы
- **Raspberry Pi** (ARM)
- **BeagleBone** (ARM)
- **Intel Edison** (x86)

## 📚 FAQ

### Q: Можно ли использовать библиотеку в коммерческих проектах?
A: Да, библиотека предназначена для коммерческого использования.

### Q: Нужны ли дополнительные лицензии для распространения?
A: Нет, статическая библиотека встраивается в ваше приложение.

### Q: Поддерживается ли online валидация лицензий?
A: В Embedded Edition только offline валидация. Online валидация доступна в Enterprise версии.

### Q: Можно ли изменить алгоритм Hardware ID?
A: Да, исходные коды позволяют кастомизировать алгоритм.

### Q: Что делать если Hardware ID изменился?
A: Используйте wildcard "*" в поле hwid или реализуйте tolerance к изменениям.

## 📞 Поддержка

- **Email:** support@licensecore.tech
- **Документация:** https://docs.licensecore.tech
- **Примеры:** https://github.com/licensecore/examples
- **Форум:** https://forum.licensecore.tech

## 📄 Лицензия

© 2024 LicenseCore Technologies
Licensed under Commercial License Agreement
