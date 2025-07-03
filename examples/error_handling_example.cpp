#include <iostream>
#include <string>
#include "license_core/license_manager.hpp"
#include "license_core/exceptions.hpp"

using namespace license_core;

int main() {
    try {
        std::cout << "=== LicenseCore Error Handling Example ===" << std::endl;
        
        // Initialize license manager
        std::string secret_key = "this-is-a-very-secure-secret-key-123";
        LicenseManager manager(secret_key);
        
        // Example 1: Loading an invalid license
        std::cout << "\n1. Testing invalid license JSON..." << std::endl;
        try {
            std::string invalid_json = "{ invalid json }";
            auto info = manager.load_and_validate(invalid_json);
            std::cout << "ERROR: Should have thrown an exception!" << std::endl;
        } catch (const JsonParsingException& e) {
            std::cout << "✓ Caught JsonParsingException: " << e.what() << std::endl;
        }
        
        // Example 2: Missing required fields
        std::cout << "\n2. Testing license with missing fields..." << std::endl;
        try {
            std::string incomplete_json = R"({
                "user_id": "test_user"
            })";
            auto info = manager.load_and_validate(incomplete_json);
            std::cout << "ERROR: Should have thrown an exception!" << std::endl;
        } catch (const MalformedLicenseException& e) {
            std::cout << "✓ Caught MalformedLicenseException: " << e.what() << std::endl;
        }
        
        // Example 3: Testing hardware fingerprint error handling
        std::cout << "\n3. Testing hardware fingerprint..." << std::endl;
        try {
            std::string hwid = manager.get_current_hwid();
            std::cout << "✓ Hardware ID retrieved: " << hwid.substr(0, 16) << "..." << std::endl;
        } catch (const HardwareDetectionException& e) {
            std::cout << "✗ Hardware detection failed: " << e.what() << std::endl;
        }
        
        // Example 4: Generate a valid license
        std::cout << "\n4. Generating valid license..." << std::endl;
        try {
            LicenseInfo info;
            info.user_id = "test_user_123";
            info.license_id = "lic_" + std::to_string(std::time(nullptr));
            info.hardware_hash = manager.get_current_hwid();
            info.features = {"feature1", "feature2", "premium_feature"};
            
            // Set expiry to 1 year from now
            auto now = std::chrono::system_clock::now();
            info.expiry = now + std::chrono::hours(24 * 365);
            info.issued_at = now;
            info.version = 1;
            
            std::string license_json = manager.generate_license(info);
            std::cout << "✓ License generated successfully" << std::endl;
            
            // Example 5: Validate the generated license
            std::cout << "\n5. Validating generated license..." << std::endl;
            auto validated_info = manager.load_and_validate(license_json);
            std::cout << "✓ License validation successful!" << std::endl;
            std::cout << "  User ID: " << validated_info.user_id << std::endl;
            std::cout << "  License ID: " << validated_info.license_id << std::endl;
            std::cout << "  Features: ";
            for (const auto& feature : validated_info.features) {
                std::cout << feature << " ";
            }
            std::cout << std::endl;
            
            // Example 6: Test feature requirement
            std::cout << "\n6. Testing feature requirements..." << std::endl;
            try {
                manager.require_feature("feature1");
                std::cout << "✓ Feature 'feature1' is available" << std::endl;
                
                manager.require_feature("non_existent_feature");
                std::cout << "ERROR: Should have thrown an exception!" << std::endl;
            } catch (const MissingFeatureException& e) {
                std::cout << "✓ Caught MissingFeatureException: " << e.what() << std::endl;
            }
            
        } catch (const LicenseException& e) {
            std::cout << "✗ License operation failed: " << e.what() << std::endl;
        }
        
        // Example 7: Test expired license
        std::cout << "\n7. Testing expired license..." << std::endl;
        try {
            LicenseInfo expired_info;
            expired_info.user_id = "expired_user";
            expired_info.license_id = "expired_license";
            expired_info.hardware_hash = manager.get_current_hwid();
            expired_info.features = {"basic_feature"};
            
            // Set expiry to yesterday
            auto yesterday = std::chrono::system_clock::now() - std::chrono::hours(24);
            expired_info.expiry = yesterday;
            expired_info.issued_at = yesterday - std::chrono::hours(24);
            expired_info.version = 1;
            
            std::string expired_license = manager.generate_license(expired_info);
            auto expired_result = manager.load_and_validate(expired_license);
            std::cout << "ERROR: Should have thrown an exception!" << std::endl;
            
        } catch (const ExpiredLicenseException& e) {
            std::cout << "✓ Caught ExpiredLicenseException: " << e.what() << std::endl;
        }
        
        // Example 8: Test strict validation mode
        std::cout << "\n8. Testing strict validation mode..." << std::endl;
        try {
            LicenseManager strict_manager(secret_key);
            strict_manager.set_strict_validation(true);
            
            // Try to check features without loading a license
            bool has_feature = strict_manager.has_feature("any_feature");
            std::cout << "ERROR: Should have thrown an exception!" << std::endl;
            
        } catch (const ValidationException& e) {
            std::cout << "✓ Caught ValidationException in strict mode: " << e.what() << std::endl;
        }
        
        std::cout << "\n=== All error handling tests completed successfully! ===" << std::endl;
        
    } catch (const LicenseException& e) {
        std::cerr << "LicenseCore error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
