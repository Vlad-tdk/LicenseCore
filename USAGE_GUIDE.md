# 📚 LicenseCore++ - Документация основной библиотеки

## 📋 Обзор

Основная библиотека LicenseCore++ предоставляет полный C++ API для управления лицензиями. Включает классы `HardwareFingerprint`, `LicenseManager`, `HMACValidator` и полную систему исключений.

---

## 🔧 Подключение к проекту

### CMake (рекомендуется):
```cmake
cmake_minimum_required(VERSION 3.16)
project(MyProject)

# Найти LicenseCore++
find_package(LicenseCore REQUIRED)

# Создать исполняемый файл
add_executable(myapp main.cpp)

# Подключить библиотеку
target_link_libraries(myapp LicenseCore::licensecore)

# Установить C++ стандарт
set_property(TARGET myapp PROPERTY CXX_STANDARD 17)
```

### Makefile:
```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I/path/to/licensecore/include
LIBS = -L/path/to/licensecore/lib -llicensecore

# Платформо-зависимые библиотеки
ifeq ($(shell uname -s),Darwin)
    PLATFORM_LIBS = -framework IOKit -framework CoreFoundation
else ifeq ($(shell uname -s),Linux)
    PLATFORM_LIBS = -lpthread
else
    PLATFORM_LIBS = -liphlpapi -lole32 -loleaut32
endif

myapp: main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(PLATFORM_LIBS)
```

---

## 💻 API Reference

### HardwareFingerprint класс

```cpp
#include "license_core/hardware_fingerprint.hpp"
using namespace license_core;
```

#### Конфигурация:
```cpp
struct HardwareConfig {
    bool enable_cpu_id = true;                    // ID процессора
    bool enable_mac_address = true;               // MAC адрес
    bool enable_motherboard_serial = false;       // Серийный номер материнской платы
    bool enable_disk_serial = false;              // Серийный номер диска
    bool enable_bios_info = false;                // Информация BIOS
    bool enable_caching = true;                   // Включить кэширование
    std::chrono::seconds cache_lifetime = std::chrono::seconds(300);  // Время жизни кэша
    bool thread_safe = true;                      // Потокобезопасность
};
```

#### Конструктор:
```cpp
HardwareFingerprint(const HardwareConfig& config = HardwareConfig{});
```

#### Основные методы:
```cpp
// Получение полного отпечатка оборудования
std::string get_fingerprint();           // Может выбросить исключение
std::string get_fingerprint_safe();      // Безопасная версия, не выбрасывает исключения

// Получение отдельных компонентов
std::string get_cpu_id();               // ID процессора
std::string get_cpu_id_safe();          // Безопасная версия
std::string get_mac_address();          // MAC адрес
std::string get_mac_address_safe();     // Безопасная версия

// Управление кэшем
void clear_cache();                      // Очистить кэш
```

### LicenseManager класс

```cpp
#include "license_core/license_manager.hpp"
using namespace license_core;
```

#### Структура лицензии:
```cpp
struct LicenseInfo {
    std::string user_id;                 // ID пользователя
    std::string license_id;              // ID лицензии
    std::string hardware_hash;           // Хэш оборудования
    std::string expiry;                  // Срок действия (ISO 8601)
    std::string issued_at;               // Время выдачи (ISO 8601)
    std::vector<std::string> features;   // Список функций
    int version = 1;                     // Версия формата
};
```

#### Конструктор:
```cpp
LicenseManager(const std::string& secret_key);
```

#### Методы:
```cpp
// Проверка валидности лицензии
bool validate_license(const LicenseInfo& license);

// Проверка наличия функции
bool has_feature(const LicenseInfo& license, const std::string& feature);

// Проверка срока действия
bool is_expired(const LicenseInfo& license);
```

### HMACValidator класс

```cpp
#include "license_core/hmac_validator.hpp"
using namespace license_core;
```

#### Методы:
```cpp
// Создание HMAC подписи
static std::string create_signature(const std::string& data, const std::string& secret);

// Проверка HMAC подписи
static bool verify_signature(const std::string& data, 
                            const std::string& signature, 
                            const std::string& secret);
```

### Система исключений

```cpp
#include "license_core/exceptions.hpp"
using namespace license_core;
```

#### Иерархия исключений:
```cpp
// Базовый класс всех исключений библиотеки
class LicenseCoreException : public std::exception {
public:
    const char* what() const noexcept override;
};

// Ошибки определения оборудования
class HardwareDetectionException : public LicenseCoreException {
public:
    int error_code() const noexcept;
};

// Ошибки валидации лицензий
class LicenseValidationException : public LicenseCoreException {};

// Ошибки парсинга JSON
class LicenseParsingException : public LicenseCoreException {};
```

---

## 🎯 Примеры использования

Подробные примеры кода находятся в файле `EXAMPLES.md` в этой же папке.

---

## 🧪 Тестирование

Для запуска тестов:
```bash
./build.sh
# или
cd build && make test
```

Доступно 56 автоматических тестов, включая:
- Unit тесты (37 тестов)
- Тесты производительности (10 тестов) 
- Тесты многопоточности (8 тестов)

---

## 🛠️ Сборка

Основная сборка осуществляется через:
```bash
./build.sh
```

Результат: `build/liblicensecore.a`

---

## 📋 Системные требования

- **Компилятор**: GCC 7+ или Clang 6+ или MSVC 2019+
- **C++ стандарт**: C++17
- **CMake**: 3.16+
- **Зависимости**: OpenSSL

---

## ⚠️ Важные замечания

1. Используйте `*_safe()` методы в production коде для избежания исключений
2. Настройте `HardwareConfig` в соответствии с требованиями безопасности
3. Храните secret_key в безопасном месте
4. Регулярно очищайте кэш при изменении конфигурации оборудования
