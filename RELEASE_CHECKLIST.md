# Release Checklist

## Before Tag
1. Verify production build locally:
   - `cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DLICENSECORE_BUILD_EXAMPLES=OFF -DLICENSECORE_BUILD_TESTS=OFF -DLICENSECORE_BUILD_GTESTS=OFF`
   - `cmake --build build/release -j`
2. Verify development tests locally:
   - `cmake -S . -B build/dev -DCMAKE_BUILD_TYPE=Debug -DLICENSECORE_BUILD_EXAMPLES=ON -DLICENSECORE_BUILD_TESTS=ON -DLICENSECORE_BUILD_GTESTS=OFF`
   - `cmake --build build/dev -j`
   - `ctest --test-dir build/dev --output-on-failure`
3. Check obfuscated smoke build:
   - `cd obfuscated`
   - `gcc -std=c99 -O2 -DNDEBUG -DLICENSECORE_OBFUSCATED=1 -DANTI_DEBUG_BUILD -Iinclude -Isrc -c src/license_core_pure_c.c -o src/license_core_pure_c.o`
   - `ar rcs liblicense_core_ci.a src/license_core_pure_c.o`
4. Update `CHANGELOG.md` for the release version.
5. Confirm `README.md` and `release_obfuscated/README.md` are up to date.

## Create Release
1. Create and push tag:
   - `git tag vX.Y.Z`
   - `git push origin vX.Y.Z`
2. Create GitHub Release manually and upload artifacts:
   - `licensecore-obfuscated-vX.Y.Z.tar.gz`
   - `checksums.txt`
3. Download artifacts and verify checksum:
   - `sha256sum -c checksums.txt`

## After Release
1. Smoke-test integration in a clean sample app.
2. Announce release and include changelog highlights.
