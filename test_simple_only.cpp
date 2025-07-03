#include <iostream>
#include <string>
#include <cassert>

// Simple test without any LicenseCore dependencies
int main() {
    std::cout << "=== Simple Test (No Dependencies) ===" << std::endl;
    
    // Test basic functionality
    std::string test = "Hello, LicenseCore!";
    assert(!test.empty());
    
    std::cout << "✅ Basic string operations working" << std::endl;
    std::cout << "✅ Assertions working" << std::endl;
    std::cout << "✅ Console output working" << std::endl;
    
    std::cout << "🎉 Simple test completed successfully!" << std::endl;
    return 0;
}
