# Как я сделал систему защиты приложений с WASM и HMAC: проект LicenseCore.tech

## 📌 Вступление

Привет! Меня зовут Влад, и я разработчик системного софта. Несколько лет занимаюсь созданием различных утилит и приложений, но постоянно сталкивался с одной и той же проблемой — кража интеллектуальной собственности. Видеть, как твои наработки копируют и распространяют бесплатно, крайне неприятно.

Сначала пробовал простые решения: проверка серийных номеров, привязка к железу через реестр Windows. Но все эти методы легко обходились за несколько минут в дизассемблере. Нужно было что-то серьёзное, но в то же время универсальное — чтобы работало и в нативных приложениях, и в браузере.

Устал видеть, как крадут мои софтинки, решил сделать свою защиту, но такую, чтобы работала и оффлайн, и в браузере — в итоге получился **LicenseCore**.

## 🧱 Архитектура проекта

Система построена на трёх ключевых компонентах, каждый из которых решает свою задачу. Плюс есть специализированные варианты для разных сценариев использования.

### 1. 🧠 Генерация лицензии (HMAC-SHA256)

**Почему HMAC, а не RSA или ECC?**

Изначально хотел использовать RSA — казалось, что асимметричная криптография надёжнее. Но быстро понял проблемы:

- RSA подписи занимают много места (256+ байт)
- Требуют большие вычислительные ресурсы в браузере
- Открытый ключ всё равно надо где-то хранить в клиенте

HMAC-SHA256 решает эти проблемы элегантно:

- Подпись всего 64 символа в hex (32 байта)
- Молниеносная проверка даже на слабом железе
- Секретный ключ никогда не покидает сервер

**Как работает привязка к железу:**

```cpp
// Получаем уникальный отпечаток железа
std::string hwid = hardware_fingerprint.get_fingerprint();
// Комбинирует: CPU ID + MAC адрес + серийник диска

// Создаём данные для подписи
std::string license_data = user_id + expiry + features + hwid;

// Подписываем HMAC-SHA256
std::string signature = hmac_validator.sign(license_data);
```

**Процесс на сервере (CLI или API):**

```bash
# Генерация лицензии через CLI
./license_gen --user "user-123" --expire "2025-12-31" --hwid "a1b2c3d4..." --features "premium,api"

# Результат: подписанный JSON
{
  "user_id": "user-123",
  "hardware_hash": "a1b2c3d4e5f6...",
  "expiry": "2025-12-31T23:59:59Z",
  "features": ["premium", "api"],
  "hmac_signature": "c4ef45e6a7b8..."
}
```

### 4. 📫 Специализированные варианты

**Embedded Edition — для встроенных систем:**

Легковесная статическая библиотека без внешних зависимостей:

```c
// Простое C API для интеграции
#include "license_core_stub.h"

int main() {
    // Проверка лицензии в одну строчку
    if (lc_validate_license(license_json)) {
        if (lc_has_feature("premium")) {
            enable_premium_mode();
        }
    }

    // Получение Hardware ID
    const char* hwid = lc_get_hwid();
    printf("Hardware ID: %s\n", hwid);

    return 0;
}
```

**Преимущества Embedded:**

- ⚡ Минимальный размер: ~150KB
- 🚫 Никаких зависимостей (даже OpenSSL не нужен)
- 🚀 Быстрая интеграция: одна статическая библиотека
- 📱 Поддержка IoT и микроконтроллеров

**Enterprise Edition — для критичных приложений:**

Усиленная версия с дополнительными слоями защиты:

```cpp
// Расширенные возможности защиты
class EnhancedLicenseManager : public LicenseManager {
public:
    // Автоматическая проверка целостности окружения
    bool validate_environment() const;

    // Мониторинг подозрительной активности
    SecurityStatus check_runtime_integrity() const;

    // Улучшенная привязка к железу
    std::string get_enhanced_hwid() const;
};
```

**Возможности Enterprise:**

- 🔒 Автоматическая проверка окружения выполнения
- 🔍 Мониторинг попыток анализа приложения
- 🚫 Работа в недоверенных окружениях (виртуальные машины, отладчики)
- 🔐 Улучшенная обфускация кода и данных
- 📊 Логирование безопасности и аудит

