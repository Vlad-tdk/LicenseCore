# 💻 Примеры использования основной библиотеки LicenseCore++

## 📁 Структура примеров

Все примеры для основной библиотеки находятся в папке `examples/`:
- `simple_example` - базовое использование
- `license_generator` - создание лицензий
- `hwid_tool` - получение Hardware ID

## 🚀 Базовое использование

### Простейший пример

```cpp
#include <iostream>
#include "license_core/hardware_fingerprint.hpp"
#include "license_core/license_manager.hpp"

using namespace license_core;

int main() {
    try {
        // 1. Получение Hardware ID
        HardwareConfig config;
        HardwareFingerprint fingerprint(config);
        std::string hardware_id = fingerprint.get_fingerprint();
        
        std::cout << "Hardware ID: " << hardware_id << std::endl;
        
        // 2. Создание тестовой лицензии
        LicenseInfo license;
        license.user_id = "customer-123";
        license.license_id = "lic-" + std::to_string(time(nullptr));
        license.hardware_hash = hardware_id;
        license.expiry = "2025-12-31T23:59:59Z";
        license.features = {"basic", "premium"};
        
        // 3. Проверка лицензии
        LicenseManager manager("your-secret-key");
        bool is_valid = manager.validate_license(license);
        
        if (is_valid) {
            std::cout << "✅ Лицензия валидна!" << std::endl;
            
            if (manager.has_feature(license, "premium")) {
                std::cout << "🌟 Премиум функции доступны!" << std::endl;
            }
        } else {
            std::cout << "❌ Лицензия невалидна!" << std::endl;
        }
        
    } catch (const LicenseCoreException& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

## 🔧 Продвинутое использование

### Кастомная конфигурация оборудования

```cpp
#include "license_core/hardware_fingerprint.hpp"

HardwareConfig create_secure_config() {
    HardwareConfig config;
    
    // Компоненты для отпечатка
    config.enable_cpu_id = true;              
    config.enable_mac_address = true;         
    config.enable_motherboard_serial = false; // Отключено для совместимости
    config.enable_disk_serial = true;         
    config.enable_bios_info = false;          // Отключено для стабильности
    
    // Настройки кэширования
    config.enable_caching = true;             
    config.cache_lifetime = std::chrono::minutes(30);
    config.thread_safe = true;                
    
    return config;
}

void hardware_example() {
    try {
        HardwareConfig config = create_secure_config();
        HardwareFingerprint fingerprint(config);
        
        // Получить полный отпечаток
        std::string hardware_id = fingerprint.get_fingerprint();
        std::cout << "Full Hardware ID: " << hardware_id << std::endl;
        
        // Получить отдельные компоненты
        std::string cpu_id = fingerprint.get_cpu_id();
        std::string mac_addr = fingerprint.get_mac_address();
        
        std::cout << "CPU ID: " << cpu_id << std::endl;
        std::cout << "MAC Address: " << mac_addr << std::endl;
        
        // Безопасные версии (не выбрасывают исключения)
        std::string safe_hardware_id = fingerprint.get_fingerprint_safe();
        std::cout << "Safe Hardware ID: " << safe_hardware_id << std::endl;
        
    } catch (const HardwareDetectionException& e) {
        std::cerr << "Ошибка оборудования: " << e.what() << std::endl;
        std::cerr << "Код ошибки: " << e.error_code() << std::endl;
    }
}
```

### Работа с лицензиями из файла

```cpp
#include <fstream>
#include <json/json.h> // Требуется JSON библиотека

class LicenseFileHandler {
private:
    LicenseManager manager_;
    
public:
    LicenseFileHandler(const std::string& secret_key) 
        : manager_(secret_key) {}
    
