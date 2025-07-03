# Hardware Fingerprint Caching System

## Overview

Кэширование в LicenseCore повышает производительность за счет сохранения результатов детекции аппаратных компонентов в памяти на заданное время. Это особенно важно для приложений, которые часто проверяют hardware fingerprint.

## Configuration

### Basic Configuration

```cpp
HardwareConfig config;
config.enable_caching = true;                          // Включить кэширование
config.cache_lifetime = std::chrono::minutes(5);       // Время жизни кэша
config.thread_safe_cache = true;                       // Thread-safe кэш
```

### Cache Lifetime Options

```cpp
// Различные варианты времени жизни кэша
config.cache_lifetime = std::chrono::seconds(30);      // 30 секунд
config.cache_lifetime = std::chrono::minutes(5);       // 5 минут (по умолчанию)
config.cache_lifetime = std::chrono::hours(1);         // 1 час
```

## Usage Examples

### 1. Basic Caching

```cpp
HardwareConfig config;
config.enable_caching = true;
config.cache_lifetime = std::chrono::minutes(5);

HardwareFingerprint fingerprint(config);

// Первый вызов - cache miss (медленно)
std::string fp1 = fingerprint.get_fingerprint();

// Последующие вызовы - cache hit (быстро)
std::string fp2 = fingerprint.get_fingerprint();
std::string fp3 = fingerprint.get_fingerprint();
```

### 2. Performance Comparison

```cpp
auto measure_time = [](auto func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start);
};

// С кэшированием
HardwareConfig cached_config;
cached_config.enable_caching = true;
HardwareFingerprint cached_fp(cached_config);

// Без кэширования
HardwareConfig no_cache_config;
no_cache_config.enable_caching = false;
HardwareFingerprint no_cache_fp(no_cache_config);

// Первый вызов (одинаковое время)
auto time1 = measure_time([&]() { cached_fp.get_fingerprint(); });
auto time2 = measure_time([&]() { no_cache_fp.get_fingerprint(); });

// Второй вызов (кэш работает только для cached_fp)
auto time3 = measure_time([&]() { cached_fp.get_fingerprint(); });  // Быстро
auto time4 = measure_time([&]() { no_cache_fp.get_fingerprint(); }); // Медленно
```

### 3. Cache Statistics

```cpp
HardwareFingerprint fingerprint(config);

// Выполняем несколько вызовов
fingerprint.get_fingerprint();
fingerprint.get_fingerprint();
fingerprint.get_fingerprint();

// Проверяем статистику
auto stats = fingerprint.get_cache_stats();
std::cout << "Cache hits: " << stats.cache_hits << std::endl;
std::cout << "Cache misses: " << stats.cache_misses << std::endl;
std::cout << "Hit rate: " << (stats.hit_rate() * 100) << "%" << std::endl;
```

### 4. Cache Management

```cpp
HardwareFingerprint fingerprint(config);

// Проверка валидности кэша
if (fingerprint.is_cache_valid()) {
    std::cout << "Cache is valid" << std::endl;
}

// Принудительная очистка кэша
fingerprint.clear_cache();

// Или инвалидация (то же самое)
fingerprint.invalidate_cache();

// Время последнего обновления кэша
auto cache_time = fingerprint.get_cache_time();
```

## Thread Safety

### Thread-Safe Mode (Default)

```cpp
HardwareConfig config;
config.thread_safe_cache = true; // По умолчанию

HardwareFingerprint fingerprint(config);

// Безопасно использовать из разных потоков
std::thread t1([&]() { fingerprint.get_fingerprint(); });
std::thread t2([&]() { fingerprint.get_fingerprint(); });
t1.join();
t2.join();
```

### Non-Thread-Safe Mode (Faster)

```cpp
HardwareConfig config;
config.thread_safe_cache = false; // Быстрее, но не thread-safe

HardwareFingerprint fingerprint(config);

// Использовать только из одного потока!
std::string fp = fingerprint.get_fingerprint();
```

## Cache Expiration

### Automatic Expiration

```cpp
HardwareConfig config;
config.cache_lifetime = std::chrono::seconds(2);

HardwareFingerprint fingerprint(config);

// Первый вызов
std::string fp1 = fingerprint.get_fingerprint(); // Cache miss

// Второй вызов (в пределах времени жизни)
std::string fp2 = fingerprint.get_fingerprint(); // Cache hit

// Ждем истечения времени жизни кэша
std::this_thread::sleep_for(std::chrono::seconds(3));

// Третий вызов (кэш истек)
std::string fp3 = fingerprint.get_fingerprint(); // Cache miss
```

### Manual Cache Control

```cpp
HardwareFingerprint fingerprint(config);

// Построить кэш
fingerprint.get_fingerprint();

// Проверить валидность
if (fingerprint.is_cache_valid()) {
    std::cout << "Cache is still valid" << std::endl;
}

// Принудительно очистить
fingerprint.clear_cache();

// Проверить снова
if (!fingerprint.is_cache_valid()) {
    std::cout << "Cache was cleared" << std::endl;
}
```

## Performance Benefits

### Typical Performance Gains

- **Первый вызов**: 1000-5000 μs (зависит от системы)
- **Кэшированные вызовы**: 1-10 μs
- **Ускорение**: 100-1000x для кэшированных вызовов

### When to Use Caching

**Используйте кэширование когда:**
- Приложение часто проверяет hardware fingerprint
- Производительность критична
- Hardware fingerprint не изменяется часто

**Отключите кэширование когда:**
- Нужна гарантия свежих данных при каждом вызове
- Memory usage критичен
- Hardware может изменяться динамически

## Configuration Examples

### High-Frequency Checks

```cpp
// Для приложений с частыми проверками
HardwareConfig config;
config.enable_caching = true;
config.cache_lifetime = std::chrono::minutes(30);
config.thread_safe_cache = true;
```

### Security-Critical Applications

```cpp
// Для security-critical приложений
HardwareConfig config;
config.enable_caching = true;
config.cache_lifetime = std::chrono::seconds(30); // Короткое время жизни
config.thread_safe_cache = true;
```

### Single-Threaded Performance

```cpp
// Для максимальной производительности (один поток)
HardwareConfig config;
config.enable_caching = true;
config.cache_lifetime = std::chrono::minutes(10);
config.thread_safe_cache = false; // Отключить мьютексы
```

### Real-Time Applications

```cpp
// Для real-time приложений
HardwareConfig config;
config.enable_caching = false; // Отключить кэширование для предсказуемости
```

## Best Practices

1. **Выбор времени жизни кэша**: Баланс между производительностью и актуальностью данных
2. **Thread safety**: Включайте только если нужно (overhead ~10-20%)
3. **Мониторинг hit rate**: Стремитесь к >90% для эффективного кэширования
4. **Очистка кэша**: При изменениях в hardware конфигурации
5. **Memory usage**: Кэш занимает ~1KB на экземпляр

## Error Handling with Caching

```cpp
try {
    HardwareFingerprint fingerprint(config);
    std::string fp = fingerprint.get_fingerprint();
    
    // Кэш автоматически сохраняет успешные результаты
    // При ошибках кэш не обновляется
    
} catch (const HardwareDetectionException& e) {
    // Ошибка не кэшируется - следующий вызов попробует снова
    std::cerr << "Hardware detection failed: " << e.what() << std::endl;
}
```

Кэширование в LicenseCore обеспечивает значительное улучшение производительности при сохранении надежности и безопасности системы лицензирования.
