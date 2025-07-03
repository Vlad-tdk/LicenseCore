#include "test_utils.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace license_core;
using namespace license_core::testing;
using namespace ::testing;

class HardwareFingerprintBasicTest : public HardwareFingerprintTest {};

// Test basic functionality
TEST_F(HardwareFingerprintBasicTest, GetFingerprint_ReturnsNonEmptyString) {
    std::string fingerprint = fingerprint_->get_fingerprint_safe();
    
    EXPECT_FALSE(fingerprint.empty());
    EXPECT_GE(fingerprint.length(), 32);  // At least 32 chars (SHA256 hex should be 64)
}

TEST_F(HardwareFingerprintBasicTest, GetFingerprint_IsConsistent) {
    std::string fp1 = fingerprint_->get_fingerprint_safe();
    std::string fp2 = fingerprint_->get_fingerprint_safe();
    
    EXPECT_EQ(fp1, fp2) << "Hardware fingerprint should be consistent";
}

TEST_F(HardwareFingerprintBasicTest, GetFingerprint_IsHexString) {
    std::string fingerprint = fingerprint_->get_fingerprint_safe();
    
    // Check if all characters are valid hex
    for (char c : fingerprint) {
        EXPECT_TRUE(std::isxdigit(c)) << "Fingerprint should contain only hex characters, found: " << c;
    }
}

// Test individual components
TEST_F(HardwareFingerprintBasicTest, GetCpuId_ReturnsValidData) {
    if (config_.use_cpu_id) {
        std::string cpu_id = fingerprint_->get_cpu_id_safe();
        EXPECT_FALSE(cpu_id.empty()) << "CPU ID should not be empty when enabled";
    }
}

TEST_F(HardwareFingerprintBasicTest, GetMacAddress_ReturnsValidData) {
    if (config_.use_mac_address) {
        std::string mac = fingerprint_->get_mac_address_safe();
        EXPECT_FALSE(mac.empty()) << "MAC address should not be empty when enabled";
    }
}

// Test configuration options
TEST_F(HardwareFingerprintBasicTest, DisabledComponents_ReturnEmpty) {
    HardwareConfig empty_config = TestUtils::CreateEmptyConfig();
    
    EXPECT_THROW({
        HardwareFingerprint empty_fingerprint(empty_config);
        empty_fingerprint.get_fingerprint();
    }, HardwareDetectionException) << "Should throw when no components are enabled";
}

TEST_F(HardwareFingerprintBasicTest, SafeVersions_NeverThrow) {
    HardwareConfig empty_config = TestUtils::CreateEmptyConfig();
    HardwareFingerprint empty_fingerprint(empty_config);
    
    // Safe versions should never throw
    EXPECT_NO_THROW({
        std::string fp = empty_fingerprint.get_fingerprint_safe();
        std::string cpu = empty_fingerprint.get_cpu_id_safe();
        std::string mac = empty_fingerprint.get_mac_address_safe();
        
        // They should return empty strings for invalid config
        EXPECT_TRUE(fp.empty());
        EXPECT_TRUE(cpu.empty());
        EXPECT_TRUE(mac.empty());
    });
}

// Test different hardware configurations
class HardwareConfigTest : public ::testing::Test {
protected:
    void TestConfigCombination(bool cpu, bool mac, bool volume, bool motherboard) {
        HardwareConfig config;
        config.use_cpu_id = cpu;
        config.use_mac_address = mac;
        config.use_volume_serial = volume;
        config.use_motherboard_serial = motherboard;
        config.enable_caching = false;  // Disable caching for pure testing
        
        bool should_work = cpu || mac || volume || motherboard;
        
        if (should_work) {
            EXPECT_NO_THROW({
                HardwareFingerprint fingerprint(config);
                std::string fp = fingerprint.get_fingerprint();
                EXPECT_FALSE(fp.empty());
            }) << "Should work with cpu=" << cpu << ", mac=" << mac 
               << ", volume=" << volume << ", motherboard=" << motherboard;
        } else {
            EXPECT_THROW({
                HardwareFingerprint fingerprint(config);
                fingerprint.get_fingerprint();
            }, HardwareDetectionException) 
            << "Should throw when no components enabled";
        }
    }
};

TEST_F(HardwareConfigTest, AllCombinations_WorkCorrectly) {
    // Test all 16 combinations of 4 boolean flags
    for (int i = 0; i < 16; ++i) {
        bool cpu = (i & 1) != 0;
        bool mac = (i & 2) != 0;
        bool volume = (i & 4) != 0;
        bool motherboard = (i & 8) != 0;
        
        TestConfigCombination(cpu, mac, volume, motherboard);
    }
}

// Test error conditions
class HardwareFingerprintErrorTest : public ::testing::Test {};

TEST_F(HardwareFingerprintErrorTest, ExceptionHierarchy_IsCorrect) {
    try {
        HardwareConfig empty_config = TestUtils::CreateEmptyConfig();
        HardwareFingerprint fingerprint(empty_config);
        fingerprint.get_fingerprint();
        FAIL() << "Should have thrown an exception";
    } catch (const HardwareDetectionException& e) {
        // Should catch as HardwareDetectionException
        EXPECT_TRUE(true);
    } catch (const LicenseException& e) {
        // Should also be catchable as base LicenseException
        EXPECT_TRUE(true);
    } catch (...) {
        FAIL() << "Caught unexpected exception type";
    }
}

TEST_F(HardwareFingerprintErrorTest, ErrorMessages_AreInformative) {
    try {
        HardwareConfig empty_config = TestUtils::CreateEmptyConfig();
        HardwareFingerprint fingerprint(empty_config);
        fingerprint.get_fingerprint();
        FAIL() << "Should have thrown an exception";
    } catch (const HardwareDetectionException& e) {
        std::string message = e.what();
        EXPECT_FALSE(message.empty());
        EXPECT_THAT(message, HasSubstr("hardware")) << "Error message should mention hardware";
    }
}

// Performance characteristics test
class HardwareFingerprintPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = TestUtils::CreateTestConfig(false);  // No caching for baseline
        fingerprint_ = std::make_unique<HardwareFingerprint>(config_);
    }
    
    HardwareConfig config_;
    std::unique_ptr<HardwareFingerprint> fingerprint_;
};

TEST_F(HardwareFingerprintPerformanceTest, BasicPerformance_IsReasonable) {
    // Warmup
    fingerprint_->get_fingerprint_safe();
    
    // Measure performance
    auto duration = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // Should complete within 30 seconds (very generous for unit tests)
    EXPECT_LT(duration.count(), 30000000) << "Fingerprint generation took too long: " 
                                          << duration.count() << " microseconds";
    
    // Should take at least some time (not instant, which would indicate caching)
    // Very relaxed timing requirement
    EXPECT_GT(duration.count(), 1) << "Fingerprint generation was suspiciously fast: " 
                                     << duration.count() << " microseconds";
}
