#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <functional>
#include "license_core/hardware_fingerprint.hpp"
#include "license_core/exceptions.hpp"

using namespace license_core;

void print_cache_stats(const HardwareFingerprint& fingerprint) {
    auto stats = fingerprint.get_cache_stats();
    std::cout << "Cache Stats - Hits: " << stats.cache_hits 
              << ", Misses: " << stats.cache_misses 
              << ", Hit Rate: " << std::fixed << std::setprecision(1) 
              << (stats.hit_rate() * 100) << "%" << std::endl;
}

void measure_time(const std::string& description, std::function<void()> func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << description << ": " << duration.count() << " μs" << std::endl;
}

int main() {
    try {
        std::cout << "=== Hardware Fingerprint Caching Demo ===" << std::endl;
        
        // Test 1: Default caching (5 minutes)
        std::cout << "\n1. Testing default caching..." << std::endl;
        {
            HardwareConfig config;
            config.enable_caching = true;
            config.cache_lifetime = std::chrono::seconds(300); // 5 minutes
            
            HardwareFingerprint fingerprint(config);
            std::string fp1, fp2, fp3;
            
            // First call - cache miss
            measure_time("First call", [&]() {
                fp1 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            // Second call - cache hit
            measure_time("Second call", [&]() {
                fp2 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            // Third call - cache hit
            measure_time("Third call", [&]() {
                fp3 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            std::cout << "Results identical: " << (fp1 == fp2 && fp2 == fp3 ? "YES" : "NO") << std::endl;
            std::cout << "Fingerprint: " << fp1.substr(0, 16) << "..." << std::endl;
        }
        
        // Test 2: Disabled caching
        std::cout << "\n2. Testing disabled caching..." << std::endl;
        {
            HardwareConfig config;
            config.enable_caching = false;
            
            HardwareFingerprint fingerprint(config);
            std::string fp1, fp2;
            
            measure_time("First call (no cache)", [&]() {
                fp1 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            measure_time("Second call (no cache)", [&]() {
                fp2 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            std::cout << "Results identical: " << (fp1 == fp2 ? "YES" : "NO") << std::endl;
        }
        
        // Test 3: Short cache lifetime
        std::cout << "\n3. Testing short cache lifetime (1 second)..." << std::endl;
        {
            HardwareConfig config;
            config.enable_caching = true;
            config.cache_lifetime = std::chrono::seconds(1);
            
            HardwareFingerprint fingerprint(config);
            std::string fp1, fp2, fp3;
            
            // First call
            measure_time("First call", [&]() {
                fp1 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            // Second call (cache hit)
            measure_time("Second call (immediate)", [&]() {
                fp2 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            // Wait for cache expiry
            std::cout << "Waiting 1.5s for cache expiry..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            
            // Third call (cache miss due to expiry)
            measure_time("Third call (after expiry)", [&]() {
                fp3 = fingerprint.get_fingerprint_safe();
            });
            print_cache_stats(fingerprint);
            
            std::cout << "Results identical: " << (fp1 == fp2 && fp2 == fp3 ? "YES" : "NO") << std::endl;
        }
        
        // Test 4: Cache management
        std::cout << "\n4. Testing cache management..." << std::endl;
        {
            HardwareConfig config;
            config.enable_caching = true;
            config.cache_lifetime = std::chrono::seconds(600); // 10 minutes
            
            HardwareFingerprint fingerprint(config);
            
            // Build cache
            std::string fp1 = fingerprint.get_fingerprint_safe();
            print_cache_stats(fingerprint);
            std::cout << "Cache valid: " << (fingerprint.is_cache_valid() ? "YES" : "NO") << std::endl;
            
            // Clear cache
            fingerprint.clear_cache();
            std::cout << "After clear - Cache valid: " << (fingerprint.is_cache_valid() ? "YES" : "NO") << std::endl;
            print_cache_stats(fingerprint);
            
            // Rebuild cache
            std::string fp2 = fingerprint.get_fingerprint_safe();
            print_cache_stats(fingerprint);
            std::cout << "Cache valid: " << (fingerprint.is_cache_valid() ? "YES" : "NO") << std::endl;
            
            std::cout << "Results identical: " << (fp1 == fp2 ? "YES" : "NO") << std::endl;
        }
        
        std::cout << "\n=== Caching demo completed ===" << std::endl;
        
    } catch (const LicenseException& e) {
        std::cerr << "LicenseCore error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
