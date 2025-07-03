# Error Handling Guide for LicenseCore

This document describes the comprehensive error handling system implemented in LicenseCore using C++ exceptions.

## Exception Hierarchy

All LicenseCore exceptions inherit from the base `LicenseException` class:

```cpp
namespace license_core {
    class LicenseException : public std::exception {
        // Base exception for all LicenseCore errors
    };
}
```

### Specific Exception Types

#### 1. `InvalidSignatureException`
**When thrown:** License signature verification fails
```cpp
try {
    auto info = manager.load_and_validate(license_json);
} catch (const InvalidSignatureException& e) {
    std::cerr << "Signature verification failed: " << e.what() << std::endl;
}
```

#### 2. `ExpiredLicenseException`
**When thrown:** License has passed its expiry date
```cpp
try {
    auto info = manager.load_and_validate(license_json);
} catch (const ExpiredLicenseException& e) {
    std::cerr << "License expired: " << e.what() << std::endl;
    // Could include expiry date in message
}
```

#### 3. `HardwareMismatchException`
**When thrown:** Hardware fingerprint doesn't match license
```cpp
try {
    auto info = manager.load_and_validate(license_json);
} catch (const HardwareMismatchException& e) {
    std::cerr << "Hardware mismatch: " << e.what() << std::endl;
    // Message includes abbreviated fingerprints for debugging
}
```

#### 4. `MalformedLicenseException`
**When thrown:** License format is invalid or corrupted
```cpp
try {
    auto info = manager.load_and_validate(license_json);
} catch (const MalformedLicenseException& e) {
    std::cerr << "Invalid license format: " << e.what() << std::endl;
}
```

#### 5. `MissingFeatureException`
**When thrown:** Required feature is not available in license
```cpp
try {
    manager.require_feature("premium_feature");
} catch (const MissingFeatureException& e) {
    std::cerr << "Feature not available: " << e.what() << std::endl;
}
```

#### 6. `HardwareDetectionException`
**When thrown:** Hardware fingerprinting fails
```cpp
try {
    std::string hwid = manager.get_current_hwid();
} catch (const HardwareDetectionException& e) {
    std::cerr << "Hardware detection failed: " << e.what() << std::endl;
}
```

#### 7. `CryptographicException`
**When thrown:** Cryptographic operations fail
```cpp
try {
    std::string signature = hmac_validator.sign(data);
} catch (const CryptographicException& e) {
    std::cerr << "Crypto operation failed: " << e.what() << std::endl;
}
```

#### 8. `JsonParsingException`
**When thrown:** JSON parsing fails (inherits from `MalformedLicenseException`)
```cpp
try {
    auto info = manager.load_and_validate(invalid_json);
} catch (const JsonParsingException& e) {
    std::cerr << "JSON parsing error: " << e.what() << std::endl;
}
```

#### 9. `ValidationException`
**When thrown:** General license validation fails
```cpp
try {
    bool expired = manager.is_expired(); // in strict mode
} catch (const ValidationException& e) {
    std::cerr << "Validation error: " << e.what() << std::endl;
}
```

#### 10. `NotInitializedException`
**When thrown:** Component not properly initialized
```cpp
try {
    // Using uninitialized component
} catch (const NotInitializedException& e) {
    std::cerr << "Component not initialized: " << e.what() << std::endl;
}
```

## Error Handling Patterns

### Pattern 1: Specific Exception Handling
```cpp
try {
    auto info = manager.load_and_validate(license_json);
    // Use license...
} catch (const ExpiredLicenseException& e) {
    // Handle expired license specifically
    show_renewal_dialog();
} catch (const HardwareMismatchException& e) {
    // Handle hardware mismatch
    request_license_transfer();
} catch (const InvalidSignatureException& e) {
    // Handle tampering
    report_security_violation();
} catch (const LicenseException& e) {
    // Handle any other license error
    show_generic_error(e.what());
}
```

