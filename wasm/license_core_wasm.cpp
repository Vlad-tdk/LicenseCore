#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <future>

// Используем Web Crypto API для настоящего HMAC-SHA256
namespace license_core_wasm {

// JavaScript функция для HMAC-SHA256 через Web Crypto API
EM_JS(void, js_compute_hmac_sha256, (const char* data, const char* key, char* result, int result_size), {
    const dataStr = UTF8ToString(data);
    const keyStr = UTF8ToString(key);
    
    // Используем Web Crypto API
    const encoder = new TextEncoder();
    const keyData = encoder.encode(keyStr);
    const dataBuffer = encoder.encode(dataStr);
    
    crypto.subtle.importKey(
        'raw',
        keyData,
        { name: 'HMAC', hash: 'SHA-256' },
        false,
        ['sign']
    ).then(cryptoKey => {
        return crypto.subtle.sign('HMAC', cryptoKey, dataBuffer);
    }).then(signature => {
        const hashArray = Array.from(new Uint8Array(signature));
        const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
        
        // Копируем результат в C++ буфер
        const resultBytes = lengthBytesUTF8(hashHex) + 1;
        if (resultBytes <= result_size) {
            stringToUTF8(hashHex, result, result_size);
        }
        
        // Уведомляем C++ что операция завершена
        Module._hmac_ready(result);
    }).catch(error => {
        console.error('HMAC computation failed:', error);
        Module._hmac_error();
    });
});

// Callback функции для асинхронного HMAC
extern "C" {
    void hmac_ready(char* result);
    void hmac_error();
}

static std::string pending_hmac_result;
static bool hmac_operation_complete = false;
static bool hmac_operation_error = false;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void hmac_ready(char* result) {
        pending_hmac_result = std::string(result);
        hmac_operation_complete = true;
        hmac_operation_error = false;
    }
    
    EMSCRIPTEN_KEEPALIVE
    void hmac_error() {
        hmac_operation_complete = true;
        hmac_operation_error = true;
        pending_hmac_result.clear();
    }
}

// Синхронная обертка для асинхронного Web Crypto API
std::string compute_hmac_sha256(const std::string& data, const std::string& key) {
    // Сброс состояния
    hmac_operation_complete = false;
    hmac_operation_error = false;
    pending_hmac_result.clear();
    
    // Буфер для результата (64 символа hex + null terminator)
    char result_buffer[65] = {0};
    
    // Вызываем JavaScript функцию
    js_compute_hmac_sha256(data.c_str(), key.c_str(), result_buffer, sizeof(result_buffer));
    
    // Ждем завершения асинхронной операции
    int timeout_ms = 5000; // 5 секунд таймаут
    int elapsed_ms = 0;
    const int poll_interval_ms = 10;
    
    while (!hmac_operation_complete && elapsed_ms < timeout_ms) {
        emscripten_sleep(poll_interval_ms);
        elapsed_ms += poll_interval_ms;
    }
    
    if (hmac_operation_error || elapsed_ms >= timeout_ms) {
        throw std::runtime_error("HMAC computation failed or timed out");
    }
    
    return pending_hmac_result;
}

// WASM-compatible hardware fingerprint
std::string get_browser_fingerprint() {
    // В реальном WASM можно получить:
    // - navigator.userAgent
    // - screen resolution
    // - timezone
    // - installed fonts
    // - WebGL renderer info
    
    std::string fingerprint = "wasm-demo-fingerprint-";
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    
    std::hash<std::string> hasher;
    auto hash_value = hasher(std::to_string(timestamp));
    
    std::stringstream ss;
    ss << fingerprint << std::hex << (hash_value % 0xFFFFFF);
    return ss.str();
}

class LicenseCoreWasm {
private:
    std::string secret_key_;
    std::string current_hwid_;
    
public:
    LicenseCoreWasm(const std::string& secret_key) 
        : secret_key_(secret_key), current_hwid_(get_browser_fingerprint()) {}
    
    std::string getCurrentHwid() const {
        return current_hwid_;
    }
    
    void generateNewHwid() {
        current_hwid_ = get_browser_fingerprint();
    }
    
    std::string generateLicense(
        const std::string& user_id,
        const std::vector<std::string>& features,
        int expiry_days
    ) {
        auto now = std::chrono::system_clock::now();
        auto expiry = now + std::chrono::hours(24 * expiry_days);
        
        // Create license JSON
        std::stringstream license_json;
        license_json << "{\n";
        license_json << "  \"user_id\": \"" << user_id << "\",\n";
        license_json << "  \"license_id\": \"lic-" << std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() << "\",\n";
        
        // Format dates
        std::time_t expiry_time = std::chrono::system_clock::to_time_t(expiry);
        std::time_t issued_time = std::chrono::system_clock::to_time_t(now);
        
        license_json << "  \"expiry\": \"" << std::put_time(std::gmtime(&expiry_time), "%Y-%m-%dT%H:%M:%SZ") << "\",\n";
        license_json << "  \"issued_at\": \"" << std::put_time(std::gmtime(&issued_time), "%Y-%m-%dT%H:%M:%SZ") << "\",\n";
        license_json << "  \"hardware_hash\": \"" << current_hwid_ << "\",\n";
        
        // Features array
        license_json << "  \"features\": [";
        for (size_t i = 0; i < features.size(); ++i) {
            license_json << "\"" << features[i] << "\"";
            if (i < features.size() - 1) license_json << ", ";
        }
        license_json << "],\n";
        license_json << "  \"version\": 1\n";
        license_json << "}";
        
        std::string license_data = license_json.str();
        std::string signature = compute_hmac_sha256(license_data, secret_key_);
        
        // Add signature
        size_t insert_pos = license_data.find_last_of('}');
        license_data.insert(insert_pos, ",\n  \"hmac_signature\": \"" + signature + "\"\n");
        
        return license_data;
    }
    
