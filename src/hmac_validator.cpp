#include "license_core/hmac_validator.hpp"
#include "license_core/license_manager.hpp"
#include "json/simple_json.hpp"
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cctype>

namespace license_core {

HMACValidator::HMACValidator(const std::string& secret_key) 
    : secret_key_(secret_key) {
    if (secret_key_.empty()) {
        throw CryptographicException("Secret key cannot be empty");
    }
    
    // Validate key strength (minimum 16 bytes recommended)
    if (secret_key_.length() < 16) {
        throw CryptographicException("Secret key too short (minimum 16 characters required)");
    }
}

std::string HMACValidator::sign(const std::string& data) const {
    if (data.empty()) {
        throw CryptographicException("Cannot sign empty data");
    }
    
    try {
        return compute_hmac_sha256(data);
    } catch (const std::exception& e) {
        throw CryptographicException("HMAC signing failed: " + std::string(e.what()));
    }
}

bool HMACValidator::verify(const std::string& data, const std::string& signature) const {
    if (data.empty()) {
        throw CryptographicException("Cannot verify empty data");
    }
    
    if (signature.empty()) {
        throw InvalidSignatureException("Signature cannot be empty");
    }
    
    try {
        auto computed_signature = compute_hmac_sha256(data);
        
        // Constant-time comparison to prevent timing attacks
        if (computed_signature.length() != signature.length()) {
            return false;
        }
        
        int result = 0;
        for (size_t i = 0; i < computed_signature.length(); ++i) {
            result |= computed_signature[i] ^ signature[i];
        }
        
        return result == 0;
        
    } catch (const CryptographicException&) {
        throw;
    } catch (const std::exception& e) {
        throw CryptographicException("HMAC verification failed: " + std::string(e.what()));
    }
}

void HMACValidator::verify_or_throw(const std::string& data, const std::string& signature) const {
    if (!verify(data, signature)) {
        throw InvalidSignatureException("HMAC signature verification failed");
    }
}

std::string HMACValidator::sign_json(const std::string& json_without_signature) const {
    if (json_without_signature.empty()) {
        throw JsonParsingException("Cannot sign empty JSON");
    }
    
    try {
        // Validate that input is valid JSON
        auto parsed = json::SimpleJson::parse(json_without_signature);
        
        // Re-stringify to ensure consistent formatting
        std::string normalized_json = json::SimpleJson::stringify(parsed);
        
        return sign(normalized_json);
        
    } catch (const std::exception& e) {
        throw JsonParsingException("JSON signing failed: " + std::string(e.what()));
    }
}

bool HMACValidator::verify_json(const std::string& json_with_signature) const {
    if (json_with_signature.empty()) {
        throw JsonParsingException("Cannot verify empty JSON");
    }
    
    try {
        // Parse JSON
        auto parsed = json::SimpleJson::parse(json_with_signature);
        
        // Check if signature field exists
        if (!json::SimpleJson::has_key(parsed, "hmac_signature")) {
            throw JsonParsingException("JSON missing hmac_signature field");
        }
        
        // Extract signature
        std::string signature = json::SimpleJson::get_string(parsed, "hmac_signature");
        
        // Remove signature field for verification
        parsed.erase("hmac_signature");
        std::string data_to_verify = json::SimpleJson::stringify(parsed);
        
        return verify(data_to_verify, signature);
        
    } catch (const LicenseException&) {
        throw;
    } catch (const std::exception& e) {
        throw JsonParsingException("JSON verification failed: " + std::string(e.what()));
    }
}

void HMACValidator::verify_json_or_throw(const std::string& json_with_signature) const {
    if (!verify_json(json_with_signature)) {
        throw InvalidSignatureException("JSON HMAC signature verification failed");
    }
}

std::string HMACValidator::compute_hmac_sha256(const std::string& data) const {
    try {
        unsigned char result[EVP_MAX_MD_SIZE];
        unsigned int result_len = 0;
        
        unsigned char* mac = HMAC(
            EVP_sha256(),
            secret_key_.c_str(),
            static_cast<int>(secret_key_.length()),
            reinterpret_cast<const unsigned char*>(data.c_str()),
            data.length(),
            result,
            &result_len
        );
        
        if (mac == nullptr) {
            throw CryptographicException("HMAC computation failed");
        }
        
        return to_hex(std::vector<uint8_t>(result, result + result_len));
        
    } catch (const std::exception& e) {
        throw CryptographicException("HMAC-SHA256 computation error: " + std::string(e.what()));
    }
}

std::string HMACValidator::to_hex(const std::vector<uint8_t>& bytes) const {
    try {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (uint8_t byte : bytes) {
            ss << std::setw(2) << static_cast<unsigned int>(byte);
        }
        return ss.str();
    } catch (const std::exception& e) {
        throw CryptographicException("Hex encoding failed: " + std::string(e.what()));
    }
}

std::vector<uint8_t> HMACValidator::from_hex(const std::string& hex) const {
    try {
        if (hex.length() % 2 != 0) {
            throw CryptographicException("Invalid hex string length (must be even)");
        }
        
        std::vector<uint8_t> bytes;
        bytes.reserve(hex.length() / 2);
        
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byte_string = hex.substr(i, 2);
            
            // Validate hex characters
            for (char c : byte_string) {
                if (!std::isxdigit(static_cast<unsigned char>(c))) {
                    throw CryptographicException("Invalid hex character: " + std::string(1, c));
                }
            }
            
            uint8_t byte = static_cast<uint8_t>(std::stoul(byte_string, nullptr, 16));
            bytes.push_back(byte);
        }
        
        return bytes;
        
    } catch (const CryptographicException&) {
        throw;
    } catch (const std::exception& e) {
        throw CryptographicException("Hex decoding failed: " + std::string(e.what()));
    }
}

bool HMACValidator::validate_license(const LicenseInfo& license_info, const std::string& hardware_id) const {
    try {
        // Check hardware ID match
        if (license_info.hardware_hash != hardware_id) {
            throw LicenseValidationException("Hardware ID mismatch");
        }
        
        // Check expiration
        auto now = std::chrono::system_clock::now();
        if (license_info.expiry < now) {
            throw LicenseValidationException("License has expired");
        }
        
        // If we get here, validation passed
        return true;
        
    } catch (const LicenseValidationException&) {
        throw;
    } catch (const std::exception& e) {
        throw LicenseValidationException("Validation failed: " + std::string(e.what()));
    }
}

} // namespace license_core
