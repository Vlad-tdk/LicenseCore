#include <license_core/license_manager.hpp>
#include <license_core/hmac_validator.hpp>
#include <license_core/hardware_fingerprint.hpp>
#include <iostream>
#include <cassert>
#include <chrono>

using namespace license_core;

bool test_hmac_validator() {
    std::cout << "Testing HMAC Validator..." << std::endl;
    
    HMACValidator validator("test-secret-key");
    
    // Test basic sign/verify
    std::string data = "test data for signing";
    std::string signature = validator.sign(data);
    
    assert(!signature.empty());
    assert(validator.verify(data, signature));
    assert(!validator.verify("different data", signature));
    assert(!validator.verify(data, "wrong-signature"));
    
    std::cout << "✅ HMAC Validator tests passed" << std::endl;
    return true;
}

bool test_hardware_fingerprint() {
    std::cout << "Testing Hardware Fingerprint..." << std::endl;
    
    HardwareConfig config;
    HardwareFingerprint fingerprint(config);
    
    std::string fp1 = fingerprint.get_fingerprint();
    std::string fp2 = fingerprint.get_fingerprint();
    
    // Should be consistent
    assert(fp1 == fp2);
    assert(!fp1.empty());
    
    // Test individual components
    std::string cpu_id = fingerprint.get_cpu_id();
    std::string mac = fingerprint.get_mac_address();
    
    // At least one component should work
    assert(!cpu_id.empty() || !mac.empty());
    
    std::cout << "✅ Hardware Fingerprint tests passed" << std::endl;
    return true;
}

bool test_license_manager() {
    std::cout << "Testing License Manager..." << std::endl;
    
    LicenseManager manager("test-secret-key-2024");
    
    // Get current hardware ID
    std::string hwid = manager.get_current_hwid();
    assert(!hwid.empty());
    
    // Create valid license
    LicenseInfo license_info;
    license_info.user_id = "test-user";
    license_info.license_id = "test-license-123";
    license_info.hardware_hash = hwid;
    license_info.features = {"basic", "premium"};
    license_info.issued_at = std::chrono::system_clock::now();
    license_info.expiry = std::chrono::system_clock::now() + std::chrono::hours(24);
    license_info.version = 1;
    
    // Generate license
    std::string license_json = manager.generate_license(license_info);
    assert(!license_json.empty());
    
    // Validate license
    auto validated = manager.load_and_validate(license_json);
    assert(validated.valid);
    assert(validated.user_id == "test-user");
    assert(validated.license_id == "test-license-123");
    assert(validated.features.size() == 2);
    
    // Test feature checking
    assert(manager.has_feature("basic"));
    assert(manager.has_feature("premium"));
    assert(!manager.has_feature("enterprise"));
    
    // Test expired license
    LicenseInfo expired_license = license_info;
    expired_license.expiry = std::chrono::system_clock::now() - std::chrono::hours(1);
    std::string expired_json = manager.generate_license(expired_license);
    auto expired_validated = manager.load_and_validate(expired_json);
    assert(!expired_validated.valid);
    assert(expired_validated.error_message.find("expired") != std::string::npos);
    
    // Test wrong hardware hash
    LicenseInfo wrong_hw_license = license_info;
    wrong_hw_license.hardware_hash = "wrong-hardware-hash";
    std::string wrong_hw_json = manager.generate_license(wrong_hw_license);
    auto wrong_hw_validated = manager.load_and_validate(wrong_hw_json);
    assert(!wrong_hw_validated.valid);
    assert(wrong_hw_validated.error_message.find("mismatch") != std::string::npos);
    
    std::cout << "✅ License Manager tests passed" << std::endl;
    return true;
}

bool test_invalid_signatures() {
    std::cout << "Testing Invalid Signatures..." << std::endl;
    
    LicenseManager manager("test-secret-key");
    
    // Test tampered license
    std::string tampered_license = R"({
        "user_id": "hacker",
        "license_id": "fake-license",
        "expiry": "2030-12-31T23:59:59Z",
        "hardware_hash": ")" + manager.get_current_hwid() + R"(",
        "features": ["premium", "enterprise"],
        "version": "1",
        "hmac_signature": "fake-signature-12345"
    })";
    
    auto result = manager.load_and_validate(tampered_license);
    assert(!result.valid);
    assert(result.error_message.find("signature") != std::string::npos);
    
    std::cout << "✅ Invalid signature tests passed" << std::endl;
    return true;
}

int main() {
    std::cout << "=== LicenseCore Simple Tests ===" << std::endl;
    
    try {
        assert(test_hmac_validator());
        assert(test_hardware_fingerprint());
        assert(test_license_manager());
        assert(test_invalid_signatures());
        
        std::cout << "\\n🎉 All tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