### Pattern 2: Safe vs. Throwing Methods
Some methods provide both throwing and non-throwing versions:

```cpp
// Throwing version (recommended for proper error handling)
try {
    std::string hwid = fingerprint.get_fingerprint();
} catch (const HardwareDetectionException& e) {
    // Handle error appropriately
}

// Safe version (returns empty string on error)
std::string hwid = fingerprint.get_fingerprint_safe(); // noexcept
if (hwid.empty()) {
    // Handle error case
}
```

### Pattern 3: Strict Validation Mode
```cpp
LicenseManager manager(secret_key);
manager.set_strict_validation(true);

// Now methods throw exceptions instead of returning false/empty
try {
    bool has_feature = manager.has_feature("premium"); // throws if no license loaded
    bool expired = manager.is_expired(); // throws if expired
} catch (const ValidationException& e) {
    // Handle validation errors
}
```

### Pattern 4: Comprehensive Error Handling
```cpp
#include "license_core/exceptions.hpp"

void process_license(const std::string& license_json) {
    try {
        LicenseManager manager("secret-key");
        auto info = manager.load_and_validate(license_json);
        
        // Check specific features
        manager.require_feature("required_feature");
        
        // Proceed with application logic
        run_application_with_license(info);
        
    } catch (const ExpiredLicenseException& e) {
        handle_expired_license(e.message());
    } catch (const HardwareMismatchException& e) {
        handle_hardware_mismatch(e.message());
    } catch (const MissingFeatureException& e) {
        handle_missing_feature(e.message());
    } catch (const InvalidSignatureException& e) {
        handle_tampered_license(e.message());
    } catch (const MalformedLicenseException& e) {
        handle_corrupted_license(e.message());
    } catch (const HardwareDetectionException& e) {
        handle_hardware_detection_failure(e.message());
    } catch (const CryptographicException& e) {
        handle_crypto_failure(e.message());
    } catch (const LicenseException& e) {
        handle_generic_license_error(e.message());
    } catch (const std::exception& e) {
        handle_unexpected_error(e.what());
    }
}
```

## Migration from Error Codes

### Before (Error Codes)
```cpp
LicenseInfo info = manager.load_and_validate(license_json);
if (!info.valid) {
    std::cerr << "Error: " << info.error_message << std::endl;
    return;
}
```

### After (Exceptions)
```cpp
try {
    LicenseInfo info = manager.load_and_validate(license_json);
    // License is guaranteed to be valid here
} catch (const LicenseException& e) {
    std::cerr << "License error: " << e.what() << std::endl;
    return;
}
```

## Best Practices

1. **Always catch specific exceptions first**, then general ones
2. **Use RAII and smart pointers** to ensure exception safety
3. **Don't catch and ignore exceptions** - always handle appropriately
4. **Log exceptions with context** for debugging
5. **Use strict validation mode** in production for better error detection
6. **Prefer throwing methods** over safe methods for better error handling
7. **Document exception behavior** in your API

## Error Recovery Strategies

### License Expired
```cpp
catch (const ExpiredLicenseException& e) {
    // Try to refresh/renew license
    if (auto new_license = try_license_renewal()) {
        return process_license(*new_license);
    }
    // Fall back to limited functionality
    run_demo_mode();
}
```

### Hardware Mismatch
```cpp
catch (const HardwareMismatchException& e) {
    // Request license transfer
    if (user_confirms_hardware_change()) {
        request_license_reactivation();
    }
}
```

### Hardware Detection Failure
```cpp
catch (const HardwareDetectionException& e) {
    // Fall back to alternative identification
    try {
        HardwareConfig fallback_config;
        fallback_config.use_cpu_id = false; // Disable problematic component
        manager.set_hardware_config(fallback_config);
        auto info = manager.load_and_validate(license_json);
    } catch (...) {
        // Final fallback
        run_limited_mode();
    }
}
```

This error handling system provides clear, actionable information about what went wrong and enables robust error recovery strategies in your applications.
