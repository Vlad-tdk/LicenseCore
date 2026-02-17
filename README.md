# LicenseCore

LicenseCore is a C++17 library for offline license validation with:
- HMAC-SHA256 signature checks
- hardware fingerprint binding
- cross-platform support (Windows, macOS, Linux)

## Requirements
- CMake 3.16+
- OpenSSL
- C++17 compiler

## Production Build

```bash
cmake -S . -B build/release \
  -DCMAKE_BUILD_TYPE=Release \
  -DLICENSECORE_BUILD_EXAMPLES=OFF \
  -DLICENSECORE_BUILD_TESTS=OFF \
  -DLICENSECORE_BUILD_GTESTS=OFF

cmake --build build/release -j
```

## Development Build

```bash
cmake -S . -B build/dev \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLICENSECORE_BUILD_EXAMPLES=ON \
  -DLICENSECORE_BUILD_TESTS=ON \
  -DLICENSECORE_BUILD_GTESTS=ON

cmake --build build/dev -j
ctest --test-dir build/dev --output-on-failure
```

## Quick API Example

```cpp
#include <license_core/license_manager.hpp>

int main() {
    license_core::LicenseManager manager("your-strong-secret-key");
    auto info = manager.load_and_validate(license_json);
    return info.valid ? 0 : 1;
}
```

## Install

```bash
cmake --install build/release
```

## CI/CD
- CI checks: `.github/workflows/ci.yml`
- Tagged release pipeline: `.github/workflows/release.yml` (trigger: `v*` tags)
- Release steps: `RELEASE_CHECKLIST.md`

## License
See `LICENSE`.
