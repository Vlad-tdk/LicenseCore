# 🚀 LicenseCore++ - Быстрая шпаргалка

## Три команды для трёх сборок:

### 🔧 Основная библиотека (разработка):
```bash
./build.sh
# Результат: build/liblicensecore.a + 56 тестов
```

### 🔧 Embedded версия (C API):  
```bash
cd embedded && make clean && make all && make test && make package
# Результат: ../release/licensecore-embedded-YYYYMMDD.tar.gz
```

### 🔒 Obfuscated версия (Enterprise):
```bash
cd obfuscated && make clean && make obfuscated && make test && make package  
# Результат: ../release_obfuscated/licensecore-obfuscated-YYYYMMDD.tar.gz
```

## Быстрая проверка:

```bash
# Основная
./build.sh && echo "✅ Основная: OK" || echo "❌ Основная: FAIL"

# Embedded  
cd embedded && make test && echo "✅ Embedded: OK" || echo "❌ Embedded: FAIL"

# Obfuscated
cd obfuscated && make test && echo "✅ Obfuscated: OK" || echo "❌ Obfuscated: FAIL"
```

## Что отправлять клиентам:

| Клиент | Файл | Когда |
|--------|------|-------|
| Разработчики | `build/liblicensecore.a` | Для интеграции и тестов |
| C-проекты | `release/licensecore-embedded-*.tar.gz` | Простая интеграция |
| Коммерческие | `release_obfuscated/licensecore-obfuscated-*.tar.gz` | Максимальная защита |

## Устранение проблем:

```bash
# Если что-то не работает:
brew install cmake openssl    # macOS
sudo apt install build-essential cmake libssl-dev    # Linux

# Дайте права:
chmod +x build.sh embedded/build.sh obfuscated/build_obfuscated.sh

# Проверьте зависимости:
cmake --version && gcc --version && make --version
```

📚 **Полная инструкция:** `PRODUCTION_BUILD_GUIDE.md`
