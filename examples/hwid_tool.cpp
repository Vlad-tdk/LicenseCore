#include <license_core/hardware_fingerprint.hpp>
#include <license_core/license_manager.hpp>
#include <iostream>

using namespace license_core;

int main() {
    std::cout << "=== Hardware Fingerprint Tool ===" << std::endl;
    
    try {
        // Default configuration
        HardwareConfig config;
        HardwareFingerprint fingerprint(config);
        
        std::cout << "\\nHardware Components:" << std::endl;
        std::cout << "- CPU ID: " << fingerprint.get_cpu_id() << std::endl;
        std::cout << "- MAC Address: " << fingerprint.get_mac_address() << std::endl;
        std::cout << "- Volume Serial: " << fingerprint.get_volume_serial() << std::endl;
        std::cout << "- Motherboard Serial: " << fingerprint.get_motherboard_serial() << std::endl;
        
        std::cout << "\\nCombined Fingerprint:" << std::endl;
        std::cout << fingerprint.get_fingerprint() << std::endl;
        
        // Test different configurations
        std::cout << "\\n=== Different Configurations ===" << std::endl;
        
        // CPU + MAC only
        HardwareConfig cpu_mac_config;
        cpu_mac_config.use_volume_serial = false;
        cpu_mac_config.use_motherboard_serial = false;
        HardwareFingerprint cpu_mac_fp(cpu_mac_config);
        std::cout << "CPU + MAC only: " << cpu_mac_fp.get_fingerprint() << std::endl;
        
        // All components including motherboard
        HardwareConfig all_config;
        all_config.use_motherboard_serial = true;
        HardwareFingerprint all_fp(all_config);
        std::cout << "All components: " << all_fp.get_fingerprint() << std::endl;
        
        // Using LicenseManager
        std::cout << "\\n=== Via LicenseManager ===" << std::endl;
        LicenseManager manager("dummy-key");
        std::cout << "Default HWID: " << manager.get_current_hwid() << std::endl;
        
        manager.set_hardware_config(cpu_mac_config);
        std::cout << "CPU+MAC HWID: " << manager.get_current_hwid() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