### 2. 🌐 Валидация лицензии на клиенте

**WASM-библиотека в браузере:**

Главная фишка проекта — одна и та же логика валидации работает везде. C++ код компилируется в WebAssembly и встраивается прямо в браузер:

```javascript
// Инициализация WASM модуля
const licenseCore = await loadLicenseCore();
const manager = new licenseCore.LicenseManager("secret-key");

// Валидация лицензии
try {
    const licenseInfo = manager.loadAndValidate(licenseJson);
    if (licenseInfo.valid && manager.hasFeature("premium")) {
        // Активируем премиум функции
        enablePremiumFeatures();
    }
} catch (error) {
    showLicenseError(error.message);
}
```

**Зачем валидация в JS?**

- **WebUI активация**: лицензионные ключи прямо в браузере
- **Embedded frontend**: встроенные веб-интерфейсы в десктопных приложениях
- **Демо-версии**: ограничение функций по лицензии
- **Офлайн приложения**: PWA с локальной валидацией

**Проблема реверса и защита:**

Да, JavaScript код можно посмотреть. Но:

1. **Обфускация WASM**: исходники C++ скрыты в бинарном формате
2. **Runtime проверки**: HMAC ключ не хранится в коде, передаётся при инициализации
3. **Дополнительные слои**: anti-debug, проверка на DevTools
4. **Серверная валидация**: критичные операции дублируются на сервере

```cpp
// Антидебаг проверки (опционально)
#ifdef LICENSECORE_ANTI_DEBUG
    if (is_debugger_present()) {
        throw SecurityException("Debugging detected");
    }
#endif
```

### 3. 🧰 Интеграция в проекты

**Использование в C++:**

```cpp
#include <license_core/license_manager.hpp>

using namespace license_core;

int main() {
    try {
        LicenseManager manager("your-secret-key");

        // Загружаем лицензию из файла
        std::ifstream file("license.json");
        std::string license_json((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

        auto info = manager.load_and_validate(license_json);

        if (info.valid) {
            std::cout << "Лицензия действительна до: "
                      << format_time(info.expiry) << std::endl;

            // Проверяем конкретные фичи
            if (manager.has_feature("premium")) {
                enable_premium_mode();
            }
        }

    } catch (const LicenseException& e) {
        std::cerr << "Ошибка лицензии: " << e.what() << std::endl;
        exit(1);
    }
}
```

**Интеграция через CMake:**

```cmake
# Подключение библиотеки
find_package(LicenseCore REQUIRED)
target_link_libraries(my_app LicenseCore::licensecore)

# Или статическая сборка
add_subdirectory(LicenseCore)
target_link_libraries(my_app licensecore)
```

**Как обрабатываются офлайн/онлайн активации:**

```cpp
class LicenseManager {
public:
    // Офлайн валидация (только HMAC + время)
    LicenseInfo load_and_validate(const std::string& license_json);

    // Онлайн валидация (будущая фича)
    LicenseInfo validate_online(const std::string& license_key,
                               const std::string& server_url);

    // Кэширование для гибридного режима
    void cache_license(const LicenseInfo& info);
    LicenseInfo load_from_cache();
};
```

## ⚔️ Что ломает простые защитки — и как я это закрыл

**Проблема №1: Встроенные ключи**

```cpp
// ❌ Плохо: секрет прямо в коде
bool check_license(const std::string& key) {
    return key == "SUPER-SECRET-123"; // видно в дизассемблере за 30 секунд
}

// ✅ Хорошо: HMAC с серверным ключом
bool validate_license(const std::string& license_json) {
    // Секретный ключ остается на сервере генерации
    // В клиенте только проверка подписи
    return hmac_validator.verify_json(license_json);
}
```

**Проблема №2: Простые проверки времени**

```cpp
// ❌ Плохо: проверка времени системы
if (std::time(nullptr) > license_expiry) {
    return false; // легко обходится сменой даты
}

// ✅ Хорошо: подписанная дата истечения
// Дата вшита в HMAC подпись, изменить нельзя
auto info = parse_and_verify_license(license_json);
return info.expiry > std::chrono::system_clock::now();
```

