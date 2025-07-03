cmake_minimum_required(VERSION 3.16)

# Simple example
add_executable(simple_example simple_example.cpp)
target_link_libraries(simple_example licensecore)

# License generator
add_executable(license_generator license_generator.cpp)
target_link_libraries(license_generator licensecore)

# Hardware fingerprint tool
add_executable(hwid_tool hwid_tool.cpp)
target_link_libraries(hwid_tool licensecore)

# Error handling example
add_executable(error_handling_example error_handling_example.cpp)
target_link_libraries(error_handling_example licensecore)

# Caching example
add_executable(caching_example caching_example.cpp)
target_link_libraries(caching_example licensecore)
