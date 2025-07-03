#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include "hardware_fingerprint.hpp"
#include "exceptions.hpp"

namespace license_core {

struct LicenseInfo {
    std::string user_id;
    std::string hardware_hash;
    std::vector<std::string> features;
    std::chrono::system_clock::time_point expiry;
    std::chrono::system_clock::time_point issued_at;
    std::string license_id;
    uint32_t version = 1;
    bool valid = false;
    std::string error_message;
};

class LicenseManager {
public:
    explicit LicenseManager(const std::string& secret_key);
    ~LicenseManager();

    // Core functionality - now throws exceptions instead of returning error info
    LicenseInfo load_and_validate(const std::string& license_json);
    bool validate_license(const std::string& license_json, const std::string& hardware_id) const;
    
    // Feature checking - throws MissingFeatureException if feature not available
    bool has_feature(const std::string& feature) const;
    void require_feature(const std::string& feature) const; // throws if missing
    
    // Utility methods
    std::string generate_license(const LicenseInfo& info) const;
    bool is_expired() const; // throws ExpiredLicenseException if expired and strict mode enabled
    std::vector<std::string> get_available_features() const;
    std::string get_current_hwid() const; // throws HardwareDetectionException on failure
    
    // Configuration
    void set_hardware_config(const HardwareConfig& config);
    void set_strict_validation(bool strict = true); // If true, methods throw on invalid state
    
private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
    
    // Helper functions for date parsing/formatting
    static std::chrono::system_clock::time_point parse_iso8601(const std::string& date_str);
    static std::string format_iso8601(const std::chrono::system_clock::time_point& time_point);
};

} // namespace license_core
