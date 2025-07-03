#include <license_core/license_manager.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <cassert>

using namespace license_core;

class StressTester {
public:
    static void test_concurrent_validation() {
        std::cout << "🔥 Testing concurrent license validation..." << std::endl;
        
        const int num_threads = 100;
        const int validations_per_thread = 1000;
        
        LicenseManager manager("stress-test-key-2024");
        
        // Create a valid license
        LicenseInfo info;
        info.user_id = "stress-test-user";
        info.license_id = "stress-test-license";
        info.hardware_hash = manager.get_current_hwid();
        info.features = {"basic", "premium", "enterprise"};
        info.issued_at = std::chrono::system_clock::now();
        info.expiry = std::chrono::system_clock::now() + std::chrono::hours(24);
        info.version = 1;
        
        std::string license_json = manager.generate_license(info);
        
        std::vector<std::thread> threads;
        std::atomic<int> success_count{0};
        std::atomic<int> error_count{0};
        
        auto start_time = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&, i]() {
                try {
                    LicenseManager thread_manager("stress-test-key-2024");
                    
                    for (int j = 0; j < validations_per_thread; ++j) {
                        auto result = thread_manager.load_and_validate(license_json);
                        
                        if (result.valid) {
                            success_count++;
                            
                            // Test feature checking
                            assert(thread_manager.has_feature("premium"));
                            assert(!thread_manager.has_feature("nonexistent"));
                        } else {
                            error_count++;
                            std::cout << "❌ Thread " << i << " validation " << j 
                                     << " failed: " << result.error_message << std::endl;
                        }
                        
                        // Small random delay
                        if (j % 100 == 0) {
                            std::this_thread::sleep_for(std::chrono::microseconds(10));
                        }
                    }
                } catch (const std::exception& e) {
                    std::cout << "❌ Thread " << i << " crashed: " << e.what() << std::endl;
                    error_count += validations_per_thread;
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        int total_validations = num_threads * validations_per_thread;
        double validations_per_second = (total_validations * 1000.0) / duration.count();
        
        std::cout << "✅ Concurrent validation test completed:" << std::endl;
        std::cout << "   Threads: " << num_threads << std::endl;
        std::cout << "   Total validations: " << total_validations << std::endl;
        std::cout << "   Successful: " << success_count.load() << std::endl;
        std::cout << "   Errors: " << error_count.load() << std::endl;
        std::cout << "   Duration: " << duration.count() << "ms" << std::endl;
        std::cout << "   Performance: " << static_cast<int>(validations_per_second) << " validations/sec" << std::endl;
        
        assert(error_count.load() == 0);
        assert(success_count.load() == total_validations);
    }
    
    static void test_memory_pressure() {
        std::cout << "🧠 Testing memory pressure..." << std::endl;
        
        const int iterations = 10000;
        std::vector<std::unique_ptr<LicenseManager>> managers;
        
        auto start_memory = get_memory_usage();
        
        for (int i = 0; i < iterations; ++i) {
            auto manager = std::make_unique<LicenseManager>("memory-test-" + std::to_string(i));
            
            // Generate and validate license
            LicenseInfo info;
            info.user_id = "memory-test-user-" + std::to_string(i);
            info.license_id = "memory-test-license-" + std::to_string(i);
            info.hardware_hash = manager->get_current_hwid();
            info.features = {"feature1", "feature2", "feature3"};
            info.issued_at = std::chrono::system_clock::now();
            info.expiry = std::chrono::system_clock::now() + std::chrono::hours(1);
            
            std::string license = manager->generate_license(info);
            auto result = manager->load_and_validate(license);
            assert(result.valid);
            
            if (i % 1000 == 0) {
                managers.push_back(std::move(manager));
            }
            
            if (i % 1000 == 0) {
                auto current_memory = get_memory_usage();
                std::cout << "   Iteration " << i << ", Memory: " << current_memory << " MB" << std::endl;
            }
        }
        
        auto end_memory = get_memory_usage();
        std::cout << "✅ Memory pressure test completed:" << std::endl;
        std::cout << "   Start memory: " << start_memory << " MB" << std::endl;
        std::cout << "   End memory: " << end_memory << " MB" << std::endl;
        std::cout << "   Memory increase: " << (end_memory - start_memory) << " MB" << std::endl;
        
        // Memory increase should be reasonable (less than 50MB for 10k iterations)
        assert((end_memory - start_memory) < 50);
    }
    
    static void test_malformed_inputs() {
        std::cout << "🕷️ Testing malformed inputs..." << std::endl;
        
        LicenseManager manager("malformed-test-key");
        
        std::vector<std::string> malformed_licenses = {
            "",
            "not-json",
            "{}",
            "{\"invalid\": \"license\"}",
            "{\"user_id\": \"test\"}",  // Missing required fields
            "{\"user_id\": \"test\", \"expiry\": \"invalid-date\"}",
            R"({"user_id": "test", "expiry": "2025-01-01", "features": "not-array"})",
            R"({"user_id": "test", "expiry": "2025-01-01", "features": [], "hmac_signature": "invalid"})",
            std::string(10000, 'x'),  // Very long string
            "{\"user_id\": \"" + std::string(10000, 'y') + "\"}",  // Long field
            "{'single_quotes': 'invalid'}",  // Invalid JSON syntax
            "{\"unicode\": \"\\u0000\\u0001\\u0002\"}",  // Control characters
            "{\"nested\": {\"very\": {\"deep\": {\"object\": \"value\"}}}}",  // Deep nesting
        };
        
        int handled_correctly = 0;
        
        for (const auto& malformed : malformed_licenses) {
            try {
                auto result = manager.load_and_validate(malformed);
                
                if (!result.valid && !result.error_message.empty()) {
                    handled_correctly++;
                    std::cout << "   ✅ Correctly rejected: " << result.error_message.substr(0, 50) << std::endl;
                } else {
                    std::cout << "   ❌ Should have been rejected but wasn't" << std::endl;
                }
            } catch (const std::exception& e) {
                // Exceptions are also acceptable for malformed input
                handled_correctly++;
                std::cout << "   ✅ Exception caught: " << std::string(e.what()).substr(0, 50) << std::endl;
            }
        }
        
        std::cout << "✅ Malformed input test completed:" << std::endl;
        std::cout << "   Total tests: " << malformed_licenses.size() << std::endl;
        std::cout << "   Handled correctly: " << handled_correctly << std::endl;
        
        assert(handled_correctly == malformed_licenses.size());
    }
    
    static void test_hardware_fingerprint_consistency() {
        std::cout << "🔧 Testing hardware fingerprint consistency..." << std::endl;
        
        const int iterations = 1000;
        LicenseManager manager("hwid-test-key");
        
        std::string first_hwid = manager.get_current_hwid();
        assert(!first_hwid.empty());
        
        int consistent_count = 0;
        for (int i = 0; i < iterations; ++i) {
            std::string current_hwid = manager.get_current_hwid();
            
            if (current_hwid == first_hwid) {
                consistent_count++;
            } else {
                std::cout << "   ❌ HWID changed: " << first_hwid << " -> " << current_hwid << std::endl;
            }
            
            if (i % 100 == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        
        std::cout << "✅ Hardware fingerprint consistency test:" << std::endl;
        std::cout << "   Total checks: " << iterations << std::endl;
        std::cout << "   Consistent: " << consistent_count << std::endl;
        std::cout << "   HWID: " << first_hwid.substr(0, 16) << "..." << std::endl;
        
        // HWID should be 100% consistent
        assert(consistent_count == iterations);
    }
    
    static void test_performance_benchmarks() {
        std::cout << "⚡ Running performance benchmarks..." << std::endl;
        
        LicenseManager manager("perf-test-key");
        
        // Create test license
        LicenseInfo info;
        info.user_id = "perf-test-user";
        info.license_id = "perf-test-license";
        info.hardware_hash = manager.get_current_hwid();
        info.features = {"basic", "premium"};
        info.issued_at = std::chrono::system_clock::now();
        info.expiry = std::chrono::system_clock::now() + std::chrono::hours(1);
        
        std::string license_json = manager.generate_license(info);
        
        // Benchmark license generation
        const int gen_iterations = 10000;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < gen_iterations; ++i) {
            info.license_id = "perf-test-" + std::to_string(i);
            manager.generate_license(info);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto gen_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double gen_per_second = (gen_iterations * 1000000.0) / gen_duration.count();
        
        // Benchmark license validation
        const int val_iterations = 10000;
        start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < val_iterations; ++i) {
            auto result = manager.load_and_validate(license_json);
            assert(result.valid);
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto val_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double val_per_second = (val_iterations * 1000000.0) / val_duration.count();
        
        // Benchmark HWID generation
        const int hwid_iterations = 1000;
        start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < hwid_iterations; ++i) {
            manager.get_current_hwid();
        }
        
        end = std::chrono::high_resolution_clock::now();
        auto hwid_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double hwid_per_second = (hwid_iterations * 1000000.0) / hwid_duration.count();
        
        std::cout << "✅ Performance benchmarks completed:" << std::endl;
        std::cout << "   License generation: " << static_cast<int>(gen_per_second) << " ops/sec" << std::endl;
        std::cout << "   License validation: " << static_cast<int>(val_per_second) << " ops/sec" << std::endl;
        std::cout << "   HWID generation: " << static_cast<int>(hwid_per_second) << " ops/sec" << std::endl;
        
        // Performance requirements (adjust based on needs)
        assert(gen_per_second > 10000);  // At least 10K generations per second
        assert(val_per_second > 50000);  // At least 50K validations per second
        assert(hwid_per_second > 1000);  // At least 1K HWID generations per second
    }

private:
    static double get_memory_usage() {
        // Simplified memory usage getter (platform specific implementation needed)
        std::ifstream status("/proc/self/status");
        std::string line;
        while (std::getline(status, line)) {
            if (line.substr(0, 6) == "VmRSS:") {
                std::istringstream iss(line);
                std::string label, value, unit;
                iss >> label >> value >> unit;
                return std::stod(value) / 1024.0; // Convert KB to MB
            }
        }
        return 0.0;
    }
};

int main() {
    std::cout << "🚀 === LicenseCore++ Stress Testing ===" << std::endl;
    std::cout << "Running comprehensive stress tests..." << std::endl;
    std::cout << "" << std::endl;
    
    try {
        StressTester::test_hardware_fingerprint_consistency();
        std::cout << "" << std::endl;
        
        StressTester::test_malformed_inputs();
        std::cout << "" << std::endl;
        
        StressTester::test_performance_benchmarks();
        std::cout << "" << std::endl;
        
        StressTester::test_memory_pressure();
        std::cout << "" << std::endl;
        
        StressTester::test_concurrent_validation();
        std::cout << "" << std::endl;
        
        std::cout << "🎉 === ALL STRESS TESTS PASSED ===" << std::endl;
        std::cout << "LicenseCore++ is production ready!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Stress test failed: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
