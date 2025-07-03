#include "license_core/hardware_fingerprint.hpp"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <functional>
#include <thread>
#include <unistd.h>  // For getpid() and gethostname()

#ifdef _WIN32
    #include <windows.h>
    #include <iphlpapi.h>
    #include <intrin.h>
    #include <comdef.h>
    #include <wbemidl.h>
    #pragma comment(lib, "wbemuuid.lib")
    #pragma comment(lib, "iphlpapi.lib")
#elif __APPLE__
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <ifaddrs.h>
    #include <net/if_dl.h>
    #include <sys/mount.h>
    #include <sys/socket.h>
#else
    #include <fstream>
    #include <ifaddrs.h>
    #include <netpacket/packet.h>
    #include <sys/statvfs.h>
    #include <unistd.h>
#endif

namespace license_core {

HardwareFingerprint::HardwareFingerprint(const HardwareConfig& config) 
    : config_(config), cache_time_(std::chrono::steady_clock::time_point::min()) {
}

std::string HardwareFingerprint::get_fingerprint() const {
    if (!config_.enable_caching) {
        update_cache_stats(false);
        return compute_hash();
    }
    
    if (config_.thread_safe_cache) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        
        auto now = std::chrono::steady_clock::now();
        
        if (cached_fingerprint_.has_value() && !is_cache_expired()) {
            update_cache_stats(true);
            return cached_fingerprint_.value();
        }
        
        std::string result = compute_hash();
        cached_fingerprint_ = result;
        cache_time_ = now;
        cache_stats_.last_update = now;
        update_cache_stats(false);
        return result;
    } else {
        auto now = std::chrono::steady_clock::now();
        
        if (cached_fingerprint_.has_value() && !is_cache_expired()) {
            update_cache_stats(true);
            return cached_fingerprint_.value();
        }
        
        std::string result = compute_hash();
        cached_fingerprint_ = result;
        cache_time_ = now;
        cache_stats_.last_update = now;
        update_cache_stats(false);
        return result;
    }
}

std::string HardwareFingerprint::get_fingerprint_safe() const noexcept {
    try {
        return get_fingerprint();
    } catch (...) {
        return "";
    }
}

