#pragma once

#include <exception>
#include <string>

namespace license_core {

/**
 * Base exception class for all LicenseCore-related errors
 */
class LicenseException : public std::exception {
public:
    explicit LicenseException(const std::string& message) 
        : message_(message) {}
    
    virtual const char* what() const noexcept override {
        return message_.c_str();
    }
    
    const std::string& message() const noexcept {
        return message_;
    }

protected:
    std::string message_;
};

/**
 * Thrown when license signature verification fails
 */
class InvalidSignatureException : public LicenseException {
public:
    explicit InvalidSignatureException(const std::string& message = "Invalid license signature")
        : LicenseException("Invalid signature: " + message) {}
};

/**
 * Thrown when license has expired
 */
class ExpiredLicenseException : public LicenseException {
public:
    explicit ExpiredLicenseException(const std::string& expiry_date = "")
        : LicenseException("License has expired" + (expiry_date.empty() ? "" : " (expired: " + expiry_date + ")")) {}
};

/**
 * Thrown when hardware fingerprint doesn't match
 */
class HardwareMismatchException : public LicenseException {
public:
    explicit HardwareMismatchException(const std::string& expected = "", const std::string& actual = "")
        : LicenseException("Hardware mismatch" + 
                          (expected.empty() ? "" : " (expected: " + expected.substr(0, 8) + "..., actual: " + actual.substr(0, 8) + "...)")) {}
};

/**
 * Thrown when license format is invalid or corrupted
 */
class MalformedLicenseException : public LicenseException {
public:
    explicit MalformedLicenseException(const std::string& details = "")
        : LicenseException("Malformed license" + (details.empty() ? "" : ": " + details)) {}
};

/**
 * Thrown when required license feature is missing
 */
class MissingFeatureException : public LicenseException {
public:
    explicit MissingFeatureException(const std::string& feature = "")
        : LicenseException("Missing required feature" + (feature.empty() ? "" : ": " + feature)) {}
};

/**
 * Thrown when hardware fingerprinting fails
 */
class HardwareDetectionException : public LicenseException {
public:
    explicit HardwareDetectionException(const std::string& component = "")
        : LicenseException("Hardware detection failed" + (component.empty() ? "" : " for component: " + component)) {}
};

/**
 * Thrown when cryptographic operations fail
 */
class CryptographicException : public LicenseException {
public:
    explicit CryptographicException(const std::string& operation = "")
        : LicenseException("Cryptographic operation failed" + (operation.empty() ? "" : ": " + operation)) {}
};

/**
 * Thrown when JSON parsing fails
 */
class JsonParsingException : public MalformedLicenseException {
public:
    explicit JsonParsingException(const std::string& details = "")
        : MalformedLicenseException("JSON parsing error" + (details.empty() ? "" : ": " + details)) {}
};

/**
 * Thrown when license validation fails for unspecified reasons
 */
class ValidationException : public LicenseException {
public:
    explicit ValidationException(const std::string& reason = "")
        : LicenseException("License validation failed" + (reason.empty() ? "" : ": " + reason)) {}
};

/**
 * Thrown when license validation fails (alias for ValidationException)
 */
class LicenseValidationException : public ValidationException {
public:
    explicit LicenseValidationException(const std::string& reason = "")
        : ValidationException(reason) {}
};

/**
 * Thrown when license parsing fails (alias for JsonParsingException)
 */
class LicenseParsingException : public JsonParsingException {
public:
    explicit LicenseParsingException(const std::string& details = "")
        : JsonParsingException(details) {}
};

/**
 * Thrown when attempting to use license functionality before proper initialization
 */
class NotInitializedException : public LicenseException {
public:
    explicit NotInitializedException(const std::string& component = "")
        : LicenseException("Component not properly initialized" + (component.empty() ? "" : ": " + component)) {}
};

} // namespace license_core
