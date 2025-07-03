#include "test_utils.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <random>

using namespace license_core;
using namespace license_core::testing;
using namespace ::testing;

class ThreadSafetyBasicTest : public ThreadSafetyTest {};

// Test basic thread safety of fingerprint generation
TEST_F(ThreadSafetyBasicTest, ConcurrentFingerprinting_IsThreadSafe) {
    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> results(NUM_THREADS);
    std::atomic<int> errors{0};
    
    // Launch threads
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, &results, &errors]() {
            try {
                for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                    std::string result = fingerprint_->get_fingerprint_safe();
                    results[t].push_back(result);
                }
            } catch (...) {
                errors++;
            }
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no errors occurred
    EXPECT_EQ(errors.load(), 0) << "Thread safety violations detected";
    
    // Verify all results are identical
    std::string expected_result;
    bool first_result = true;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        EXPECT_EQ(results[t].size(), ITERATIONS_PER_THREAD) 
            << "Thread " << t << " didn't complete all iterations";
        
        for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
            if (first_result) {
                expected_result = results[t][i];
                first_result = false;
                EXPECT_FALSE(expected_result.empty()) << "First result should not be empty";
            } else {
                EXPECT_EQ(expected_result, results[t][i]) 
                    << "Thread " << t << ", iteration " << i << " returned different result";
            }
        }
    }
}

TEST_F(ThreadSafetyBasicTest, ConcurrentCacheOperations_AreThreadSafe) {
    std::vector<std::thread> threads;
    std::atomic<int> cache_clears{0};
    std::atomic<int> fingerprint_calls{0};
    std::atomic<int> errors{0};
    
    // Mix of cache operations and fingerprint calls
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, &cache_clears, &fingerprint_calls, &errors]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 10);
            
            try {
                for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                    int operation = dis(gen);
                    
                    if (operation <= 2) {  // 20% chance to clear cache
                        fingerprint_->clear_cache();
                        cache_clears++;
                    } else {  // 80% chance to get fingerprint
                        std::string result = fingerprint_->get_fingerprint_safe();
                        if (!result.empty()) {
                            fingerprint_calls++;
                        }
                    }
                    
                    // Small random delay to increase chance of race conditions
                    std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
                }
            } catch (...) {
                errors++;
            }
        });
    }
    
    // Wait for completion
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no errors and operations completed
    EXPECT_EQ(errors.load(), 0) << "Concurrent cache operations caused errors";
    EXPECT_GT(cache_clears.load(), 0) << "Should have performed cache clears";
    EXPECT_GT(fingerprint_calls.load(), 0) << "Should have performed fingerprint calls";
    
    // Final verification - system should still work
    std::string final_result = fingerprint_->get_fingerprint_safe();
    EXPECT_FALSE(final_result.empty()) << "System should work after concurrent operations";
}

// Test thread safety under heavy load
class ThreadSafetyLoadTest : public ThreadSafetyTest {
protected:
    static constexpr int HEAVY_NUM_THREADS = 16;
    static constexpr int HEAVY_ITERATIONS = 100;
};

TEST_F(ThreadSafetyLoadTest, HighConcurrency_MaintainsThreadSafety) {
    std::vector<std::future<std::vector<std::string>>> futures;
    
    // Use async to create high concurrency
    for (int t = 0; t < HEAVY_NUM_THREADS; ++t) {
        futures.push_back(std::async(std::launch::async, [this]() {
            std::vector<std::string> thread_results;
            
            for (int i = 0; i < HEAVY_ITERATIONS; ++i) {
                std::string result = fingerprint_->get_fingerprint_safe();
                thread_results.push_back(result);
            }
            
            return thread_results;
        }));
    }
    
    // Collect all results
    std::vector<std::vector<std::string>> all_results;
    for (auto& future : futures) {
        try {
            all_results.push_back(future.get());
        } catch (const std::exception& e) {
            FAIL() << "Thread threw exception: " << e.what();
        }
    }
    
    // Verify consistency
    EXPECT_EQ(all_results.size(), HEAVY_NUM_THREADS) << "All threads should complete";
    
    std::string expected_result = all_results[0][0];
    EXPECT_FALSE(expected_result.empty()) << "Results should not be empty";
    
    for (size_t t = 0; t < all_results.size(); ++t) {
        EXPECT_EQ(all_results[t].size(), HEAVY_ITERATIONS) 
            << "Thread " << t << " should complete all iterations";
        
        for (size_t i = 0; i < all_results[t].size(); ++i) {
            EXPECT_EQ(expected_result, all_results[t][i]) 
                << "Inconsistent result at thread " << t << ", iteration " << i;
        }
    }
}

