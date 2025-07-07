# 🔒 LicenseCore++ Multi-Platform Package

## 📦 Libraries included:

### Available libraries:
- `liblicense_core_macos_x64.a` - macOS Intel (x86_64)
- `liblicense_core_macos_arm64.a` - macOS Apple Silicon (ARM64)  
- `liblicense_core_macos_universal.a` - macOS Universal (Intel + Apple Silicon)
- `liblicense_core_linux_x64.a` - Linux x86_64 (if Docker was available)
- `liblicense_core_windows_x64.a` - Windows x86_64 (if MinGW was available)

## 🚀 Quick Start:

```c
#include "license_core_pure_c.h"
#include <stdio.h>

int main() {
    // Test embedded license
    if (lc_validate_embedded()) {
        printf("✅ Embedded license valid!\n");
        
        if (lc_has_feature("basic")) {
            printf("✅ Basic features available\n");
        }
    }
    
    // Test external license
    const char* license = "{\"user_id\":\"demo\",\"features\":[\"basic\",\"premium\"]}";
    if (lc_validate_license(license)) {
        printf("✅ External license valid!\n");
    }
    
    // Get hardware ID
    printf("🖥️ Hardware ID: %s\n", lc_get_hwid());
    
    return 0;
}
```

## 🔗 Platform-specific compilation:

### macOS (recommended - works on all Macs):
```bash
gcc app.c -L./lib -llicense_core_macos_universal \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### macOS Intel only:
```bash
gcc app.c -L./lib -llicense_core_macos_x64 \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### macOS Apple Silicon only:
```bash
gcc app.c -L./lib -llicense_core_macos_arm64 \
  -framework IOKit -framework CoreFoundation -framework Security -o app
```

### Linux:
```bash
gcc app.c -L./lib -llicense_core_linux_x64 -o app
```

### Windows (MinGW):
```bash
gcc app.c -L./lib -llicense_core_windows_x64 -o app.exe
```

## 📁 Directory structure:
```
package/
├── lib/                    # Static libraries for all platforms
├── include/               # Header files
├── examples/              # Usage examples
├── docs/                  # Documentation
└── README.md             # This file
```

## 🔒 Security Features:
- ✅ Symbol obfuscation and hiding
- ✅ Anti-debug runtime protection  
- ✅ Embedded encrypted license
- ✅ Minimal API surface (4 functions only)
- ✅ Pure C implementation (no C++ dependencies)

## 🔧 API Reference:
- `lc_validate_license(json)` - Validate external license from JSON
- `lc_validate_embedded()` - Validate built-in embedded license
- `lc_has_feature(name)` - Check if specific feature is available
- `lc_get_hwid()` - Get unique hardware identifier

Built with LicenseCore++ Multi-Platform Builder
