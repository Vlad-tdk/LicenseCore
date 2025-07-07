#pragma once

// LicenseCore++ Pure C API
// Zero C++ dependencies for minimal symbol export

#ifdef __cplusplus
extern "C" {
#endif

// Strict C API with explicit visibility control
#if defined(__APPLE__)
    #define LC_EXPORT __attribute__((visibility("default")))
#elif defined(__GNUC__)
    #define LC_EXPORT __attribute__((visibility("default")))
#elif defined(_WIN32)
    #define LC_EXPORT __declspec(dllexport)
#else
    #define LC_EXPORT
#endif

/**
 * Validate license from JSON string
 * @param license_json License JSON data (null-terminated)
 * @return 1 if valid, 0 if invalid
 */
LC_EXPORT int lc_validate_license(const char* license_json);

/**
 * Check if license has specific feature
 * @param feature_name Feature name to check (null-terminated)
 * @return 1 if feature available, 0 if not
 */
LC_EXPORT int lc_has_feature(const char* feature_name);

/**
 * Get current hardware ID
 * @return Hardware ID string (static buffer, don't free)
 */
LC_EXPORT const char* lc_get_hwid(void);

/**
 * Validate embedded license (built into library)
 * @return 1 if valid, 0 if invalid
 */
LC_EXPORT int lc_validate_embedded(void);

#ifdef __cplusplus
}
#endif

// Usage example:
/*
#include "license_core_stub.h"

int main() {
    // Method 1: External license
    if (lc_validate_license(license_json)) {
        if (lc_has_feature("premium")) {
            enable_premium_features();
        }
    }
    
    // Method 2: Embedded license
    if (lc_validate_embedded()) {
        run_application();
    }
    
    return 0;
}
*/
