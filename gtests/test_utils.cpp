#include "test_utils.hpp"
#include <random>
#include <thread>
#include <algorithm>
#include <ctime>

namespace license_core {
namespace testing {

// TestUtils implementation
HardwareConfig TestUtils::CreateTestConfig(bool enable_caching, 
                                          std::chrono::seconds lifetime,
                                          bool thread_safe) {
    HardwareConfig config;
    config.enable_caching = enable_caching;
    config.cache_lifetime = lifetime;
    config.thread_safe_cache = thread_safe;
    config.use_cpu_id = true;
    config.use_mac_address = true;
    config.use_volume_serial = false;  // Disable for consistent testing
    config.use_motherboard_serial = false;  // Disable for consistent testing
    return config;
}

HardwareConfig TestUtils::CreateEmptyConfig() {
    HardwareConfig config;
    config.use_cpu_id = false;
    config.use_mac_address = false;
    config.use_volume_serial = false;
    config.use_motherboard_serial = false;
    config.enable_caching = false;
    return config;
}

// Explicit instantiation for common use cases
// (Implementation moved to header file)

LicenseInfo TestUtils::CreateTestLicense(const std::string& hwid) {
    LicenseInfo info;
    info.user_id = "test_user_" + RandomString(8);
    info.license_id = "test_license_" + RandomString(12);
    info.hardware_hash = hwid;
    info.features = {"feature1", "feature2", "test_feature"};
    
    auto now = std::chrono::system_clock::now();
    info.issued_at = now;
    info.expiry = now + std::chrono::hours(24 * 365);  // 1 year
    info.version = 1;
    info.valid = true;
    
    return info;
}

std::string TestUtils::RandomString(size_t length) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 61);
    
    std::string chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += chars[dis(gen)];
    }
    
    return result;
}

void TestUtils::Sleep(std::chrono::milliseconds duration) {
    std::this_thread::sleep_for(duration);
}

bool TestUtils::ApproximatelyEqual(std::chrono::microseconds a, 
                                 std::chrono::microseconds b,
                                 double tolerance_percent) {
    auto diff = std::abs(a.count() - b.count());
    auto avg = (a.count() + b.count()) / 2.0;
    return (diff / avg) * 100.0 <= tolerance_percent;
}

// HardwareFingerprintTest implementation
void HardwareFingerprintTest::SetUp() {
    config_ = TestUtils::CreateTestConfig();
    fingerprint_ = std::make_unique<HardwareFingerprint>(config_);
}

void HardwareFingerprintTest::TearDown() {
    fingerprint_.reset();
}

// CachingTest implementation
void CachingTest::SetUp() {
    cached_config_ = TestUtils::CreateTestConfig(true, LONG_CACHE_LIFETIME, true);
    non_cached_config_ = TestUtils::CreateTestConfig(false, LONG_CACHE_LIFETIME, true);
    
    cached_fingerprint_ = std::make_unique<HardwareFingerprint>(cached_config_);
    non_cached_fingerprint_ = std::make_unique<HardwareFingerprint>(non_cached_config_);
}

void CachingTest::TearDown() {
    cached_fingerprint_.reset();
    non_cached_fingerprint_.reset();
}

// PerformanceTest implementation
void PerformanceTest::SetUp() {
    config_ = TestUtils::CreateTestConfig(true, LONG_CACHE_LIFETIME, true);
    fingerprint_ = std::make_unique<HardwareFingerprint>(config_);
}

void PerformanceTest::TearDown() {
    fingerprint_.reset();
}

std::pair<std::chrono::microseconds, std::chrono::microseconds> 
PerformanceTest::BenchmarkCachePerformance() {
    // Clear cache to ensure clean state
    fingerprint_->clear_cache();
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; ++i) {
        fingerprint_->get_fingerprint_safe();
    }
    
    // Clear cache for first measurement
    fingerprint_->clear_cache();
    
    // Measure cache miss (first call)
    auto miss_time = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // Measure cache hit (subsequent call)
    auto hit_time = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    return {miss_time, hit_time};
}

// ThreadSafetyTest implementation
void ThreadSafetyTest::SetUp() {
    config_ = TestUtils::CreateTestConfig(true, LONG_CACHE_LIFETIME, true);
    fingerprint_ = std::make_unique<HardwareFingerprint>(config_);
}

void ThreadSafetyTest::TearDown() {
    fingerprint_.reset();
}

} // namespace testing
} // namespace license_core