    struct ValidationResult {
        bool valid;
        std::string error_message;
        std::string user_id;
        std::vector<std::string> features;
    };
    
    ValidationResult validateLicense(const std::string& license_json) {
        ValidationResult result;
        result.valid = false;
        
        try {
            // Simple JSON parsing for demo
            if (license_json.find("\"user_id\"") == std::string::npos) {
                result.error_message = "Missing user_id field";
                return result;
            }
            
            if (license_json.find("\"hmac_signature\"") == std::string::npos) {
                result.error_message = "Missing signature field";
                return result;
            }
            
            // Extract user_id (простой парсинг для демо)
            size_t user_start = license_json.find("\"user_id\": \"") + 12;
            size_t user_end = license_json.find("\"", user_start);
            result.user_id = license_json.substr(user_start, user_end - user_start);
            
            // Extract hardware hash
            size_t hw_start = license_json.find("\"hardware_hash\": \"") + 18;
            size_t hw_end = license_json.find("\"", hw_start);
            std::string hardware_hash = license_json.substr(hw_start, hw_end - hw_start);
            
            // Check hardware fingerprint
            if (hardware_hash != current_hwid_) {
                result.error_message = "Hardware fingerprint mismatch";
                return result;
            }
            
            // Extract signature
            size_t sig_start = license_json.find("\"hmac_signature\": \"") + 19;
            size_t sig_end = license_json.find("\"", sig_start);
            std::string signature = license_json.substr(sig_start, sig_end - sig_start);
            
            // Create data without signature for verification
            std::string data_without_sig = license_json;
            size_t sig_field_start = data_without_sig.find(",\n  \"hmac_signature\"");
            if (sig_field_start != std::string::npos) {
                size_t sig_field_end = data_without_sig.find("\n}", sig_field_start) + 1;
                data_without_sig.erase(sig_field_start, sig_field_end - sig_field_start);
            }
            
            // Verify signature
            std::string computed_signature = compute_hmac_sha256(data_without_sig, secret_key_);
            if (signature != computed_signature) {
                result.error_message = "Invalid license signature";
                return result;
            }
            
            // Extract features
            size_t features_start = license_json.find("\"features\": [") + 13;
            size_t features_end = license_json.find("]", features_start);
            std::string features_str = license_json.substr(features_start, features_end - features_start);
            
            // Parse features (простой парсинг)
            std::stringstream ss(features_str);
            std::string feature;
            while (std::getline(ss, feature, ',')) {
                // Clean up feature string
                feature.erase(0, feature.find_first_not_of(" \t\n\r\""));
                feature.erase(feature.find_last_not_of(" \t\n\r\"") + 1);
                if (!feature.empty()) {
                    result.features.push_back(feature);
                }
            }
            
            result.valid = true;
            result.error_message = "License is valid";
            
        } catch (const std::exception& e) {
            result.error_message = "JSON parsing error: " + std::string(e.what());
        }
        
        return result;
    }
    
    bool hasFeature(const std::string& feature, const std::vector<std::string>& license_features) {
        return std::find(license_features.begin(), license_features.end(), feature) != license_features.end();
    }
};

} // namespace license_core_wasm

// Emscripten bindings
using namespace emscripten;
using namespace license_core_wasm;

EMSCRIPTEN_BINDINGS(license_core) {
    register_vector<std::string>("VectorString");
    
    value_object<LicenseCoreWasm::ValidationResult>("ValidationResult")
        .field("valid", &LicenseCoreWasm::ValidationResult::valid)
        .field("error_message", &LicenseCoreWasm::ValidationResult::error_message)
        .field("user_id", &LicenseCoreWasm::ValidationResult::user_id)
        .field("features", &LicenseCoreWasm::ValidationResult::features);
    
    class_<LicenseCoreWasm>("LicenseCoreWasm")
        .constructor<std::string>()
        .function("getCurrentHwid", &LicenseCoreWasm::getCurrentHwid)
        .function("generateNewHwid", &LicenseCoreWasm::generateNewHwid)
        .function("generateLicense", &LicenseCoreWasm::generateLicense)
        .function("validateLicense", &LicenseCoreWasm::validateLicense)
        .function("hasFeature", &LicenseCoreWasm::hasFeature);
}