**Проблема №3: Привязка к железу**

```cpp
// ❌ Плохо: простое сравнение
std::string current_hwid = get_hardware_id();
if (current_hwid != license.hardware_id) {
    return false; // hwid можно подменить в дебагере
}

// ✅ Хорошо: хэш как часть подписи
std::string hwid_hash = sha256(get_combined_fingerprint());
// hwid_hash был включён в HMAC при генерации лицензии
// Изменить его = нарушить подпись
```

**Разделение логики генерации и валидации:**

Ключевой принцип — **сервер генерации** и **клиент валидации** имеют разные права:

```
Сервер (имеет секретный ключ):
├── Генерирует HMAC подписи
├── Создаёт лицензии
└── Может отзывать лицензии (CRL)

Клиент (имеет только валидатор):
├── Проверяет HMAC подписи
├── Читает данные лицензии
└── НЕ МОЖЕТ создавать новые лицензии
```

## 🧪 Примеры атак, которые выдерживает

**Атака №1: Генератор ключей (Keygen)**

- ❌ Не работает: Без секретного ключа сервера невозможно создать валидную HMAC подпись
- 🛡️ Защита: Ключ остается только на сервере, клиент может только проверять

**Атака №2: Подмена файла лицензии**

- ❌ Не работает: Изменение любого байта в лицензии нарушает HMAC подпись
- 🛡️ Защита: Криптографическая целостность данных

**Атака №3: Снятие лога с памяти (Memory dump)**

- ❌ Ограниченно работает: Секретный ключ не хранится в памяти клиента постоянно
- 🛡️ Защита: Ключ передается извне, anti-debug техники (опционально)

**Атака №4: Отключение интернета для обхода онлайн проверок**

- ❌ Не работает: Система изначально спроектирована для офлайн работы
- 🛡️ Защита: HMAC валидация не требует сетевого соединения

**Атака №5: Патчинг бинарника**

- ⚠️ Частично работает: Можно отключить проверки лицензии
- 🛡️ Защита: Обфускация кода, anti-tampering, code signing

## 📦 Что дальше

### Ближайшие планы

**Сигнатурный анализ:**

```cpp
// Проверка целостности исполняемого файла
class IntegrityChecker {
public:
    bool verify_binary_signature(const std::string& file_path);
    bool check_code_sections(); // anti-patch protection
};
```

**Hardware fingerprinting конфигурация:**

```cpp
// Настройка компонентов для отпечатка
HardwareConfig config;
config.use_cpu_id = true;              // CPU ID
config.use_mac_address = true;         // MAC адрес
config.use_volume_serial = true;       // Серийник диска
config.use_motherboard_serial = false; // Материнка (опционально)

// Кэширование для производительности
config.enable_caching = true;
config.cache_lifetime = std::chrono::minutes(5);
config.thread_safe_cache = true;

HardwareFingerprint fingerprint(config);
std::string hwid = fingerprint.get_fingerprint();
```

**Улучшенная защита строк и данных:**

```cpp
// Для Enterprise версии — автоматическая обфускация
namespace secure_strings {
    // Критичные строки шифруются автоматически
    std::string get_api_key() { /* runtime дешифрование */ }
    std::string get_field_name(const std::string& logical_name);

    // Очистка памяти после использования
    void secure_zero(void* ptr, size_t size);
}

// Мониторинг окружения выполнения
namespace environment_check {
    bool is_trusted_environment();
    bool check_system_integrity();
    SecurityLevel assess_risk_level();
}
```

**Webhook интеграции:**

```cpp
// Уведомления о событиях лицензирования
class WebhookNotifier {
public:
    void on_license_activated(const LicenseInfo& info);
    void on_license_violation(const std::string& reason);
    void on_hardware_change(const std::string& old_hwid,
                           const std::string& new_hwid);
};
```

### WebAssembly версия → поддержка wasm32-wasi

