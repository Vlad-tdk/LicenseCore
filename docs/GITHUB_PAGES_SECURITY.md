# 🛡️ LicenseCore++ Secure Demo Configuration

## Проблемы безопасности и решения

### 🚨 **Критические проблемы:**

#### 1. **Секретный ключ в коде**
```javascript
// ❌ ОПАСНО - ключ видим всем
this.secretKey = "demo-secret-key-2024";

// ✅ БЕЗОПАСНО - отдельные ключи для демо
this.demoKey = "demo-public-key-for-testing-only";
this.productionKey = null; // Только на сервере
```

#### 2. **HTTPS Requirement для Web Crypto API**
```html
<!-- Обязательно для GitHub Pages -->
<script>
if (location.protocol !== 'https:' && location.hostname !== 'localhost') {
    location.href = 'https:' + window.location.href.substring(window.location.protocol.length);
}
</script>
```

#### 3. **WASM MIME Type Fix для GitHub Pages**
```javascript
// Исправление для GitHub Pages
async function loadWASM() {
    const response = await fetch('./license_core.wasm');
    if (!response.ok) {
        throw new Error('Failed to load WASM');
    }
    
    // Force correct MIME type
    const bytes = await response.arrayBuffer();
    return new Uint8Array(bytes);
}
```

### 🔒 **Безопасная архитектура демо:**

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│  Demo Frontend  │ ←→ │   Demo Server    │ ←→ │ Production API  │
│   (Public)      │    │  (GitHub Pages)  │    │   (Private)     │
└─────────────────┘    └──────────────────┘    └─────────────────┘
        │                         │                        │
   Demo Keys Only          Limited Demo         Real Keys
   No Real Secrets         No Production         Secure Vault
```

### 📁 **Рекомендуемая структура для GitHub Pages:**

```
docs/
├── demo/                  # Демо версия (GitHub Pages)
│   ├── demo_keys.js      # Публичные ключи только для демо
│   ├── demo_data.js      # Безопасные тестовые данные  
│   └── demo_limits.js    # Ограничения демо
├── production/           # Production guide
│   ├── integration.md    # Как интегрировать
│   └── security.md       # Security best practices
└── .nojekyll            # Для правильной работы WASM
```

### 🎛️ **Demo Configuration:**

```javascript
// demo_config.js - Безопасная конфигурация
const DEMO_CONFIG = {
    // Публичные ключи ТОЛЬКО для демо
    DEMO_KEYS: {
        basic: "demo-basic-key-2024",
        premium: "demo-premium-key-2024"
    },
    
    // Ограничения демо
    LIMITS: {
        maxLicenses: 10,          // Максимум лицензий в час
        maxFeatures: 3,           // Максимум фич
        demoExpiryDays: 1,        // Демо лицензии на 1 день
        watermark: "DEMO-ONLY"    // Водяной знак
    },
    
    // Предупреждения
    WARNINGS: {
        notForProduction: true,
        keysArePublic: true,
        demoDataOnly: true
    }
};
```

### 🔐 **WASM Security Headers:**

```javascript
// Добавить в HTML для правильной работы WASM
const securityHeaders = `
<meta http-equiv="Cross-Origin-Embedder-Policy" content="require-corp">
<meta http-equiv="Cross-Origin-Opener-Policy" content="same-origin">
<meta http-equiv="Content-Security-Policy" content="
    default-src 'self';
    script-src 'self' 'wasm-eval';
    worker-src 'self';
    connect-src 'self' https:;
">`;
```

### 🚀 **Production Integration Guide:**

```markdown
## Переход от демо к продакшену:

1. **Замените демо ключи:**
   ```javascript
   // ❌ Demo
   const manager = new LicenseCoreWasm('demo-key');
   
   // ✅ Production  
   const manager = new LicenseCoreWasm(await getSecureKey());
   ```

2. **Используйте server-side генерацию:**
   ```javascript
   // ❌ Client-side (небезопасно)
   const license = manager.generateLicense(...);
   
   // ✅ Server-side (безопасно)
   const license = await api.generateLicense(...);
   ```

3. **Добавьте rate limiting:**
   ```javascript
   // ✅ Production safeguards
   if (requests_per_hour > 100) {
       throw new Error('Rate limit exceeded');
   }
   ```
```

### 📋 **GitHub Pages Checklist:**

- [ ] HTTPS включен
- [ ] `.nojekyll` файл добавлен
- [ ] WASM MIME type обработан
- [ ] CSP headers настроены
- [ ] Demo keys отделены от production
- [ ] Rate limiting добавлен
- [ ] Watermarks для demo лицензий
- [ ] Предупреждения о безопасности
- [ ] Production integration guide

### ⚡ **Quick Fix для существующего демо:**

```javascript
// Добавить в начало wasm.js
if (typeof window !== 'undefined') {
    // GitHub Pages security fixes
    window.DEMO_MODE = true;
    window.DEMO_WARNING = "⚠️ Demo keys only - not for production!";
    
    // Override secret key for demo
    const originalSecretKey = "demo-secret-key-2024";
    const demoSecretKey = "github-pages-demo-" + btoa(location.hostname);
    
    console.warn("🚨 DEMO MODE: Using public demo keys only!");
    console.warn("📚 See documentation for production integration");
}
```
