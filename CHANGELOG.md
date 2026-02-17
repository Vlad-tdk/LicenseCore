# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

### Added
- New CI pipeline with core CMake checks and obfuscated smoke tests: `.github/workflows/ci.yml`.
- New tag-based release pipeline for obfuscated artifacts: `.github/workflows/release.yml`.
- Release operating checklist: `RELEASE_CHECKLIST.md`.

### Changed
- Build/test helper scripts now use repository-relative paths instead of machine-specific absolute paths:
  - `check_specialized_builds.sh`
  - `test_compilation.sh`
  - `debug_test.sh`
  - `test_pure_c_obfuscated.sh`
  - `update_docs_wasm.sh`
  - `test_simplified_build.sh`
  - `test_obfuscated_quick.sh`
  - `test_obf_build.sh`
  - `test_all_builds.sh`
  - `simple_build_test.sh`

### Removed
- Deprecated GitHub Actions workflows:
  - `.github/workflows/nightly.yml`
  - `.github/workflows/build.yml`
  - `.github/workflows/test.yml`
