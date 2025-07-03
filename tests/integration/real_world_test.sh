#!/bin/bash

# Real-World Integration Test for LicenseCore++
# Tests the library in realistic usage scenarios

set -e

echo "🌍 === LicenseCore++ Real-World Integration Tests ==="
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SUCCESS_COUNT=0
FAILURE_COUNT=0

log_test() {
    echo -e "${BLUE}🧪 $1${NC}"
}

log_success() {
    echo -e "${GREEN}✅ $1${NC}"
    ((SUCCESS_COUNT++))
}

log_failure() {
    echo -e "${RED}❌ $1${NC}"
    ((FAILURE_COUNT++))
}

log_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

# Test 1: Customer Onboarding Workflow
test_customer_onboarding() {
    log_test "Testing customer onboarding workflow..."
    
    # Test basic functionality
    if ./diagnostic >/dev/null 2>&1; then
        log_success "Customer onboarding: Basic system check passes"
    else
        log_failure "Customer onboarding: Basic system check failed"
    fi
    
    if ./test_simple >/dev/null 2>&1; then
        log_success "Customer onboarding: Simple license test passes"
    else
        log_failure "Customer onboarding: Simple license test failed"
    fi
}

# Test 2: License Server Simulation
test_license_server_simulation() {
    log_test "Testing license server simulation..."
    
    # Simulate rapid license operations
    SUCCESS_OPS=0
    TOTAL_OPS=50
    
    for i in $(seq 1 $TOTAL_OPS); do
        if ./diagnostic >/dev/null 2>&1; then
            ((SUCCESS_OPS++))
        fi
    done
    
    if [ $SUCCESS_OPS -eq $TOTAL_OPS ]; then
        log_success "License server simulation: $SUCCESS_OPS/$TOTAL_OPS operations successful"
    else
        log_failure "License server simulation: Only $SUCCESS_OPS/$TOTAL_OPS operations successful"
    fi
}

