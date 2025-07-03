#include <license_core/license_manager.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace license_core;

void print_usage() {
    std::cout << "Usage: license_generator [options]\\n";
    std::cout << "Options:\\n";
    std::cout << "  --user-id <id>        User identifier\\n";
    std::cout << "  --secret-key <key>    Secret key for signing\\n";
    std::cout << "  --hardware-hash <hw>  Hardware hash (or 'auto' for current)\\n";
    std::cout << "  --features <f1,f2>    Comma-separated features\\n";
    std::cout << "  --days <n>            License validity in days (default: 365)\\n";
    std::cout << "  --help                Show this help\\n";
}

std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

int main(int argc, char* argv[]) {
    std::string user_id;
    std::string secret_key;
    std::string hardware_hash;
    std::vector<std::string> features;
    int validity_days = 365;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            print_usage();
            return 0;
        }
        else if (arg == "--user-id" && i + 1 < argc) {
            user_id = argv[++i];
        }
        else if (arg == "--secret-key" && i + 1 < argc) {
            secret_key = argv[++i];
        }
        else if (arg == "--hardware-hash" && i + 1 < argc) {
            hardware_hash = argv[++i];
        }
        else if (arg == "--features" && i + 1 < argc) {
            features = split_string(argv[++i], ',');
        }
        else if (arg == "--days" && i + 1 < argc) {
            validity_days = std::stoi(argv[++i]);
        }
    }
    
    // Validate required parameters
    if (user_id.empty() || secret_key.empty()) {
        std::cerr << "Error: --user-id and --secret-key are required\\n";
        print_usage();
        return 1;
    }
    
    try {
        LicenseManager manager(secret_key);
        
        // Get hardware hash
        if (hardware_hash.empty() || hardware_hash == "auto") {
            hardware_hash = manager.get_current_hwid();
            std::cout << "Using current hardware hash: " << hardware_hash << std::endl;
        }
        
        // Default features if none specified
        if (features.empty()) {
            features = {"basic"};
        }
        
        // Create license info
        LicenseInfo license_info;
        license_info.user_id = user_id;
        license_info.license_id = "lic-" + std::to_string(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        license_info.hardware_hash = hardware_hash;
        license_info.features = features;
        license_info.issued_at = std::chrono::system_clock::now();
        license_info.expiry = std::chrono::system_clock::now() + 
                              std::chrono::hours(24 * validity_days);
        license_info.version = 1;
        
        // Generate license
        std::string license_json = manager.generate_license(license_info);
        
        std::cout << "\\n=== Generated License ===\\n";
        std::cout << license_json << std::endl;
        
        // Verify the generated license
        auto verified = manager.load_and_validate(license_json);
        if (verified.valid) {
            std::cout << "\\n✅ License verification: PASSED\\n";
        } else {
            std::cout << "\\n❌ License verification: FAILED - " << verified.error_message << "\\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