TEST_F(ThreadSafetyLoadTest, ConcurrentCacheEviction_IsThreadSafe) {
    // Use short cache lifetime to test cache eviction under load
    HardwareConfig short_config = TestUtils::CreateTestConfig(true, std::chrono::seconds(1), true);
    HardwareFingerprint short_cache_fingerprint(short_config);
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_calls{0};
    std::atomic<int> errors{0};
    
    auto start_time = std::chrono::steady_clock::now();
    auto test_duration = std::chrono::seconds(3);  // Run for 3 seconds
    
    for (int t = 0; t < HEAVY_NUM_THREADS; ++t) {
        threads.emplace_back([&short_cache_fingerprint, &successful_calls, &errors, start_time, test_duration]() {
            try {
                while (std::chrono::steady_clock::now() - start_time < test_duration) {
                    std::string result = short_cache_fingerprint.get_fingerprint_safe();
                    if (!result.empty()) {
                        successful_calls++;
                    }
                    
                    // Small delay
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            } catch (...) {
                errors++;
            }
        });
    }
    
    // Wait for test completion
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify no errors and successful operations
    EXPECT_EQ(errors.load(), 0) << "Cache eviction under load caused errors";
    EXPECT_GT(successful_calls.load(), 0) << "Should have successful calls during cache eviction test";
}

// Test thread safety of different components
class ComponentThreadSafetyTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_ = TestUtils::CreateTestConfig(true, LONG_CACHE_LIFETIME, true);
        secret_key_ = DEFAULT_TEST_SECRET;
        hardware_id_ = "test-hardware-id";
    }
    
    HardwareConfig config_;
    std::string secret_key_;
    std::string hardware_id_;
    
    static constexpr int COMPONENT_THREADS = 6;
    static constexpr int COMPONENT_ITERATIONS = 30;
};

