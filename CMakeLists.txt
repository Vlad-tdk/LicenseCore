cmake_minimum_required(VERSION 3.16)

project(LicenseCore 
    VERSION 1.0.0
    DESCRIPTION "Modern C++ License Management Library"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(LICENSECORE_BUILD_EXAMPLES "Build examples" ON)
option(LICENSECORE_BUILD_TESTS "Build tests" ON)
option(LICENSECORE_BUILD_GTESTS "Build Google Tests" ON)
option(LICENSECORE_BUILD_SHARED "Build shared library" OFF)

# Find dependencies
find_package(OpenSSL)
if(NOT OpenSSL_FOUND)
    # Try to find OpenSSL manually on different platforms
    if(APPLE)
        # Check Homebrew locations
        if(EXISTS "/opt/homebrew/include/openssl")
            set(OPENSSL_ROOT_DIR "/opt/homebrew")
            set(OPENSSL_INCLUDE_DIR "/opt/homebrew/include")
            set(OPENSSL_LIBRARIES "/opt/homebrew/lib/libssl.dylib;/opt/homebrew/lib/libcrypto.dylib")
            set(OpenSSL_FOUND TRUE)
        elseif(EXISTS "/usr/local/include/openssl")
            set(OPENSSL_ROOT_DIR "/usr/local")
            set(OPENSSL_INCLUDE_DIR "/usr/local/include")
            set(OPENSSL_LIBRARIES "/usr/local/lib/libssl.dylib;/usr/local/lib/libcrypto.dylib")
            set(OpenSSL_FOUND TRUE)
        endif()
    endif()
endif()

# Create main library
set(LICENSECORE_SOURCES
    src/license_manager.cpp
    src/hardware_fingerprint.cpp
    src/hmac_validator.cpp
    src/json/simple_json.cpp
)

set(LICENSECORE_HEADERS
    include/license_core/license_manager.hpp
    include/license_core/hardware_fingerprint.hpp
    include/license_core/hmac_validator.hpp
)

if(LICENSECORE_BUILD_SHARED)
    add_library(licensecore SHARED ${LICENSECORE_SOURCES})
else()
    add_library(licensecore STATIC ${LICENSECORE_SOURCES})
endif()

# Set library properties
target_include_directories(licensecore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(licensecore
    PUBLIC
        ${OPENSSL_LIBRARIES}
)

if(OpenSSL_FOUND)
    target_include_directories(licensecore PRIVATE ${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found. Install with: brew install openssl (macOS) or sudo apt install libssl-dev (Linux)")
endif()

# Platform-specific libraries
if(WIN32)
    target_link_libraries(licensecore PRIVATE iphlpapi wbemuuid ole32 oleaut32)
elseif(APPLE)
    target_link_libraries(licensecore PRIVATE "-framework IOKit" "-framework CoreFoundation")
elseif(UNIX)
    # Linux - no additional libraries needed for basic implementation
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(licensecore PRIVATE /W4)
else()
    target_compile_options(licensecore PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Examples
if(LICENSECORE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Tests
if(LICENSECORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Google Tests
if(LICENSECORE_BUILD_GTESTS)
    # Download and configure Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    add_subdirectory(gtests)
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS licensecore
    EXPORT LicenseCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export configuration
install(EXPORT LicenseCoreTargets
    FILE LicenseCoreTargets.cmake
    NAMESPACE LicenseCore::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LicenseCore
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    LicenseCoreConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/LicenseCoreConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LicenseCoreConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LicenseCore
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/LicenseCoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/LicenseCoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LicenseCore
)
