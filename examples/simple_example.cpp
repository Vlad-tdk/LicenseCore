#include <license_core/license_manager.hpp>
#include <iostream>
#include <chrono>

using namespace license_core;

int main() {
    std::cout << "=== LicenseCore++ Simple Example ===" << std::endl;
    
    try {
        // Initialize license manager with secret key
        LicenseManager manager("my-super-secret-key-2024");
        
        // Get current hardware fingerprint
        std::string hwid = manager.get_current_hwid();
        std::cout << "Current Hardware ID: " << hwid << std::endl;
        
        // Create sample license
        LicenseInfo sample_license;
        sample_license.user_id = "vlad-1337";
        sample_license.license_id = "lic-" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        sample_license.hardware_hash = hwid;
        sample_license.features = {"basic", "premium", "api"};
        sample_license.issued_at = std::chrono::system_clock::now();
        sample_license.expiry = std::chrono::system_clock::now() + std::chrono::hours(24 * 365); // 1 year
        sample_license.version = 1;
        
        // Generate signed license
        std::string license_json = manager.generate_license(sample_license);
        std::cout << "\\nGenerated License:\\n" << license_json << std::endl;
        
        // Validate the license
        auto validated_info = manager.load_and_validate(license_json);
        
        if (validated_info.valid) {
            std::cout << "\\n✅ License is VALID!" << std::endl;
            std::cout << "User: " << validated_info.user_id << std::endl;
            std::cout << "License ID: " << validated_info.license_id << std::endl;
            std::cout << "Features: ";
            for (const auto& feature : validated_info.features) {
                std::cout << feature << " ";
            }
            std::cout << std::endl;
            
            // Test feature checking
            if (manager.has_feature("premium")) {
                std::cout << "✅ Premium features available!" << std::endl;
            }
            if (manager.has_feature("enterprise")) {
                std::cout << "❌ Enterprise features not available" << std::endl;
            } else {
                std::cout << "ℹ️  Enterprise features not included in license" << std::endl;
            }
            
        } else {
            std::cout << "\\n❌ License is INVALID!" << std::endl;
            std::cout << "Error: " << validated_info.error_message << std::endl;
        }
        
        // Test with invalid license
        std::cout << "\\n=== Testing Invalid License ===" << std::endl;
        std::string invalid_license = R"({
            "user_id": "hacker",
            "license_id": "fake-license",
            "expiry": "2025-12-31T23:59:59Z",
            "hardware_hash": "wrong-hash",
            "features": ["premium"],
            "version": "1",
            "hmac_signature": "invalid-signature"
        })";
        
        auto invalid_info = manager.load_and_validate(invalid_license);
        if (!invalid_info.valid) {
            std::cout << "✅ Invalid license correctly rejected: " << invalid_info.error_message << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\\n=== Example completed successfully ===" << std::endl;
    return 0;
}
