#pragma once

#include <string>
#include <vector>
#include "exceptions.hpp"

// Forward declaration
namespace license_core {
    struct LicenseInfo;
}

namespace license_core {

class HMACValidator {
public:
    explicit HMACValidator(const std::string& secret_key);
    
    // Generate HMAC signature for data - throws CryptographicException on failure
    std::string sign(const std::string& data) const;
    
    // Verify HMAC signature - throws InvalidSignatureException if verification fails
    bool verify(const std::string& data, const std::string& signature) const;
    void verify_or_throw(const std::string& data, const std::string& signature) const;
    
    // Utility: sign JSON without signature field - throws on parsing/crypto errors
    std::string sign_json(const std::string& json_without_signature) const;
    
    // Utility: verify JSON by extracting signature field - throws on errors
    bool verify_json(const std::string& json_with_signature) const;
    void verify_json_or_throw(const std::string& json_with_signature) const;
    
    // License validation - throws LicenseValidationException on failure
    bool validate_license(const LicenseInfo& license_info, const std::string& hardware_id) const;
    
private:
    std::string secret_key_;
    
    std::string compute_hmac_sha256(const std::string& data) const;
    std::string to_hex(const std::vector<uint8_t>& bytes) const;
    std::vector<uint8_t> from_hex(const std::string& hex) const;
};

} // namespace license_core
