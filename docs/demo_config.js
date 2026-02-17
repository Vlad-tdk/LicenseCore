// LicenseCore++ Demo Configuration
// 🚨 ВАЖНО: Это ДЕМО ключи, НЕ используйте в продакшене!

const DEMO_CONFIG = {
    // Demo-only ключи (безопасно для публичного репозитория)
    DEMO_KEYS: {
        // Каждый ключ имеет префикс для безопасности
        demo: "demo-github-pages-" + btoa("licensecore-demo-2024").substring(0, 16),
        fallback: "demo-fallback-" + btoa("javascript-only-demo").substring(0, 16)
    },
    
    // Ограничения демо
    LIMITS: {
        maxLicensesPerHour: 10,
        maxFeaturesPerLicense: 3, 
        maxExpiryDays: 7,
        watermark: "🔬 DEMO-ONLY"
    },
    
    // Warnings для пользователей
    SECURITY_WARNINGS: {
        demoOnly: "⚠️ Demo keys only - not for production use!",
        publicKeys: "🔓 These keys are public and visible to everyone",
        notSecure: "🚨 Never use demo keys in real applications",
        productionGuide: "📚 See documentation for production setup"
    },
    
    // Визуальные индикаторы
    UI_INDICATORS: {
        demoMode: true,
        showWarnings: true,
        limitFeatures: true
    }
};

// Экспорт для использования
if (typeof module !== 'undefined' && module.exports) {
    module.exports = DEMO_CONFIG;
} else if (typeof window !== 'undefined') {
    window.DEMO_CONFIG = DEMO_CONFIG;
}
