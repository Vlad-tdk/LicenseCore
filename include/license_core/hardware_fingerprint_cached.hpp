#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <mutex>
#include "exceptions.hpp"

namespace license_core {

// Hardware configuration structure
struct HardwareConfig {
    bool use_cpu_id = true;
    bool use_mac_address = true;
    bool use_volume_serial = true;
    bool use_motherboard_serial = false;
    
    // Caching configuration
    std::chrono::minutes cache_lifetime{5}; // Default 5 minutes
    bool enable_caching = true;
    bool thread_safe_cache = true; // Enable mutex protection
};

class HardwareFingerprint {
public:
    explicit HardwareFingerprint(const HardwareConfig& config = HardwareConfig{});
    
    // Get hardware fingerprint as hash string - throws HardwareDetectionException on failure
    std::string get_fingerprint() const;
    
    // Get individual components (for debugging) - may throw HardwareDetectionException
    std::string get_cpu_id() const;
    std::string get_mac_address() const;
    std::string get_volume_serial() const;
    std::string get_motherboard_serial() const;
    
    // Combine all enabled components into single hash - throws on failure
    std::string compute_hash() const;
    
    // Safe versions that return empty string on failure instead of throwing
    std::string get_fingerprint_safe() const noexcept;
    std::string get_cpu_id_safe() const noexcept;
    std::string get_mac_address_safe() const noexcept;
    
    // Cache management
    void clear_cache() const;
    void invalidate_cache() const;
    bool is_cache_valid() const;
    std::chrono::steady_clock::time_point get_cache_time() const;
    
    // Cache statistics
    struct CacheStats {
        size_t cache_hits = 0;
        size_t cache_misses = 0;
        std::chrono::steady_clock::time_point last_update;
        double hit_rate() const { 
            return cache_hits + cache_misses > 0 ? 
                static_cast<double>(cache_hits) / (cache_hits + cache_misses) : 0.0; 
        }
    };
    
    CacheStats get_cache_stats() const;
    
private:
    HardwareConfig config_;
    
    // Caching members
    mutable std::optional<std::string> cached_fingerprint_;
    mutable std::optional<std::string> cached_cpu_id_;
    mutable std::optional<std::string> cached_mac_address_;
    mutable std::optional<std::string> cached_volume_serial_;
    mutable std::optional<std::string> cached_motherboard_serial_;
    mutable std::chrono::steady_clock::time_point cache_time_;
    mutable CacheStats cache_stats_;
    mutable std::mutex cache_mutex_; // For thread safety
    
    // Helper methods for cache management
    bool is_cache_expired() const;
    void update_cache_stats(bool cache_hit) const;
    template<typename T>
    std::string get_cached_or_compute(std::optional<std::string>& cache, T compute_func, const std::string& component_name) const;
    
    // Platform-specific implementations
    std::string get_cpu_id_impl() const;
    std::string get_mac_address_impl() const;
    std::string get_volume_serial_impl() const;
    std::string get_motherboard_serial_impl() const;
    
    // Security: Generate cryptographically secure fallback identifiers
    std::string generate_secure_fallback(const std::string& prefix) const;
    std::string get_hostname() const;
};

} // namespace license_core
