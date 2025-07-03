cmake_minimum_required(VERSION 3.16)

# Google Test CMakeLists.txt for LicenseCore

# Include Google Test
include(GoogleTest)

# Common test utilities
add_library(test_utils STATIC
    test_utils.cpp
)

target_link_libraries(test_utils
    PUBLIC
        gtest
        gmock
        licensecore
)

target_include_directories(test_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# Hardware Fingerprint Tests
add_executable(hardware_fingerprint_tests
    test_hardware_fingerprint.cpp
)

target_link_libraries(hardware_fingerprint_tests
    PRIVATE
        test_utils
        gtest_main
        gmock_main
        licensecore
)

# Caching System Tests
add_executable(caching_tests
    test_caching.cpp
)

target_link_libraries(caching_tests
    PRIVATE
        test_utils
        gtest_main
        gmock_main
        licensecore
)

# Error Handling Tests
add_executable(error_handling_tests
    test_error_handling.cpp
)

target_link_libraries(error_handling_tests
    PRIVATE
        test_utils
        gtest_main
        gmock_main
        licensecore
)

# Performance Benchmark Tests
add_executable(performance_tests
    test_performance.cpp
)

target_link_libraries(performance_tests
    PRIVATE
        test_utils
        gtest_main
        gmock_main
        licensecore
)

# Thread Safety Tests
add_executable(thread_safety_tests
    test_thread_safety.cpp
)

target_link_libraries(thread_safety_tests
    PRIVATE
        test_utils
        gtest_main
        gmock_main
        licensecore
)

# Platform-specific linking
if(UNIX AND NOT APPLE)
    target_link_libraries(thread_safety_tests PRIVATE pthread)
endif()

# Register tests with CTest - this must come AFTER the executables are defined
gtest_discover_tests(hardware_fingerprint_tests
    PROPERTIES
        TIMEOUT 60
        LABELS "unit;core"
)

gtest_discover_tests(caching_tests
    PROPERTIES
        TIMEOUT 60
        LABELS "unit;performance"
)

gtest_discover_tests(error_handling_tests
    PROPERTIES
        TIMEOUT 30
        LABELS "unit;robustness"
)

gtest_discover_tests(performance_tests
    PROPERTIES
        TIMEOUT 120
        LABELS "performance;benchmark"
)

gtest_discover_tests(thread_safety_tests
    PROPERTIES
        TIMEOUT 180
        LABELS "concurrency;stress"
)

# Custom test targets
add_custom_target(run_gtests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS 
        hardware_fingerprint_tests
        caching_tests
        error_handling_tests
        performance_tests
        thread_safety_tests
    COMMENT "Running all Google Tests"
)

add_custom_target(run_gtests_fast
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --exclude-regex "performance|thread"
    DEPENDS 
        hardware_fingerprint_tests
        caching_tests
        error_handling_tests
    COMMENT "Running fast Google Tests (excluding performance and thread safety tests)"
)

add_custom_target(run_gtests_performance
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex "performance"
    DEPENDS 
        performance_tests
    COMMENT "Running performance tests only"
)

add_custom_target(run_gtests_thread_safety
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose --tests-regex "thread"
    DEPENDS 
        thread_safety_tests
    COMMENT "Running thread safety tests only"
)
