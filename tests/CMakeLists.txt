cmake_minimum_required(VERSION 3.16)

# Find testing framework (optional)
find_package(GTest QUIET)

if(GTest_FOUND)
    # Google Test
    add_executable(test_licensecore 
        test_main.cpp
        test_hmac_validator.cpp
        test_hardware_fingerprint.cpp
        test_license_manager.cpp
    )
    
    target_link_libraries(test_licensecore 
        licensecore 
        GTest::gtest 
        GTest::gtest_main
    )
    
    add_test(NAME LicenseCoreTests COMMAND test_licensecore)
else()
    # Simple custom tests
    add_executable(test_simple test_simple.cpp)
    target_link_libraries(test_simple licensecore)
    
    add_test(NAME SimpleLicenseCoreTest COMMAND test_simple)
endif()
