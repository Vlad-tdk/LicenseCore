# LicenseCore++ Quick Start

## 🚀 Build & Test (5 minutes)

```bash
# Clone/download the project
cd LicenseCore

# Build everything
chmod +x build.sh
./build.sh

# Test the examples
cd build
./examples/simple_example
./examples/hwid_tool
./examples/license_generator --help
```

## 📦 Integration

### CMake Project
```cmake
find_package(LicenseCore REQUIRED)
target_link_libraries(your_app LicenseCore::licensecore)
```

### Manual Integration
```cpp
#include <license_core/license_manager.hpp>
// Link: -llicensecore -lssl -lcrypto
```

## 💡 Basic Usage

```cpp
#include <license_core/license_manager.hpp>

// 1. Initialize
LicenseManager manager("your-secret-key");

// 2. Generate license (server-side)
LicenseInfo info;
info.user_id = "customer-123";
info.hardware_hash = manager.get_current_hwid();
info.features = {"basic", "premium"};
info.expiry = now + 365_days;
std::string license = manager.generate_license(info);

// 3. Validate license (client-side)
auto result = manager.load_and_validate(license);
if (result.valid && manager.has_feature("premium")) {
    // License OK, premium features available
}
```

## 🔧 Dependencies

- **OpenSSL** (for HMAC-SHA256)
- **C++17** compiler
- **CMake 3.16+**

### Install Dependencies

**macOS:**
```bash
brew install openssl cmake
```

**Ubuntu/Debian:**
```bash
sudo apt install libssl-dev cmake build-essential
```

**Windows:**
```bash
vcpkg install openssl
```

## 📋 Next Steps

1. **Customize hardware fingerprinting** - see `HardwareConfig`
2. **Integrate into your app** - see `examples/simple_example.cpp`
3. **Deploy license server** - use `license_generator` tool
4. **Add obfuscation** - protect your secret key
5. **Consider DRM upgrade** - RSA signatures, online validation

## 🆘 Common Issues

**Build fails with OpenSSL not found:**
```bash
# macOS
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig"

# Linux
sudo apt install pkg-config

# Windows
set OpenSSL_ROOT_DIR=C:\vcpkg\installed\x64-windows
```

**Hardware fingerprint empty:**
- Check platform-specific permissions
- Some components require admin rights
- Virtual machines may have limited HW info

---

**Ready to ship? Get your commercial license at licensecore.tech**