    bool loadLicenseFromFile(const std::string& file_path) {
        try {
            std::ifstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "Не удалось открыть файл: " << file_path << std::endl;
                return false;
            }
            
            Json::Value json_data;
            file >> json_data;
            
            // Парсинг JSON в LicenseInfo
            LicenseInfo license;
            license.user_id = json_data["user_id"].asString();
            license.license_id = json_data["license_id"].asString();
            license.hardware_hash = json_data["hardware_hash"].asString();
            license.expiry = json_data["expiry"].asString();
            license.issued_at = json_data["issued_at"].asString();
            
            for (const auto& feature : json_data["features"]) {
                license.features.push_back(feature.asString());
            }
            
            return manager_.validate_license(license);
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка при загрузке лицензии: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::vector<std::string> getAvailableFeatures(const LicenseInfo& license) {
        std::vector<std::string> available;
        
        if (manager_.validate_license(license)) {
            for (const auto& feature : license.features) {
                if (manager_.has_feature(license, feature)) {
                    available.push_back(feature);
                }
            }
        }
        
        return available;
    }
};
```

### Обработка ошибок

```cpp
void comprehensive_error_handling() {
    try {
        HardwareConfig config;
        HardwareFingerprint fingerprint(config);
        
        std::string hardware_id = fingerprint.get_fingerprint();
        
    } catch (const HardwareDetectionException& e) {
        // Специфичные ошибки оборудования
        std::cerr << "Ошибка определения оборудования: " << e.what() << std::endl;
        std::cerr << "Код ошибки: " << e.error_code() << std::endl;
        
        // Обработка разных кодов ошибок
        switch (e.error_code()) {
            case 1001:
                std::cerr << "Проблема с доступом к CPU ID" << std::endl;
                break;
            case 1002:
                std::cerr << "Проблема с получением MAC адреса" << std::endl;
                break;
            default:
                std::cerr << "Неизвестная ошибка оборудования" << std::endl;
        }
        
    } catch (const LicenseValidationException& e) {
        // Ошибки валидации лицензий
        std::cerr << "Ошибка валидации лицензии: " << e.what() << std::endl;
        
    } catch (const LicenseParsingException& e) {
        // Ошибки парсинга JSON
        std::cerr << "Ошибка парсинга лицензии: " << e.what() << std::endl;
        
    } catch (const LicenseCoreException& e) {
        // Базовый класс - все остальные ошибки библиотеки
        std::cerr << "Общая ошибка LicenseCore: " << e.what() << std::endl;
        
    } catch (const std::exception& e) {
        // Стандартные исключения C++
        std::cerr << "Стандартная ошибка: " << e.what() << std::endl;
    }
}
```

### Многопоточное использование

```cpp
#include <thread>
#include <vector>
#include <mutex>

class ThreadSafeLicenseManager {
private:
    HardwareFingerprint fingerprint_;
    LicenseManager license_manager_;
    mutable std::mutex results_mutex_;
    std::vector<bool> validation_results_;
    
public:
    ThreadSafeLicenseManager(const std::string& secret_key) 
        : fingerprint_(HardwareConfig{.thread_safe = true}),
          license_manager_(secret_key) {}
    
    void validateLicenseAsync(const LicenseInfo& license, size_t thread_id) {
        bool result = false;
        
        try {
            result = license_manager_.validate_license(license);
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка в потоке " << thread_id << ": " << e.what() << std::endl;
        }
        
        // Потокобезопасное сохранение результата
        std::lock_guard<std::mutex> lock(results_mutex_);
        if (validation_results_.size() <= thread_id) {
            validation_results_.resize(thread_id + 1);
        }
        validation_results_[thread_id] = result;
    }
    
    void runConcurrentValidation(const std::vector<LicenseInfo>& licenses) {
        std::vector<std::thread> threads;
        
        for (size_t i = 0; i < licenses.size(); ++i) {
            threads.emplace_back(&ThreadSafeLicenseManager::validateLicenseAsync,
                               this, std::ref(licenses[i]), i);
        }
        
        // Ожидаем завершения всех потоков
        for (auto& thread : threads) {
            thread.join();
        }
        
        // Выводим результаты
        std::lock_guard<std::mutex> lock(results_mutex_);
        for (size_t i = 0; i < validation_results_.size(); ++i) {
            std::cout << "Лицензия " << i << ": " 
                      << (validation_results_[i] ? "✅ Валидна" : "❌ Невалидна") 
                      << std::endl;
        }
    }
};
```

### Создание собственного генератора лицензий

```cpp
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

class LicenseGenerator {
private:
    std::string secret_key_;
    
public:
    LicenseGenerator(const std::string& secret_key) : secret_key_(secret_key) {}
    
