#include "license_core/license_manager.hpp"
#include "license_core/hardware_fingerprint.hpp"
#include "license_core/hmac_validator.hpp"
#include "json/simple_json.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <stdexcept>

namespace license_core {

namespace {

std::time_t to_utc_time_t(std::tm* utc_tm) {
#ifdef _WIN32
    return _mkgmtime(utc_tm);
#else
    return timegm(utc_tm);
#endif
}

std::tm gmtime_safe(std::time_t time_value) {
    std::tm tm{};
#ifdef _WIN32
    if (gmtime_s(&tm, &time_value) != 0) {
        throw std::runtime_error("Failed to convert time");
    }
#else
    if (gmtime_r(&time_value, &tm) == nullptr) {
        throw std::runtime_error("Failed to convert time");
    }
#endif
    return tm;
}

} // namespace

// PIMPL implementation
class LicenseManager::Impl {
public:
    explicit Impl(const std::string& secret_key) 
        : hardware_config_(),
          hmac_validator_(secret_key), 
          hardware_fingerprint_(std::make_unique<HardwareFingerprint>(hardware_config_)) {
    }
    
    HardwareConfig hardware_config_;
    HMACValidator hmac_validator_;
    std::unique_ptr<HardwareFingerprint> hardware_fingerprint_;
    LicenseInfo current_license_;
    bool strict_validation_ = false;
};

LicenseManager::LicenseManager(const std::string& secret_key) 
    : pimpl_(std::make_unique<Impl>(secret_key)) {
}

LicenseManager::~LicenseManager() = default;

LicenseInfo LicenseManager::load_and_validate(const std::string& license_json) {
    LicenseInfo info;
    info.valid = false;
    
    try {
        // Parse JSON using our simple parser
        auto license_data = json::SimpleJson::parse(license_json);
        
        // Check for required fields
        std::vector<std::string> required_fields = {
            "user_id", "license_id", "expiry", "hardware_hash", 
            "features", "hmac_signature"
        };
        
        for (const auto& field : required_fields) {
            if (!json::SimpleJson::has_key(license_data, field)) {
                throw MalformedLicenseException("Missing required field: " + field);
            }
        }
        
        // Extract basic fields
        info.user_id = json::SimpleJson::get_string(license_data, "user_id");
        info.license_id = json::SimpleJson::get_string(license_data, "license_id");
        info.hardware_hash = json::SimpleJson::get_string(license_data, "hardware_hash");
        info.features = json::SimpleJson::get_string_array(license_data, "features");
        
        // Validate basic field contents
        if (info.user_id.empty()) {
            throw MalformedLicenseException("user_id cannot be empty");
        }
        if (info.license_id.empty()) {
            throw MalformedLicenseException("license_id cannot be empty");
        }
        if (info.hardware_hash.empty()) {
            throw MalformedLicenseException("hardware_hash cannot be empty");
        }
        
        // Parse version (optional, defaults to 1)
        if (json::SimpleJson::has_key(license_data, "version")) {
            try {
                std::string version_str = json::SimpleJson::get_string(license_data, "version");
                info.version = std::stoul(version_str);
            } catch (const std::exception&) {
                throw MalformedLicenseException("Invalid version format");
            }
        }
        
        // Parse dates with error handling
        try {
            std::string expiry_str = json::SimpleJson::get_string(license_data, "expiry");
            info.expiry = parse_iso8601(expiry_str);
            
            if (json::SimpleJson::has_key(license_data, "issued_at")) {
                std::string issued_str = json::SimpleJson::get_string(license_data, "issued_at");
                info.issued_at = parse_iso8601(issued_str);
            }
        } catch (const std::exception&) {
            throw MalformedLicenseException("Invalid date format");
        }
        
        // Check if license has expired
        auto now = std::chrono::system_clock::now();
        if (now > info.expiry) {
            throw ExpiredLicenseException(format_iso8601(info.expiry));
        }
        
        // Verify signature
        std::string signature = json::SimpleJson::get_string(license_data, "hmac_signature");
        
        // Create JSON without signature for verification
        auto verification_data = license_data;
        verification_data.erase("hmac_signature");
        std::string data_to_verify = json::SimpleJson::stringify(verification_data);
        
        try {
            if (!pimpl_->hmac_validator_.verify(data_to_verify, signature)) {
                throw InvalidSignatureException("HMAC verification failed");
            }
        } catch (const std::exception& e) {
            throw InvalidSignatureException(std::string("Signature verification error: ") + e.what());
        }
        
        // Check hardware fingerprint
        std::string current_hwid;
        try {
            current_hwid = pimpl_->hardware_fingerprint_->get_fingerprint();
        } catch (const HardwareDetectionException& e) {
            throw HardwareDetectionException("Failed to get current hardware fingerprint: " + std::string(e.what()));
        }
        
        if (current_hwid != info.hardware_hash) {
            throw HardwareMismatchException(info.hardware_hash, current_hwid);
        }
        
        // All checks passed
        info.valid = true;
        info.error_message.clear();
        pimpl_->current_license_ = info;
        
    } catch (const LicenseException&) {
        // Re-throw LicenseCore exceptions
        throw;
    } catch (const std::exception& e) {
        // Wrap other exceptions
        throw JsonParsingException(std::string("Unexpected error: ") + e.what());
    }
    
    return info;
}

bool LicenseManager::has_feature(const std::string& feature) const {
    if (!pimpl_->current_license_.valid) {
        if (pimpl_->strict_validation_) {
            throw ValidationException("No valid license loaded");
        }
        return false;
    }
    
    const auto& features = pimpl_->current_license_.features;
    return std::find(features.begin(), features.end(), feature) != features.end();
}

void LicenseManager::require_feature(const std::string& feature) const {
    if (!pimpl_->current_license_.valid) {
        throw ValidationException("No valid license loaded");
    }
    
    if (!has_feature(feature)) {
        throw MissingFeatureException(feature);
    }
}

std::string LicenseManager::generate_license(const LicenseInfo& info) const {
    // Validate input
    if (info.user_id.empty()) {
        throw ValidationException("user_id cannot be empty");
    }
    if (info.license_id.empty()) {
        throw ValidationException("license_id cannot be empty");
    }
    if (info.hardware_hash.empty()) {
        throw ValidationException("hardware_hash cannot be empty");
    }
    
    try {
        std::unordered_map<std::string, json::JsonValue> license_data;
        
        license_data["user_id"] = info.user_id;
        license_data["license_id"] = info.license_id;
        license_data["expiry"] = format_iso8601(info.expiry);
        license_data["issued_at"] = format_iso8601(info.issued_at);
        license_data["hardware_hash"] = info.hardware_hash;
        license_data["features"] = info.features;
        license_data["version"] = std::to_string(info.version);
        
        // Sign the license
        std::string data_to_sign = json::SimpleJson::stringify(license_data);
        std::string signature = pimpl_->hmac_validator_.sign(data_to_sign);
        license_data["hmac_signature"] = signature;
        
        return json::SimpleJson::stringify(license_data);
        
    } catch (const std::exception& e) {
        throw ValidationException("Failed to generate license: " + std::string(e.what()));
    }
}

bool LicenseManager::is_expired() const {
    if (!pimpl_->current_license_.valid) {
        if (pimpl_->strict_validation_) {
            throw ValidationException("No valid license loaded");
        }
        return true;
    }
    
    auto now = std::chrono::system_clock::now();
    bool expired = now > pimpl_->current_license_.expiry;
    
    if (expired && pimpl_->strict_validation_) {
        throw ExpiredLicenseException(format_iso8601(pimpl_->current_license_.expiry));
    }
    
    return expired;
}

std::vector<std::string> LicenseManager::get_available_features() const {
    if (!pimpl_->current_license_.valid) {
        if (pimpl_->strict_validation_) {
            throw ValidationException("No valid license loaded");
        }
        return {};
    }
    
    return pimpl_->current_license_.features;
}

std::string LicenseManager::get_current_hwid() const {
    try {
        return pimpl_->hardware_fingerprint_->get_fingerprint();
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Failed to get hardware fingerprint: " + std::string(e.what()));
    }
}

void LicenseManager::set_hardware_config(const HardwareConfig& config) {
    try {
        pimpl_->hardware_config_ = config;
        // Recreate fingerprint object with new config
        pimpl_->hardware_fingerprint_ = std::make_unique<HardwareFingerprint>(config);
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Failed to set hardware config: " + std::string(e.what()));
    }
}

void LicenseManager::set_strict_validation(bool strict) {
    pimpl_->strict_validation_ = strict;
}

// Helper functions for date parsing/formatting
std::chrono::system_clock::time_point LicenseManager::parse_iso8601(const std::string& date_str) {
    if (date_str.empty()) {
        throw std::invalid_argument("Date string cannot be empty");
    }

    auto parse_with_format = [&date_str](const char* format, bool allow_z, std::tm& out_tm) -> bool {
        out_tm = {};
        std::istringstream ss(date_str);
        ss >> std::get_time(&out_tm, format);
        if (ss.fail()) {
            return false;
        }

        ss >> std::ws;
        if (allow_z && ss.peek() == 'Z') {
            ss.get();
            ss >> std::ws;
        }

        if (!ss.eof()) {
            throw std::invalid_argument("Invalid date format: " + date_str);
        }

        return true;
    };

    std::tm tm{};
    if (!parse_with_format("%Y-%m-%dT%H:%M:%S", true, tm) &&
        !parse_with_format("%Y-%m-%d", false, tm)) {
        throw std::invalid_argument("Invalid date format: " + date_str);
    }

    const auto time_t = to_utc_time_t(&tm);
    if (time_t == static_cast<std::time_t>(-1)) {
        throw std::invalid_argument("Invalid date: " + date_str);
    }

    return std::chrono::system_clock::from_time_t(time_t);
}

std::string LicenseManager::format_iso8601(const std::chrono::system_clock::time_point& time_point) {
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    const std::tm tm = gmtime_safe(time_t);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

bool LicenseManager::validate_license(const std::string& license_json, const std::string& hardware_id) const {
    try {
        // Create a temporary non-const copy to call load_and_validate
        LicenseManager* non_const_this = const_cast<LicenseManager*>(this);
        LicenseInfo info = non_const_this->load_and_validate(license_json);
        return info.hardware_hash == hardware_id && info.valid;
    } catch (const LicenseException&) {
        return false;
    }
}

} // namespace license_core