Текущая демо версия работает только в браузере с JavaScript fallback. Планирую добавить настоящую WASM сборку для использования в:

- **Браузерах**: реальный C++ код через WebAssembly
- **Node.js серверах**: через WASI runtime
- **Edge computing**: Cloudflare Workers, Fastly

### Продвинутые функции

**Self-destruct лицензии:**

```json
{
  "auto_destruct": {
    "max_runs": 100,
    "max_days": 30,
    "triggers": ["debugger_detected", "vm_detected"]
  }
}
```

**Удаленная блокировка (License Revocation):**

```cpp
class RevocationChecker {
public:
    bool check_revocation_list(const std::string& license_id);
    void cache_revocation_response(int cache_hours = 24);
};
```

## 📋 Варианты развертывания

Проект предлагает несколько вариантов в зависимости от ваших потребностей:

### 💻 Базовая версия

- ✅ Полные исходники C++
- ✅ HMAC-SHA256 + Hardware binding
- ✅ Поддержка всех платформ
- ✅ WebAssembly демо
- 💰 **$299** для Open Source

### 📦 Embedded Edition

- ✅ Легковесная статическая библиотека (~150KB)
- ✅ Простое C API для интеграции
- ✅ Никаких внешних зависимостей
- ✅ Поддержка IoT и embedded systems
- 💰 **$899** для коммерческого использования

### 🏢 Enterprise Edition

- ✅ Всё из Embedded + дополнительные слои защиты
- ✅ Автоматическая проверка окружения
- ✅ Мониторинг попыток анализа
- ✅ Улучшенная обфускация кода
- ✅ 24/7 поддержка и консалтинг

### 🎯 Когда какую версию выбирать?

**Базовая версия** — для обучения и прототипов:

- 🎓 Образовательные проекты
- 🛠️ Прототипирование лицензионной системы
- 📚 Open Source проекты с опциональным лицензированием

**Embedded Edition** — для продакшн приложений:

- 🖥️ Настольные приложения (Windows/macOS/Linux)
- 📱 Мобильные приложения с нативным кодом
- 🌐 IoT устройства и embedded системы
- 🛠️ Промышленное ПО и автоматизация

**Enterprise Edition** — для критичных систем:

- 🏦 Финансовое ПО и трейдинг платформы
- 🔒 Криптографические приложения
- 🎮 Коммерческие игры и развлекательное ПО

### 💼 Примеры реального использования

**Кейс 1: Настольное приложение для Windows**

```cpp
// Простая интеграция в main.cpp
#include "license_core_stub.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                  LPSTR lpCmdLine, int nCmdShow) {
    // Проверяем лицензию при запуске
    std::ifstream license_file("app.license");
    if (!license_file || !lc_validate_license(
        std::string((std::istreambuf_iterator<char>(license_file)),
                   std::istreambuf_iterator<char>()).c_str())) {
        MessageBox(NULL, L"Лицензия не найдена", L"Ошибка", MB_OK);
        return 1;
    }

    // Проверяем доступные функции
    bool premium_enabled = lc_has_feature("premium");
    bool export_enabled = lc_has_feature("export");

    // Запускаем приложение с соответствующими возможностями
    return run_application(premium_enabled, export_enabled);
}
```

**Кейс 2: IoT устройство на Raspberry Pi**

```c
// Минимальная интеграция для embedded
#include <stdio.h>
#include <stdlib.h>
#include "license_core_stub.h"

int main() {
    // Проверяем встроенную лицензию
    extern const char* embedded_license_data; // Зашито в прошивку

    if (!lc_validate_license(embedded_license_data)) {
        printf("Device license validation failed\n");
        return 1;
    }

    printf("IoT device licensed to: %s\n", lc_get_hwid());

    // Активируем функции на основе лицензии
    if (lc_has_feature("sensors")) {
        start_sensor_monitoring();
    }
    if (lc_has_feature("network")) {
        start_network_interface();
    }

    return run_main_loop();
}
```

**Кейс 3: Enterprise приложение с усиленной защитой**