std::string HardwareFingerprint::get_cpu_id() const {
    if (!config_.use_cpu_id) return "";
    
    try {
        std::string result = get_cpu_id_impl();
        if (result.empty()) {
            throw HardwareDetectionException("CPU ID detection returned empty result");
        }
        return result;
    } catch (const HardwareDetectionException&) {
        throw;
    } catch (const std::exception& e) {
        throw HardwareDetectionException("CPU ID detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_cpu_id_safe() const noexcept {
    try {
        return get_cpu_id();
    } catch (...) {
        return "";
    }
}

std::string HardwareFingerprint::get_mac_address() const {
    if (!config_.use_mac_address) return "";
    
    try {
        std::string result = get_mac_address_impl();
        if (result.empty()) {
            throw HardwareDetectionException("MAC address detection returned empty result");
        }
        return result;
    } catch (const HardwareDetectionException&) {
        throw;
    } catch (const std::exception& e) {
        throw HardwareDetectionException("MAC address detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_mac_address_safe() const noexcept {
    try {
        return get_mac_address();
    } catch (...) {
        return "";
    }
}

std::string HardwareFingerprint::get_volume_serial() const {
    if (!config_.use_volume_serial) return "";
    
    try {
        std::string result = get_volume_serial_impl();
        if (result.empty()) {
            throw HardwareDetectionException("Volume serial detection returned empty result");
        }
        return result;
    } catch (const HardwareDetectionException&) {
        throw;
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Volume serial detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_motherboard_serial() const {
    if (!config_.use_motherboard_serial) return "";
    
    try {
        std::string result = get_motherboard_serial_impl();
        if (result.empty()) {
            throw HardwareDetectionException("Motherboard serial detection returned empty result");
        }
        return result;
    } catch (const HardwareDetectionException&) {
        throw;
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Motherboard serial detection failed: " + std::string(e.what()));
    }
}

bool HardwareFingerprint::is_cache_expired() const {
    auto now = std::chrono::steady_clock::now();
    return (now - cache_time_) > config_.cache_lifetime;
}

void HardwareFingerprint::update_cache_stats(bool cache_hit) const {
    if (cache_hit) {
        cache_stats_.cache_hits++;
    } else {
        cache_stats_.cache_misses++;
    }
}

void HardwareFingerprint::clear_cache() const {
    if (config_.thread_safe_cache) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        cached_fingerprint_.reset();
        cached_cpu_id_.reset();
        cached_mac_address_.reset();
        cached_volume_serial_.reset();
        cached_motherboard_serial_.reset();
        cache_time_ = std::chrono::steady_clock::time_point::min();
    } else {
        cached_fingerprint_.reset();
        cached_cpu_id_.reset();
        cached_mac_address_.reset();
        cached_volume_serial_.reset();
        cached_motherboard_serial_.reset();
        cache_time_ = std::chrono::steady_clock::time_point::min();
    }
}

void HardwareFingerprint::invalidate_cache() const {
    clear_cache();
}

bool HardwareFingerprint::is_cache_valid() const {
    if (config_.thread_safe_cache) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return cached_fingerprint_.has_value() && !is_cache_expired();
    } else {
        return cached_fingerprint_.has_value() && !is_cache_expired();
    }
}

std::chrono::steady_clock::time_point HardwareFingerprint::get_cache_time() const {
    if (config_.thread_safe_cache) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return cache_time_;
    } else {
        return cache_time_;
    }
}

HardwareFingerprint::CacheStats HardwareFingerprint::get_cache_stats() const {
    if (config_.thread_safe_cache) {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        return cache_stats_;
    } else {
        return cache_stats_;
    }
}

std::string HardwareFingerprint::compute_hash() const {
    std::stringstream combined;
    std::vector<std::string> errors;
    bool has_any_data = false;
    
    if (config_.use_cpu_id) {
        try {
            std::string cpu_id = get_cpu_id_impl();
            if (!cpu_id.empty()) {
                combined << cpu_id << "|";
                has_any_data = true;
            } else {
                errors.push_back("CPU ID empty");
            }
        } catch (const std::exception& e) {
            errors.push_back("CPU ID: " + std::string(e.what()));
        }
    }
    
    if (config_.use_mac_address) {
        try {
            std::string mac = get_mac_address_impl();
            if (!mac.empty()) {
                combined << mac << "|";
                has_any_data = true;
            } else {
                errors.push_back("MAC address empty");
            }
        } catch (const std::exception& e) {
            errors.push_back("MAC address: " + std::string(e.what()));
        }
    }
    
    if (config_.use_volume_serial) {
        try {
            std::string volume = get_volume_serial_impl();
            if (!volume.empty()) {
                combined << volume << "|";
                has_any_data = true;
            } else {
                errors.push_back("Volume serial empty");
            }
        } catch (const std::exception& e) {
            errors.push_back("Volume serial: " + std::string(e.what()));
        }
    }
    
    if (config_.use_motherboard_serial) {
        try {
            std::string mobo = get_motherboard_serial_impl();
            if (!mobo.empty()) {
                combined << mobo << "|";
                has_any_data = true;
            } else {
                errors.push_back("Motherboard serial empty");
            }
        } catch (const std::exception& e) {
            errors.push_back("Motherboard serial: " + std::string(e.what()));
        }
    }
    
    if (!has_any_data) {
        std::string error_msg = "Failed to collect any hardware data";
        if (!errors.empty()) {
            error_msg += ": ";
            for (size_t i = 0; i < errors.size(); ++i) {
                if (i > 0) error_msg += ", ";
                error_msg += errors[i];
            }
        }
        throw HardwareDetectionException(error_msg);
    }
    
    std::string data = combined.str();
    
    if (!data.empty() && data.back() == '|') {
        data.pop_back();
    }
    
    try {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        if (SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), hash) == nullptr) {
            throw CryptographicException("SHA256 hash computation failed");
        }
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
            ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
        }
        
        return ss.str();
    } catch (const std::exception& e) {
        throw CryptographicException("Hash computation error: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::generate_secure_fallback(const std::string& prefix) const {
    try {
        std::stringstream fallback_data;
        
        fallback_data << prefix << "|";
        
        auto now = std::chrono::high_resolution_clock::now();
        fallback_data << now.time_since_epoch().count() << "|";
        
#ifdef _WIN32
        fallback_data << GetCurrentProcessId() << "|";
#else
        fallback_data << getpid() << "|";
#endif
        
        fallback_data << std::hash<std::thread::id>{}(std::this_thread::get_id()) << "|";
        
        try {
            std::string hostname = get_hostname();
            fallback_data << hostname << "|";
        } catch (...) {
            fallback_data << "unknown-host|";
        }
        
        unsigned char random_bytes[16];
        if (RAND_bytes(random_bytes, sizeof(random_bytes)) == 1) {
            for (size_t i = 0; i < sizeof(random_bytes); ++i) {
                fallback_data << std::hex << std::setfill('0') << std::setw(2) 
                             << static_cast<unsigned int>(random_bytes[i]);
            }
        } else {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            for (int i = 0; i < 16; ++i) {
                fallback_data << std::hex << std::setfill('0') << std::setw(2) << dis(gen);
            }
        }
        
        std::string combined_data = fallback_data.str();
        unsigned char hash[SHA256_DIGEST_LENGTH];
        if (SHA256(reinterpret_cast<const unsigned char*>(combined_data.c_str()), 
                   combined_data.length(), hash) == nullptr) {
            throw CryptographicException("SHA256 fallback hash failed");
        }
        
        std::stringstream result;
        result << prefix << "-";
        for (int i = 0; i < 6; ++i) {
            result << std::hex << std::setfill('0') << std::setw(2) 
                   << static_cast<unsigned int>(hash[i]);
        }
        
        return result.str();
        
    } catch (const std::exception& e) {
        return prefix + "-fallback-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    }
}

std::string HardwareFingerprint::get_hostname() const {
    char hostname[256] = {0};
#ifdef _WIN32
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        return std::string(hostname);
    }
#else
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    }
#endif
    throw HardwareDetectionException("Failed to get hostname");
}

#ifdef _WIN32

std::string HardwareFingerprint::get_cpu_id_impl() const {
    try {
        int cpuid[4];
        __cpuid(cpuid, 0);
        
        std::stringstream ss;
        ss << std::hex << cpuid[1] << cpuid[3] << cpuid[2];
        std::string result = ss.str();
        
        if (result.empty() || result == "0") {
            return generate_secure_fallback("cpu");
        }
        
        return result;
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Windows CPU ID detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_mac_address_impl() const {
    IP_ADAPTER_ADDRESSES* adapter_addresses = nullptr;
    DWORD adapter_addresses_buffer_size = 16 * 1024;
    
    try {
        for (int attempts = 0; attempts != 3; ++attempts) {
            adapter_addresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(malloc(adapter_addresses_buffer_size));
            if (!adapter_addresses) {
                throw HardwareDetectionException("Failed to allocate memory for adapter addresses");
            }
            
            DWORD error = GetAdaptersAddresses(
                AF_UNSPEC,
                GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                nullptr,
                adapter_addresses,
                &adapter_addresses_buffer_size);
            
            if (ERROR_SUCCESS == error) {
                break;
            }
            
            free(adapter_addresses);
            adapter_addresses = nullptr;
            
            if (ERROR_BUFFER_OVERFLOW != error) {
                throw HardwareDetectionException("GetAdaptersAddresses failed with error: " + std::to_string(error));
            }
        }
        
        if (!adapter_addresses) {
            return generate_secure_fallback("mac");
        }
        
        std::string mac_address;
        for (IP_ADAPTER_ADDRESSES* adapter = adapter_addresses; adapter; adapter = adapter->Next) {
            if (adapter->PhysicalAddressLength == 6) {
                std::stringstream ss;
                ss << std::hex << std::setfill('0');
                for (DWORD i = 0; i < adapter->PhysicalAddressLength; ++i) {
                    ss << std::setw(2) << static_cast<unsigned int>(adapter->PhysicalAddress[i]);
                }
                mac_address = ss.str();
                break;
            }
        }
        
        free(adapter_addresses);
        return mac_address.empty() ? generate_secure_fallback("mac") : mac_address;
        
    } catch (const std::exception& e) {
        if (adapter_addresses) {
            free(adapter_addresses);
        }
        throw HardwareDetectionException("Windows MAC address detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_volume_serial_impl() const {
    try {
        DWORD serial_number;
        if (GetVolumeInformationA("C:\\", nullptr, 0, &serial_number, nullptr, nullptr, nullptr, 0)) {
            std::stringstream ss;
            ss << std::hex << serial_number;
            return ss.str();
        } else {
            throw HardwareDetectionException("GetVolumeInformation failed for C: drive");
        }
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Windows volume serial detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_motherboard_serial_impl() const {
    return generate_secure_fallback("mobo");
}

#elif __APPLE__

std::string HardwareFingerprint::get_cpu_id_impl() const {
    try {
        char buffer[256];
        size_t size = sizeof(buffer);
        
        if (sysctlbyname("hw.uuid", buffer, &size, nullptr, 0) == 0) {
            return std::string(buffer);
        }
        
        size = sizeof(buffer);
        if (sysctlbyname("hw.model", buffer, &size, nullptr, 0) == 0) {
            return std::string(buffer);
        }
        
        return generate_secure_fallback("cpu");
        
    } catch (const std::exception& e) {
        throw HardwareDetectionException("macOS CPU ID detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_mac_address_impl() const {
    struct ifaddrs *ifaddrs_ptr = nullptr;
    
    try {
        if (getifaddrs(&ifaddrs_ptr) != 0) {
            return generate_secure_fallback("mac");
        }
        
        std::string mac_address;
        for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
                struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa->ifa_addr;
                if (sdl->sdl_alen == 6 && sdl->sdl_nlen > 0) {
                    std::string if_name(sdl->sdl_data, sdl->sdl_nlen);
                    if (if_name.find("en") == 0 || if_name.find("eth") == 0) {
                        unsigned char* ptr = (unsigned char*)(sdl->sdl_data + sdl->sdl_nlen);
                        std::stringstream ss;
                        ss << std::hex << std::setfill('0');
                        for (int i = 0; i < 6; ++i) {
                            ss << std::setw(2) << static_cast<unsigned int>(ptr[i]);
                        }
                        mac_address = ss.str();
                        break;
                    }
                }
            }
        }
        
        freeifaddrs(ifaddrs_ptr);
        return mac_address.empty() ? generate_secure_fallback("mac") : mac_address;
        
    } catch (const std::exception& e) {
        if (ifaddrs_ptr) {
            freeifaddrs(ifaddrs_ptr);
        }
        throw HardwareDetectionException("macOS MAC address detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_volume_serial_impl() const {
    try {
        struct statfs fs_info;
        if (statfs("/", &fs_info) == 0) {
            std::stringstream ss;
            ss << std::hex << fs_info.f_fsid.val[0] << fs_info.f_fsid.val[1];
            return ss.str();
        } else {
            throw HardwareDetectionException("statfs failed for root filesystem");
        }
    } catch (const std::exception& e) {
        throw HardwareDetectionException("macOS volume serial detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_motherboard_serial_impl() const {
    return generate_secure_fallback("mobo");
}

#else // Linux

std::string HardwareFingerprint::get_cpu_id_impl() const {
    try {
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (cpuinfo.is_open()) {
            std::string line;
            while (std::getline(cpuinfo, line)) {
                if (line.find("Serial") != std::string::npos) {
                    size_t colon_pos = line.find(':');
                    if (colon_pos != std::string::npos) {
                        std::string serial = line.substr(colon_pos + 1);
                        serial.erase(0, serial.find_first_not_of(" \t"));
                        serial.erase(serial.find_last_not_of(" \t") + 1);
                        if (!serial.empty()) {
                            return serial;
                        }
                    }
                }
            }
        }
        
        std::ifstream machine_id("/etc/machine-id");
        if (machine_id.is_open()) {
            std::string id;
            std::getline(machine_id, id);
            if (!id.empty()) {
                return id;
            }
        }
        
        return generate_secure_fallback("cpu");
        
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Linux CPU ID detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_mac_address_impl() const {
    struct ifaddrs *ifaddrs_ptr = nullptr;
    
    try {
        if (getifaddrs(&ifaddrs_ptr) != 0) {
            return generate_secure_fallback("mac");
        }
        
        std::string mac_address;
        for (struct ifaddrs *ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET) {
                struct sockaddr_ll* s = (struct sockaddr_ll*)ifa->ifa_addr;
                if (s->sll_halen == 6) {
                    std::stringstream ss;
                    ss << std::hex << std::setfill('0');
                    for (int i = 0; i < 6; ++i) {
                        ss << std::setw(2) << static_cast<unsigned int>(s->sll_addr[i]);
                    }
                    mac_address = ss.str();
                    break;
                }
            }
        }
        
        freeifaddrs(ifaddrs_ptr);
        return mac_address.empty() ? generate_secure_fallback("mac") : mac_address;
        
    } catch (const std::exception& e) {
        if (ifaddrs_ptr) {
            freeifaddrs(ifaddrs_ptr);
        }
        throw HardwareDetectionException("Linux MAC address detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_volume_serial_impl() const {
    try {
        std::ifstream mounts("/proc/mounts");
        if (mounts.is_open()) {
            std::string line;
            while (std::getline(mounts, line)) {
                if (line.find(" / ") != std::string::npos) {
                    size_t space_pos = line.find(' ');
                    if (space_pos != std::string::npos) {
                        std::string device = line.substr(0, space_pos);
                        
                        std::ifstream uuid_file("/proc/sys/kernel/random/boot_id");
                        if (uuid_file.is_open()) {
                            std::string uuid;
                            std::getline(uuid_file, uuid);
                            if (!uuid.empty()) {
                                return uuid;
                            }
                        }
                    }
                    break;
                }
            }
        }
        
        throw HardwareDetectionException("Could not determine volume serial");
        
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Linux volume serial detection failed: " + std::string(e.what()));
    }
}

std::string HardwareFingerprint::get_motherboard_serial_impl() const {
    try {
        std::ifstream dmi("/sys/class/dmi/id/board_serial");
        if (dmi.is_open()) {
            std::string serial;
            std::getline(dmi, serial);
            if (!serial.empty() && serial != "None" && serial != "To be filled by O.E.M.") {
                return serial;
            }
        }
        
        return generate_secure_fallback("mobo");
        
    } catch (const std::exception& e) {
        throw HardwareDetectionException("Linux motherboard serial detection failed: " + std::string(e.what()));
    }
}

#endif

} // namespace license_core
