# LicenseCore++ Interactive Demo Site

Этот сайт демонстрирует работу LicenseCore++ с real-time валидацией лицензий используя WebAssembly.

## 🚀 Запуск сайта

### Быстрый запуск
```bash
cd docs
python3 -m http.server 8000
# Откройте: http://localhost:8000
```

### Альтернативы
```bash
# Node.js
npx serve .

# PHP
php -S localhost:8000

# Любой другой веб-сервер
```

## 🔧 Обновление WASM модуля

Для обновления WASM версии:

```bash
# Из корня проекта
chmod +x update_docs_wasm.sh
./update_docs_wasm.sh
```

Или вручную:
```bash
cd wasm
./build_wasm.sh
cp build/license_core.* ../docs/
```

## 📁 Структура файлов

- `index.html` - Главная страница с демо
- `docs.html` - Документация 
- `demo_wasm.js` - Hybrid JavaScript/WASM логика
- `license_core.js` - WASM loader (генерируется)
- `license_core.wasm` - WebAssembly модуль (генерируется)
- `style.css` - Стили

## 🔍 Особенности демо

### Автоматическое переключение
- **WASM доступен**: Использует real C++ implementation
- **WASM недоступен**: Fallback на JavaScript simulation

### Индикация статуса
- 🟢 **Real C++ WASM** - Production Implementation  
- 🟡 **JavaScript Demo** - Simulation Mode

### Demo сценарии
- 🖥️ **Hardware Change** - Смена fingerprint
- ⏰ **Expired License** - Тест истёкшей лицензии  
- 🕵️ **Tamper License** - Тест модификации лицензии
- 🏁 **Performance Test** - Сравнение WASM vs JS (только с WASM)

## 🛠️ Разработка

### Требования для WASM
- Emscripten SDK установлен в `~/emsdk`
- `source ~/emsdk/emsdk_env.sh` активирован

### Отладка
```javascript
// В браузере Console (F12)
licenseCore.useWasm              // true/false
licenseCore.wasmModule           // WASM модуль
licenseCore.currentHwid          // Hardware ID
await licenseCore.performanceTest()  // Benchmark
```

### Структура WASM API
```cpp
class LicenseCoreWasm {
    LicenseCoreWasm(string secret_key);
    string getCurrentHwid();
    void generateNewHwid();  
    string generateLicense(string user_id, vector<string> features, int days);
    ValidationResult validateLicense(string json);
};
```

## 🌐 Production Deploy

Для production:

1. **Соберите WASM**: `./update_docs_wasm.sh`
2. **Настройте HTTPS** (WASM требует secure context)
3. **Настройте CORS** заголовки для .wasm файлов
4. **Включите gzip** сжатие для .wasm
5. **Кешируйте** .wasm файлы (долгосрочное кеширование)

### Nginx пример
```nginx
location ~* \\.wasm$ {
    add_header Cache-Control "public, max-age=31536000, immutable";
    add_header Content-Type application/wasm;
    gzip_static on;
}
```

### Apache пример  
```apache
<Files "*.wasm">
    Header set Cache-Control "public, max-age=31536000, immutable"
    Header set Content-Type application/wasm
</Files>
```

## 📊 Performance

Типичные результаты:
- **WASM**: ~2-5ms для генерации лицензии
- **JavaScript**: ~10-20ms для той же операции  
- **Преимущество**: 3-4x ускорение с WASM

## 🔐 Security

⚠️ **Важно**: Это демо версия!

- Используется упрощённый хеш вместо HMAC-SHA256
- Для production используйте полную C++ версию
- Secret key не должен быть в client-side коде
- Генерация лицензий должна быть на сервере

## 🎯 Интеграция

Пример использования WASM в вашем проекте:

```javascript
import LicenseCoreModule from './license_core.js';

LicenseCoreModule().then(Module => {
    const manager = new Module.LicenseCoreWasm("your-secret-key");
    
    const hwid = manager.getCurrentHwid();
    console.log("Hardware ID:", hwid);
    
    const features = new Module.VectorString();
    features.push_back("premium");
    
    const license = manager.generateLicense("user123", features, 365);
    const result = manager.validateLicense(license);
    
    console.log("Valid:", result.valid);
});
```