    LicenseInfo generateLicense(const std::string& customer_id,
                               const std::string& hardware_id,
                               const std::vector<std::string>& features,
                               int validity_days = 365) {
        LicenseInfo license;
        
        license.user_id = customer_id;
        license.license_id = "lic-" + std::to_string(std::time(nullptr));
        license.hardware_hash = hardware_id;
        license.features = features;
        license.version = 1;
        
        // Устанавливаем время выдачи
        auto now = std::chrono::system_clock::now();
        license.issued_at = formatDateTime(now);
        
        // Устанавливаем срок действия
        auto expiry_time = now + std::chrono::hours(24 * validity_days);
        license.expiry = formatDateTime(expiry_time);
        
        return license;
    }
    
private:
    std::string formatDateTime(const std::chrono::system_clock::time_point& time_point) {
        auto time_t = std::chrono::system_clock::to_time_t(time_point);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

// Пример использования генератора
void license_generation_example() {
    LicenseGenerator generator("my-secret-key");
    
    // Получаем Hardware ID клиента
    HardwareFingerprint fingerprint;
    std::string customer_hardware = fingerprint.get_fingerprint_safe();
    
    // Генерируем лицензию
    auto license = generator.generateLicense(
        "enterprise-customer-001",
        customer_hardware,
        {"basic", "premium", "enterprise"},
        365  // 1 год
    );
    
    // Проверяем сгенерированную лицензию
    LicenseManager manager("my-secret-key");
    bool is_valid = manager.validate_license(license);
    
    std::cout << "Сгенерированная лицензия: " 
              << (is_valid ? "✅ Валидна" : "❌ Невалидна") << std::endl;
    
    std::cout << "License ID: " << license.license_id << std::endl;
    std::cout << "Срок действия: " << license.expiry << std::endl;
}
```

## 🎯 Практические сценарии

### Desktop приложение с активацией

```cpp
class DesktopAppLicensing {
private:
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    std::unique_ptr<LicenseManager> manager_;
    LicenseInfo current_license_;
    bool is_activated_;
    
public:
    DesktopAppLicensing(const std::string& secret_key) 
        : is_activated_(false) {
        HardwareConfig config;
        config.enable_caching = true;
        config.cache_lifetime = std::chrono::hours(1);
        
        fingerprint_ = std::make_unique<HardwareFingerprint>(config);
        manager_ = std::make_unique<LicenseManager>(secret_key);
    }
    
    std::string getActivationCode() {
        return fingerprint_->get_fingerprint_safe();
    }
    
    bool activateWithLicense(const LicenseInfo& license) {
        try {
            // Проверяем привязку к оборудованию
            std::string current_hardware = fingerprint_->get_fingerprint();
            if (license.hardware_hash != current_hardware) {
                std::cout << "❌ Лицензия не подходит для этого оборудования" << std::endl;
                return false;
            }
            
            // Проверяем валидность
            if (!manager_->validate_license(license)) {
                std::cout << "❌ Лицензия невалидна" << std::endl;
                return false;
            }
            
            // Проверяем срок действия
            if (manager_->is_expired(license)) {
                std::cout << "❌ Срок действия лицензии истёк" << std::endl;
                return false;
            }
            
            current_license_ = license;
            is_activated_ = true;
            
            std::cout << "✅ Приложение активировано успешно!" << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Ошибка активации: " << e.what() << std::endl;
            return false;
        }
    }
    
    bool hasFeature(const std::string& feature) {
        if (!is_activated_) {
            return false;
        }
        return manager_->has_feature(current_license_, feature);
    }
    
    void showFeatureStatus() {
        if (!is_activated_) {
            std::cout << "❌ Приложение не активировано" << std::endl;
            return;
        }
        
        std::cout << "🎯 Доступные функции:" << std::endl;
        for (const auto& feature : current_license_.features) {
            bool available = hasFeature(feature);
            std::cout << "  " << (available ? "✅" : "❌") << " " << feature << std::endl;
        }
    }
};
```

## 🔍 Отладка и диагностика

### Система логирования

```cpp
#include <fstream>

class LicenseLogger {
private:
    std::ofstream log_file_;
    
public:
    LicenseLogger(const std::string& log_path) : log_file_(log_path, std::ios::app) {}
    
    void logHardwareInfo() {
        HardwareFingerprint fingerprint;
        
        log_file_ << "=== Hardware Information ===" << std::endl;
        log_file_ << "Full ID: " << fingerprint.get_fingerprint_safe() << std::endl;
        log_file_ << "CPU ID: " << fingerprint.get_cpu_id_safe() << std::endl;
        log_file_ << "MAC Address: " << fingerprint.get_mac_address_safe() << std::endl;
        log_file_ << "Timestamp: " << std::time(nullptr) << std::endl;
        log_file_ << std::endl;
    }
    
    void logLicenseValidation(const LicenseInfo& license, bool result) {
        log_file_ << "=== License Validation ===" << std::endl;
        log_file_ << "License ID: " << license.license_id << std::endl;
        log_file_ << "User ID: " << license.user_id << std::endl;
        log_file_ << "Result: " << (result ? "VALID" : "INVALID") << std::endl;
        log_file_ << "Expiry: " << license.expiry << std::endl;
        log_file_ << "Features: ";
        for (const auto& feature : license.features) {
            log_file_ << feature << " ";
        }
        log_file_ << std::endl << std::endl;
    }
};
```

## 📁 Файлы примеров

Смотрите готовые примеры в папке `examples/`:

1. **`simple_example.cpp`** - базовое использование
2. **`license_generator.cpp`** - создание лицензий
3. **`hwid_tool.cpp`** - получение Hardware ID
4. **`error_handling_example.cpp`** - обработка ошибок
5. **`caching_example.cpp`** - работа с кэшированием

Компиляция примеров:
```bash
cd build
./examples/simple_example
./examples/license_generator
./examples/hwid_tool
```
