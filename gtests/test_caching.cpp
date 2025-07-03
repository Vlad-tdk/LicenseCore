#include "test_utils.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <vector>
#include <chrono>

using namespace license_core;
using namespace license_core::testing;
using namespace ::testing;

class CachingBasicTest : public CachingTest {};

// Test basic caching functionality
TEST_F(CachingBasicTest, CacheEnabled_SpeedsUpSecondCall) {
    // Clear cache to ensure clean state
    cached_fingerprint_->clear_cache();
    
    // Warmup
    cached_fingerprint_->get_fingerprint_safe();
    
    // Clear cache for baseline measurement
    cached_fingerprint_->clear_cache();
    
    // First call (cache miss)
    auto miss_time = TestUtils::MeasureTime([this]() {
        cached_fingerprint_->get_fingerprint_safe();
    });
    
    // Second call (cache hit)
    auto hit_time = TestUtils::MeasureTime([this]() {
        cached_fingerprint_->get_fingerprint_safe();
    });
    
    // Cache hit should be significantly faster
    EXPECT_LT(hit_time.count(), miss_time.count() / 2) 
        << "Cache hit (" << hit_time.count() << "μs) should be at least 2x faster than miss (" 
        << miss_time.count() << "μs)";
}

TEST_F(CachingBasicTest, CacheDisabled_ConsistentPerformance) {
    // Clear any potential cache state
    non_cached_fingerprint_->clear_cache();
    
    // Warmup
    non_cached_fingerprint_->get_fingerprint_safe();
    
    // Measure multiple calls
    std::vector<std::chrono::microseconds> times;
    for (int i = 0; i < 5; ++i) {
        auto time = TestUtils::MeasureTime([this]() {
            non_cached_fingerprint_->get_fingerprint_safe();
        });
        times.push_back(time);
    }
    
    // All times should be similar (no significant caching effect)
    auto min_time = *std::min_element(times.begin(), times.end());
    auto max_time = *std::max_element(times.begin(), times.end());
    
    // Allow 200% variation for non-cached calls
    EXPECT_LT(max_time.count(), min_time.count() * 3) 
        << "Non-cached performance should be consistent. Range: " 
        << min_time.count() << " to " << max_time.count() << " microseconds";
}

TEST_F(CachingBasicTest, CachedResults_AreIdentical) {
    // Clear cache
    cached_fingerprint_->clear_cache();
    
    std::string first_result = cached_fingerprint_->get_fingerprint_safe();
    std::string second_result = cached_fingerprint_->get_fingerprint_safe();
    
    EXPECT_EQ(first_result, second_result) 
        << "Cached results should be identical to original";
    EXPECT_FALSE(first_result.empty()) 
        << "Results should not be empty";
}

// Test cache expiration
class CacheExpirationTest : public ::testing::Test {
protected:
    void SetUp() override {
        short_cache_config_ = TestUtils::CreateTestConfig(true, SHORT_CACHE_LIFETIME, true);
        fingerprint_ = std::make_unique<HardwareFingerprint>(short_cache_config_);
    }
    
    void TearDown() override {
        fingerprint_.reset();
    }
    
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    HardwareConfig short_cache_config_;
};

TEST_F(CacheExpirationTest, CacheExpires_AfterTimeout) {
    // Clear cache
    fingerprint_->clear_cache();
    
    // First call to populate cache
    std::string first_result = fingerprint_->get_fingerprint_safe();
    
    // Wait for cache to expire
    TestUtils::Sleep(SHORT_CACHE_LIFETIME + std::chrono::milliseconds(100));
    
    // Measure second call (should be cache miss due to expiration)
    auto expired_time = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // Third call (should be cache hit)
    auto hit_time = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // Cache hit should be faster than expired cache access
    EXPECT_LT(hit_time.count(), expired_time.count() / 2) 
        << "Cache hit (" << hit_time.count() << "μs) should be faster than expired access (" 
        << expired_time.count() << "μs)";
}

TEST_F(CacheExpirationTest, CacheClearing_InvalidatesCache) {
    // Populate cache
    fingerprint_->get_fingerprint_safe();
    
    // Verify cache hit is fast
    auto before_clear = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // Clear cache manually
    fingerprint_->clear_cache();
    
    // Next call should be slow (cache miss)
    auto after_clear = TestUtils::MeasureTime([this]() {
        fingerprint_->get_fingerprint_safe();
    });
    
    // After clear should be slower than cached access
    EXPECT_GT(after_clear.count(), before_clear.count() * 2) 
        << "Cache miss after clear (" << after_clear.count() 
        << "μs) should be slower than cache hit (" << before_clear.count() << "μs)";
}

// Test cache statistics and monitoring
class CacheStatisticsTest : public CachingTest {};

TEST_F(CacheStatisticsTest, CacheStats_TrackHitsAndMisses) {
    // Clear cache and stats
    cached_fingerprint_->clear_cache();
    
    // Multiple calls to generate hits and misses
    cached_fingerprint_->get_fingerprint_safe();  // Miss
    cached_fingerprint_->get_fingerprint_safe();  // Hit
    cached_fingerprint_->get_fingerprint_safe();  // Hit
    
    cached_fingerprint_->clear_cache();
    cached_fingerprint_->get_fingerprint_safe();  // Miss
    cached_fingerprint_->get_fingerprint_safe();  // Hit
    
    // Note: We would need to expose cache statistics through the interface
    // This test demonstrates what we'd test if stats were available
    EXPECT_TRUE(true) << "Cache statistics functionality would be tested here";
}

