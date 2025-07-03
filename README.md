# LicenseCore++

Modern C++ library for software licensing with HMAC signatures and hardware fingerprinting.

## Features

- 🔐 **HMAC-SHA256** license signatures
- 🖥️ **Hardware fingerprinting** (CPU ID, MAC, Volume Serial)
- ⚡ **Fast validation** with minimal dependencies
- 🔧 **Easy integration** - just link static library
- 🚀 **Cross-platform** (Windows, macOS, Linux)
- 📦 **Extensible** - ready for RSA, server validation, CRL

## Quick Start

### Build

```bash
mkdir build && cd build
cmake ..
make -j4
```

### Usage

```cpp
#include <license_core/license_manager.hpp>

using namespace license_core;

int main() {
    // Initialize with secret key
    LicenseManager manager("your-secret-key-here");
    
    // Validate license
    auto info = manager.load_and_validate(license_json);
    
    if (info.valid && manager.has_feature("premium")) {
        // License is valid and has premium feature
        std::cout << "Welcome, " << info.user_id << "!" << std::endl;
    }
    
    return 0;
}
```

### License Format

```json
{
  "user_id": "user-123",
  "license_id": "lic-456",
  "expiry": "2025-12-31T23:59:59Z",
  "issued_at": "2024-01-01T00:00:00Z",
  "hardware_hash": "a1b2c3d4e5f6...",
  "features": ["basic", "premium", "api"],
  "version": 1,
  "hmac_signature": "c4ef45e6..."
}
```

## API Reference

### LicenseManager

Main class for license validation:

- `LicenseInfo load_and_validate(const std::string& license_json)` - Validate license
- `bool has_feature(const std::string& feature)` - Check if feature is available
- `std::string get_current_hwid()` - Get current hardware ID
- `std::string generate_license(const LicenseInfo& info)` - Generate signed license

### HardwareFingerprint

Hardware identification:

- `std::string get_fingerprint()` - Get combined hardware hash
- `std::string get_cpu_id()` - Get CPU identifier
- `std::string get_mac_address()` - Get primary MAC address

### HMACValidator

Signature operations:

- `std::string sign(const std::string& data)` - Sign data with HMAC
- `bool verify(const std::string& data, const std::string& signature)` - Verify signature

## Building

### Dependencies

- **CMake** 3.16+
- **OpenSSL** (for HMAC-SHA256)
- **C++17** compatible compiler

### Platforms

| Platform | Status | Notes |
|----------|--------|-------|
| Windows  | ✅     | MSVC 2019+, MinGW |
| macOS    | ✅     | Xcode 12+ |
| Linux    | ✅     | GCC 9+, Clang 10+ |

### Options

```bash
cmake -DLICENSECORE_BUILD_SHARED=ON    # Build shared library
cmake -DLICENSECORE_BUILD_EXAMPLES=OFF # Skip examples
cmake -DLICENSECORE_BUILD_TESTS=OFF    # Skip tests
```

## Integration

### CMake

```cmake
find_package(LicenseCore REQUIRED)
target_link_libraries(your_app LicenseCore::licensecore)
```

### Manual

```cpp
// Include headers
#include <license_core/license_manager.hpp>

// Link libraries
// -llicensecore -lssl -lcrypto
```

## License

Commercial license. Contact for pricing and terms.

## Roadmap

- [ ] RSA signature support
- [ ] Online license validation
- [ ] Certificate Revocation List (CRL)
- [ ] Hardware binding levels
- [ ] License analytics

---

**LicenseCore++** - Professional software licensing made simple.
