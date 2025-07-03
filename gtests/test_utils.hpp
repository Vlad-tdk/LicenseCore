#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>
#include <string>
#include <vector>
#include <memory>

#include "license_core/hardware_fingerprint.hpp"
#include "license_core/license_manager.hpp"
#include "license_core/hmac_validator.hpp"
#include "license_core/exceptions.hpp"

namespace license_core {
namespace testing {

// Test constants
constexpr auto DEFAULT_TEST_SECRET = "test-secret-key-for-unit-tests-12345";
constexpr auto SHORT_CACHE_LIFETIME = std::chrono::seconds(1);
constexpr auto LONG_CACHE_LIFETIME = std::chrono::seconds(300);

// Test utilities class
class TestUtils {
public:
    // Create test hardware config
    static HardwareConfig CreateTestConfig(bool enable_caching = true, 
                                         std::chrono::seconds lifetime = LONG_CACHE_LIFETIME,
                                         bool thread_safe = true);
    
    // Create test hardware config with all components disabled
    static HardwareConfig CreateEmptyConfig();
    
    // Measure execution time of a function
    template<typename Func>
    static std::chrono::microseconds MeasureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        // If we get zero time, it means the operation was extremely fast
        // Return 1 microsecond as minimum measurable time to avoid test failures
        return duration.count() == 0 ? std::chrono::microseconds(1) : duration;
    }
    
    // Create test license info
    static LicenseInfo CreateTestLicense(const std::string& hwid = "test-hardware-id");
    
    // Generate random string
    static std::string RandomString(size_t length = 16);
    
    // Sleep for specified duration
    static void Sleep(std::chrono::milliseconds duration);
    
    // Check if two durations are approximately equal (within tolerance)
    static bool ApproximatelyEqual(std::chrono::microseconds a, 
                                 std::chrono::microseconds b,
                                 double tolerance_percent = 50.0);
    
    // Measure execution time with minimum threshold to avoid zero measurements
    template<typename Func>
    static std::chrono::microseconds MeasureTimeWithMinimum(Func&& func, int iterations = 1) {
        std::chrono::microseconds total_time(0);
        
        for (int i = 0; i < iterations; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            total_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        }
        
        // Return average time per iteration, but ensure we have at least 1 microsecond
        auto avg_time = total_time / iterations;
        return avg_time.count() == 0 ? std::chrono::microseconds(1) : avg_time;
    }
};

// Custom matchers for Google Test
MATCHER_P(DurationLessThan, expected, 
          std::string("Duration is ") + (negation ? "not " : "") + "less than " + 
          std::to_string(expected.count()) + " microseconds") {
    return arg < expected;
}

MATCHER_P(DurationGreaterThan, expected,
          std::string("Duration is ") + (negation ? "not " : "") + "greater than " + 
          std::to_string(expected.count()) + " microseconds") {
    return arg > expected;
}

MATCHER_P2(DurationBetween, min_duration, max_duration,
           std::string("Duration is ") + (negation ? "not " : "") + "between " + 
           std::to_string(min_duration.count()) + " and " + 
           std::to_string(max_duration.count()) + " microseconds") {
    return arg >= min_duration && arg <= max_duration;
}

// Test fixture for hardware fingerprint tests
class HardwareFingerprintTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    HardwareConfig config_;
};

// Test fixture for caching tests
class CachingTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    std::unique_ptr<HardwareFingerprint> cached_fingerprint_;
    std::unique_ptr<HardwareFingerprint> non_cached_fingerprint_;
    HardwareConfig cached_config_;
    HardwareConfig non_cached_config_;
};

// Test fixture for performance tests
class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    static constexpr int WARMUP_ITERATIONS = 5;
    static constexpr int BENCHMARK_ITERATIONS = 100;
    
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    HardwareConfig config_;
    
    // Helper to run performance benchmark
    std::pair<std::chrono::microseconds, std::chrono::microseconds> 
    BenchmarkCachePerformance();
};

// Test fixture for thread safety tests
class ThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    static constexpr int NUM_THREADS = 8;
    static constexpr int ITERATIONS_PER_THREAD = 50;
    
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    HardwareConfig config_;
};

} // namespace testing
} // namespace license_core
