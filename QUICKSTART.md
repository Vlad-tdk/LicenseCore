# Quick Start

## 1. Build library

```bash
cmake -S . -B build/release \
  -DCMAKE_BUILD_TYPE=Release \
  -DLICENSECORE_BUILD_EXAMPLES=OFF \
  -DLICENSECORE_BUILD_TESTS=OFF \
  -DLICENSECORE_BUILD_GTESTS=OFF

cmake --build build/release -j
```

## 2. Use in your app

```cpp
#include <license_core/license_manager.hpp>

int main() {
    license_core::LicenseManager manager("your-strong-secret-key");
    auto result = manager.load_and_validate(license_json);
    return result.valid ? 0 : 1;
}
```

## 3. Enable examples/tests (optional)

```bash
cmake -S . -B build/dev \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLICENSECORE_BUILD_EXAMPLES=ON \
  -DLICENSECORE_BUILD_TESTS=ON \
  -DLICENSECORE_BUILD_GTESTS=ON

cmake --build build/dev -j
ctest --test-dir build/dev --output-on-failure
```