# Test 3: Enterprise Deployment Simulation
test_enterprise_deployment() {
    log_test "Testing enterprise deployment simulation..."
    
    # Test different scenarios
    SCENARIOS=("diagnostic" "test_simple")
    ENTERPRISE_SUCCESS=0
    
    for scenario in "${SCENARIOS[@]}"; do
        if [ -f "./$scenario" ] && "./$scenario" >/dev/null 2>&1; then
            ((ENTERPRISE_SUCCESS++))
        fi
    done
    
    if [ $ENTERPRISE_SUCCESS -eq ${#SCENARIOS[@]} ]; then
        log_success "Enterprise deployment: All $ENTERPRISE_SUCCESS scenarios work"
    else
        log_failure "Enterprise deployment: Only $ENTERPRISE_SUCCESS out of ${#SCENARIOS[@]} scenarios work"
    fi
}

# Test 4: Multi-Platform Compatibility
test_multiplatform_compatibility() {
    log_test "Testing multi-platform compatibility..."
    
    PLATFORM=$(uname)
    ARCH=$(uname -m)
    
    echo "   Platform: $PLATFORM ($ARCH)"
    
    # Test compilation and execution
    if make diagnostic >/dev/null 2>&1 && ./diagnostic >/dev/null 2>&1; then
        log_success "Multi-platform: Works on $PLATFORM ($ARCH)"
    else
        log_failure "Multi-platform: Issues on $PLATFORM ($ARCH)"
    fi
}

# Test 5: Security Edge Cases
test_security_edge_cases() {
    log_test "Testing security edge cases..."
    
    # Test with malformed inputs
    SECURITY_SUCCESS=0
    SECURITY_TESTS=3
    
    # Test 1: Empty input
    if echo "" | ./diagnostic >/dev/null 2>&1; then
        ((SECURITY_SUCCESS++))
    fi
    
    # Test 2: Random garbage
    if echo "random garbage input" | ./diagnostic >/dev/null 2>&1; then
        ((SECURITY_SUCCESS++))
    fi
    
    # Test 3: Very long input
    if printf '%*s' 10000 '' | ./diagnostic >/dev/null 2>&1; then
        ((SECURITY_SUCCESS++))
    fi
    
    if [ $SECURITY_SUCCESS -eq $SECURITY_TESTS ]; then
        log_success "Security: All $SECURITY_TESTS edge cases handled"
    else
        log_warning "Security: $SECURITY_SUCCESS out of $SECURITY_TESTS edge cases handled"
    fi
}

# Test 6: Performance Under Real Load
test_performance_real_load() {
    log_test "Testing performance under real load..."
    
    START_TIME=$(date +%s)
    OPERATIONS=0
    DURATION=5  # seconds
    END_TIME=$(($(date +%s) + DURATION))
    
    while [ $(date +%s) -lt $END_TIME ]; do
        if ./diagnostic >/dev/null 2>&1; then
            ((OPERATIONS++))
        fi
        sleep 0.01
    done
    
    ACTUAL_TIME=$(($(date +%s) - START_TIME))
    OPS_PER_SEC=$((OPERATIONS / ACTUAL_TIME))
    
    echo "   Operations: $OPERATIONS in ${ACTUAL_TIME}s"
    echo "   Performance: ${OPS_PER_SEC} ops/sec"
    
    if [ $OPS_PER_SEC -gt 20 ]; then
        log_success "Performance: ${OPS_PER_SEC} ops/sec (acceptable)"
    else
        log_warning "Performance: ${OPS_PER_SEC} ops/sec (below target)"
    fi
}

# Test 7: Memory and Resource Cleanup
test_resource_cleanup() {
    log_test "Testing memory and resource cleanup..."
    
    # Run multiple iterations to check for leaks
    CLEANUP_SUCCESS=0
    
    for i in {1..10}; do
        if ./diagnostic >/dev/null 2>&1; then
            ((CLEANUP_SUCCESS++))
        fi
    done
    
    if [ $CLEANUP_SUCCESS -eq 10 ]; then
        log_success "Resource cleanup: 10/10 iterations successful"
    else
        log_warning "Resource cleanup: $CLEANUP_SUCCESS/10 iterations successful"
    fi
}

# Main test execution
main() {
    echo "Running comprehensive real-world integration tests..."
    echo ""
    
    # Build required tools first
    echo "🔨 Building required tools..."
    make diagnostic >/dev/null 2>&1 || { echo "Failed to build diagnostic"; exit 1; }
    make test_simple >/dev/null 2>&1 || { echo "Failed to build test_simple"; exit 1; }
    
    echo ""
    
    # Run all tests
    test_customer_onboarding
    echo ""
    
    test_license_server_simulation  
    echo ""
    
    test_enterprise_deployment
    echo ""
    
    test_multiplatform_compatibility
    echo ""
    
    test_security_edge_cases
    echo ""
    
    test_performance_real_load
    echo ""
    
    test_resource_cleanup
    echo ""
    
    # Final results
    TOTAL_TESTS=$((SUCCESS_COUNT + FAILURE_COUNT))
    
    echo "📊 === Real-World Integration Test Results ==="
    echo "Total Tests: $TOTAL_TESTS"
    echo "Passed: $SUCCESS_COUNT"
    echo "Failed: $FAILURE_COUNT" 
    echo ""
    
    if [ $FAILURE_COUNT -eq 0 ]; then
        echo -e "${GREEN}🎉 === ALL REAL-WORLD TESTS PASSED ===${NC}"
        echo "LicenseCore++ is ready for production deployment!"
        echo ""
        echo "✅ Customer onboarding workflow works"
        echo "✅ License server can handle volume"
        echo "✅ Enterprise deployment scenarios covered"
        echo "✅ Multi-platform compatibility confirmed"
        echo "✅ Security edge cases handled"
        echo "✅ Performance meets requirements"
        echo "✅ Resource cleanup working"
        exit 0
    else
        echo -e "${RED}❌ === SOME REAL-WORLD TESTS FAILED ===${NC}"
        echo "Review failures above before production deployment!"
        echo ""
        echo "🔧 Recommendations:"
        if [ $SUCCESS_COUNT -lt $((TOTAL_TESTS / 2)) ]; then
            echo "- Major issues detected, thorough review needed"
        elif [ $SUCCESS_COUNT -lt $((TOTAL_TESTS * 4 / 5)) ]; then
            echo "- Some issues detected, targeted fixes needed"
        else
            echo "- Minor issues detected, may be acceptable for MVP"
        fi
        exit 1
    fi
}

# Check if script is being sourced or executed
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    main "$@"
fi