TEST_F(ComponentThreadSafetyTest, HMACValidator_IsThreadSafe) {
    HMACValidator validator(secret_key_);
    LicenseInfo test_license = TestUtils::CreateTestLicense(hardware_id_);
    
    std::vector<std::thread> threads;
    std::atomic<int> validation_successes{0};
    std::atomic<int> validation_failures{0};
    std::atomic<int> errors{0};
    
    for (int t = 0; t < COMPONENT_THREADS; ++t) {
        threads.emplace_back([&validator, &test_license, this, &validation_successes, &validation_failures, &errors]() {
            try {
                for (int i = 0; i < COMPONENT_ITERATIONS; ++i) {
                    try {
                        bool result = validator.validate_license(test_license, hardware_id_);
                        if (result) {
                            validation_successes++;
                        } else {
                            validation_failures++;
                        }
                    } catch (const LicenseValidationException&) {
                        validation_failures++;  // Expected for some test cases
                    }
                }
            } catch (...) {
                errors++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(errors.load(), 0) << "HMAC validation thread safety errors";
    EXPECT_GT(validation_successes.load() + validation_failures.load(), 0) 
        << "Should have completed validations";
}

TEST_F(ComponentThreadSafetyTest, LicenseManager_IsThreadSafe) {
    LicenseManager manager(secret_key_);
    
    std::vector<std::thread> threads;
    std::atomic<int> operations{0};
    std::atomic<int> errors{0};
    
    for (int t = 0; t < COMPONENT_THREADS; ++t) {
        threads.emplace_back([&manager, this, &operations, &errors]() {
            try {
                for (int i = 0; i < COMPONENT_ITERATIONS; ++i) {
                    // Test different license manager operations
                    try {
                        // Generate a license
                        LicenseInfo info = TestUtils::CreateTestLicense(hardware_id_);
                        std::string license_json = manager.generate_license(info);
                        
                        // Validate the license
                        bool valid = manager.validate_license(license_json, hardware_id_);
                        
                        if (!license_json.empty()) {
                            operations++;
                        }
                    } catch (const LicenseException&) {
                        // Some failures expected in concurrent testing
                        operations++;
                    }
                }
            } catch (...) {
                errors++;
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(errors.load(), 0) << "License manager thread safety errors";
    EXPECT_GT(operations.load(), 0) << "Should have completed operations";
}

// Test memory consistency under concurrent access
class MemoryConsistencyTest : public ThreadSafetyTest {};

TEST_F(MemoryConsistencyTest, ConcurrentAccess_MaintainsMemoryConsistency) {
    std::vector<std::thread> threads;
    std::vector<std::atomic<bool>> thread_completed(NUM_THREADS);
    
    // Initialize completion flags
    for (auto& completed : thread_completed) {
        completed.store(false);
    }
    
    // Launch threads with different access patterns
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, &thread_completed]() {
            // Different access patterns per thread
            for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                if (t % 3 == 0) {
                    // Reader thread
                    std::string result = fingerprint_->get_fingerprint_safe();
                    EXPECT_FALSE(result.empty());
                } else if (t % 3 == 1) {
                    // Cache manipulator thread
                    if (i % 5 == 0) {
                        fingerprint_->clear_cache();
                    }
                    std::string result = fingerprint_->get_fingerprint_safe();
                    EXPECT_FALSE(result.empty());
                } else {
                    // Mixed operations thread
                    if (i % 7 == 0) {
                        fingerprint_->clear_cache();
                    }
                    std::string result = fingerprint_->get_fingerprint_safe();
                    EXPECT_FALSE(result.empty());
                    
                    // Small delay to increase scheduling variety
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            
            thread_completed[t].store(true);
        });
    }
    
    // Monitor progress and ensure no deadlocks
    auto start_time = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(30);  // 30 second timeout
    
    bool all_completed = false;
    while (!all_completed && (std::chrono::steady_clock::now() - start_time) < timeout) {
        all_completed = true;
        for (const auto& completed : thread_completed) {
            if (!completed.load()) {
                all_completed = false;
                break;
            }
        }
        
        if (!all_completed) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    // Wait for threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_TRUE(all_completed) << "All threads should complete without deadlock";
    
    // Final verification
    std::string final_result = fingerprint_->get_fingerprint_safe();
    EXPECT_FALSE(final_result.empty()) << "System should be functional after concurrent access";
}

// Test for race conditions
class RaceConditionTest : public ThreadSafetyTest {};

TEST_F(RaceConditionTest, NoRaceConditions_InCacheAccess) {
    std::vector<std::thread> threads;
    std::atomic<int> cache_hits{0};
    std::atomic<int> cache_misses{0};
    
    // Deliberately try to create race conditions
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t, &cache_hits, &cache_misses]() {
            for (int i = 0; i < ITERATIONS_PER_THREAD; ++i) {
                // Alternating pattern to maximize chance of races
                if ((t + i) % 2 == 0) {
                    fingerprint_->clear_cache();  // Force cache miss
                }
                
                auto start = std::chrono::high_resolution_clock::now();
                std::string result = fingerprint_->get_fingerprint_safe();
                auto end = std::chrono::high_resolution_clock::now();
                
                EXPECT_FALSE(result.empty()) << "Result should never be empty";
                
                // Classify as hit or miss based on timing (rough heuristic)
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                if (duration.count() < 1000) {  // Less than 1ms = likely cache hit
                    cache_hits++;
                } else {
                    cache_misses++;
                }
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // We should have both hits and misses (indicating cache is working)
    EXPECT_GT(cache_hits.load(), 0) << "Should have some cache hits";
    EXPECT_GT(cache_misses.load(), 0) << "Should have some cache misses";
    
    // Total operations should match expected
    int total_operations = cache_hits.load() + cache_misses.load();
    EXPECT_EQ(total_operations, NUM_THREADS * ITERATIONS_PER_THREAD) 
        << "All operations should be accounted for";
}