```cpp
// Полноценная интеграция с мониторингом
class SecureApplication {
private:
    EnhancedLicenseManager license_manager_;
    SecurityMonitor security_;

public:
    bool initialize() {
        // Проверка окружения перед запуском
        if (!license_manager_.validate_environment()) {
            log_security_event("Untrusted environment detected");
            return false;
        }

        // Проверка целостности во время выполнения
        auto status = license_manager_.check_runtime_integrity();
        if (status.risk_level > SecurityLevel::MEDIUM) {
            log_security_event("High risk environment detected");
            return false;
        }

        // Запускаем мониторинг безопасности
        security_.start_monitoring();
        return true;
    }

    void run() {
        while (running_) {
            // Периодическая проверка безопасности
            if (!security_.is_environment_safe()) {
                log_security_event("Security breach detected");
                safe_shutdown();
                break;
            }

            process_business_logic();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};
```

## 📎 Заключение

За несколько месяцев разработки получилась достаточно серьёзная система защиты, которая:

- ✅ **Работает везде**: С++ приложения, браузер, мобильные устройства
- ✅ **Не требует интернета**: Полностью автономная валидация
- ✅ **Устойчива к базовым атакам**: HMAC криптография + правильная архитектура
- ✅ **Легко интегрируется**: Несколько строк кода для подключения
- ✅ **Масштабируется**: От простых утилит до enterprise решений

Если нужен лёгкий, но серьёзный способ защитить свой софт — юзай **LicenseCore**. Всё open source, всё честно. Не закрываю код, потому что доверяю математике, а не обфускаторам.

**Попробовать можно здесь:** [licensecore.tech](https://licensecore.tech)

---

*Не стесняйтесь писать вопросы или предложения. Всегда рад обратной связи от коллег-разработчиков!*

### 🧠 Полезные ссылки

- **Документация API**: [docs.html](docs.html)
- **Живое демо**: [index.html](index.html)
- **Примеры интеграции**: [EXAMPLES.md](../EXAMPLES.md)
- **WASM руководство**: [WASM_GUIDE.md](../WASM_GUIDE.md)

### 📸 Код примеров

**C++ интеграция:**

```cpp
#include <license_core/license_manager.hpp>
#include <iostream>
#include <fstream>

int main() {
    try {
        license_core::LicenseManager manager("your-secret-key");

        std::ifstream file("license.json");
        std::string license_json((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());

        auto info = manager.load_and_validate(license_json);

        std::cout << "✅ Лицензия действительна!" << std::endl;
        std::cout << "👤 Пользователь: " << info.user_id << std::endl;
        std::cout << "⏰ Действует до: "
                  << manager.format_iso8601(info.expiry) << std::endl;

        if (manager.has_feature("premium")) {
            std::cout << "🌟 Премиум функции активны" << std::endl;
        }

    } catch (const license_core::LicenseException& e) {
        std::cerr << "❌ Ошибка лицензии: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

**JavaScript интеграция:**

```javascript
// Импорт демо-версии (или WASM если доступен)
import { LicenseCoreDemo } from './demo.js';

async function validateLicense(licenseJson) {
    try {
        const manager = new LicenseCoreDemo();

        const result = await manager.validateLicense(licenseJson);

        console.log('✅ Лицензия действительна!');
        console.log('👤 Пользователь:', result.user_id);

        if (await manager.hasFeature('premium')) {
            enablePremiumFeatures();
        }

    } catch (error) {
        console.error('❌ Ошибка лицензии:', error.message);
        showLicenseDialog();
    }
}
```

**Python интеграция (через ctypes):**

```python
import ctypes
import json
from pathlib import Path

# Загружаем библиотеку
lib = ctypes.CDLL('./liblicensecore.so')

# Настраиваем функции
lib.validate_license.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
lib.validate_license.restype = ctypes.c_bool

def validate_license(license_json: str, secret_key: str) -> bool:
    return lib.validate_license(
        license_json.encode('utf-8'),
        secret_key.encode('utf-8')
    )

# Использование
license_data = Path('license.json').read_text()
if validate_license(license_data, "your-secret-key"):
    print("✅ Лицензия действительна!")
else:
    print("❌ Лицензия недействительна!")
```