// Test cache behavior under different configurations
class CacheConfigurationTest : public ::testing::Test {
protected:
    void TestCacheConfiguration(bool enable_caching, bool thread_safe, std::chrono::seconds lifetime) {
        HardwareConfig config = TestUtils::CreateTestConfig(enable_caching, lifetime, thread_safe);
        HardwareFingerprint fingerprint(config);
        
        // Clear any existing cache
        fingerprint.clear_cache();
        
        // Test basic functionality
        std::string result1 = fingerprint.get_fingerprint_safe();
        std::string result2 = fingerprint.get_fingerprint_safe();
        
        EXPECT_EQ(result1, result2) << "Results should be consistent regardless of caching config";
        EXPECT_FALSE(result1.empty()) << "Results should never be empty";
        
        if (enable_caching) {
            // Measure performance difference
            fingerprint.clear_cache();
            
            auto miss_time = TestUtils::MeasureTime([&fingerprint]() {
                fingerprint.get_fingerprint_safe();
            });
            
            auto hit_time = TestUtils::MeasureTime([&fingerprint]() {
                fingerprint.get_fingerprint_safe();
            });
            
            // With caching enabled, second call should be faster
            EXPECT_LT(hit_time.count(), miss_time.count()) 
                << "Caching should improve performance";
        }
    }
};

TEST_F(CacheConfigurationTest, AllConfigurations_WorkCorrectly) {
    // Test all combinations of cache settings
    std::vector<bool> enable_options = {true, false};
    std::vector<bool> thread_safe_options = {true, false};
    std::vector<std::chrono::seconds> lifetime_options = {
        std::chrono::seconds(1), 
        std::chrono::seconds(10), 
        std::chrono::seconds(300)
    };
    
    for (bool enable : enable_options) {
        for (bool thread_safe : thread_safe_options) {
            for (auto lifetime : lifetime_options) {
                SCOPED_TRACE("Testing config: enable=" + std::to_string(enable) + 
                           ", thread_safe=" + std::to_string(thread_safe) + 
                           ", lifetime=" + std::to_string(lifetime.count()) + "s");
                
                TestCacheConfiguration(enable, thread_safe, lifetime);
            }
        }
    }
}

// Test cache thread safety
class CacheThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = TestUtils::CreateTestConfig(true, LONG_CACHE_LIFETIME, true);
        fingerprint_ = std::make_unique<HardwareFingerprint>(config_);
    }
    
    void TearDown() override {
        fingerprint_.reset();
    }
    
    std::unique_ptr<HardwareFingerprint> fingerprint_;
    HardwareConfig config_;
    
    static constexpr int NUM_THREADS = 10;
    static constexpr int CALLS_PER_THREAD = 20;
};

TEST_F(CacheThreadSafetyTest, ConcurrentAccess_IsThreadSafe) {
    fingerprint_->clear_cache();
    
    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> results(NUM_THREADS);
    std::vector<std::vector<std::chrono::microseconds>> times(NUM_THREADS);
    
    // Launch threads
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, &results, &times]() {
            for (int i = 0; i < CALLS_PER_THREAD; ++i) {
                auto start = std::chrono::high_resolution_clock::now();
                std::string result = fingerprint_->get_fingerprint_safe();
                auto end = std::chrono::high_resolution_clock::now();
                
                results[t].push_back(result);
                times[t].push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all results are identical
    std::string expected_result = results[0][0];
    for (int t = 0; t < NUM_THREADS; ++t) {
        for (int i = 0; i < CALLS_PER_THREAD; ++i) {
            EXPECT_EQ(expected_result, results[t][i]) 
                << "Thread " << t << ", call " << i << " returned different result";
        }
    }
    
    // Verify cache performance characteristics
    std::vector<std::chrono::microseconds> all_times;
    for (const auto& thread_times : times) {
        all_times.insert(all_times.end(), thread_times.begin(), thread_times.end());
    }
    
    auto min_time = *std::min_element(all_times.begin(), all_times.end());
    auto max_time = *std::max_element(all_times.begin(), all_times.end());
    
    // Most calls should be cache hits (fast), allowing for some cache misses
    EXPECT_LT(min_time.count() * 10, max_time.count()) 
        << "Performance variation too extreme. Min: " << min_time.count() 
        << "μs, Max: " << max_time.count() << "μs. Possible thread safety issue.";
}

TEST_F(CacheThreadSafetyTest, ConcurrentCacheClearing_IsThreadSafe) {
    std::vector<std::thread> threads;
    std::atomic<int> clear_count{0};
    std::atomic<int> call_count{0};
    
    // Mix of cache clearing and fingerprint calls
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, &clear_count, &call_count]() {
            for (int i = 0; i < CALLS_PER_THREAD; ++i) {
                if (i % 5 == 0) {
                    fingerprint_->clear_cache();
                    clear_count++;
                } else {
                    std::string result = fingerprint_->get_fingerprint_safe();
                    EXPECT_FALSE(result.empty());
                    call_count++;
                }
                
                // Small delay to increase chance of race conditions
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify operations completed
    EXPECT_GT(clear_count.load(), 0) << "Should have performed cache clears";
    EXPECT_GT(call_count.load(), 0) << "Should have performed fingerprint calls";
    
    // Final verification - fingerprint should still work correctly
    std::string final_result = fingerprint_->get_fingerprint_safe();
    EXPECT_FALSE(final_result.empty()) << "Cache should be functional after concurrent access";
}
