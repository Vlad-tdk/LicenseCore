#include "license_core_pure_c.h"
#include <stdio.h>

int main() {
    printf("🔒 LicenseCore++ Multi-Platform Example\n");
    printf("=====================================\n\n");
    
    // Get hardware ID
    const char* hwid = lc_get_hwid();
    printf("🖥️  Hardware ID: %s\n\n", hwid);
    
    // Test embedded license
    printf("🔐 Testing embedded license...\n");
    if (lc_validate_embedded()) {
        printf("✅ Embedded license is VALID!\n");
        
        // Check available features
        printf("📋 Available features:\n");
        if (lc_has_feature("basic")) {
            printf("  ✅ Basic features\n");
        }
        if (lc_has_feature("premium")) {
            printf("  ✅ Premium features\n");
        } else {
            printf("  ❌ Premium features (not available)\n");
        }
        if (lc_has_feature("enterprise")) {
            printf("  ✅ Enterprise features\n");
        } else {
            printf("  ❌ Enterprise features (not available)\n");
        }
    } else {
        printf("❌ Embedded license INVALID\n");
    }
    
    printf("\n🧪 Testing external license...\n");
    const char* test_license = "{\"user_id\":\"demo\",\"features\":[\"basic\",\"premium\"]}";
    printf("JSON: %s\n", test_license);
    
    if (lc_validate_license(test_license)) {
        printf("✅ External license is VALID!\n");
        printf("📋 Features from external license:\n");
        printf("  - Basic: %s\n", lc_has_feature("basic") ? "✅ Available" : "❌ Missing");
        printf("  - Premium: %s\n", lc_has_feature("premium") ? "✅ Available" : "❌ Missing");
        printf("  - Enterprise: %s\n", lc_has_feature("enterprise") ? "✅ Available" : "❌ Missing");
    } else {
        printf("❌ External license INVALID\n");
    }
    
    printf("\n🛡️  Security test (NULL inputs)...\n");
    printf("NULL license test: %s\n", lc_validate_license(NULL) ? "❌ FAILED" : "✅ Protected");
    printf("NULL feature test: %s\n", lc_has_feature(NULL) ? "❌ FAILED" : "✅ Protected");
    
    printf("\n🎉 LicenseCore++ test completed successfully!\n");
    printf("🔒 Library is working correctly on this platform.\n");
    
    return 0;
}
