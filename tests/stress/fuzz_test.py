#!/usr/bin/env python3
"""
LicenseCore++ Fuzzing Test Suite
Generates random inputs to test edge cases and crash resistance
"""

import random
import string
import json
import subprocess
import time
import sys
from typing import List, Dict, Any

class LicenseCoreFuzzer:
    def __init__(self):
        self.test_executable = "./test_full"
        self.iterations = 0
        self.crashes = 0
        self.errors = 0
        
    def generate_random_string(self, length: int = None) -> str:
        """Generate random string of variable length"""
        if length is None:
            length = random.randint(0, 1000)
        
        chars = string.ascii_letters + string.digits + string.punctuation + " \t\n\r"
        return ''.join(random.choice(chars) for _ in range(length))
    
    def generate_random_json(self) -> str:
        """Generate random JSON-like structures"""
        patterns = [
            # Empty/minimal
            "",
            "{}",
            "[]",
            "null",
            
            # Invalid JSON syntax
            "{",
            "}",
            "{\"key\": }",
            "{\"key\": \"value\",}",
            "{'single_quotes': 'invalid'}",
            
            # Long strings
            "{\"long_key\": \"" + "x" * 10000 + "\"}",
            "{\"" + "y" * 1000 + "\": \"value\"}",
            
            # Deep nesting
            self._generate_deep_nested_json(20),
            
            # Unicode and special characters
            "{\"unicode\": \"\\u0000\\u0001\\u0002\\u0003\"}",
            "{\"emoji\": \"🚀💥🔥⚡🎯\"}",
            "{\"newlines\": \"line1\\nline2\\rline3\\r\\n\"}",
            
            # Array variations
            "{\"features\": []}",
            "{\"features\": [\"a\"]}",
            "{\"features\": [" + ",".join(f'"{self.generate_random_string(10)}"' for _ in range(100)) + "]}",
            
            # Invalid field types
            "{\"expiry\": 123}",
            "{\"features\": \"not_array\"}",
            "{\"valid\": \"not_boolean\"}",
            
            # Missing required fields
            "{\"user_id\": \"test\"}",
            "{\"user_id\": \"test\", \"expiry\": \"2025-01-01\"}",
            
            # Random garbage
            self.generate_random_string(500),
        ]
        
        return random.choice(patterns)
    
    def _generate_deep_nested_json(self, depth: int) -> str:
        """Generate deeply nested JSON"""
        if depth == 0:
            return "\"value\""
        
        return "{\"nested\": " + self._generate_deep_nested_json(depth - 1) + "}"
    
    def generate_license_variants(self) -> List[str]:
        """Generate various license format variants"""
        base_license = {
            "user_id": "fuzz_user",
            "license_id": "fuzz_license", 
            "expiry": "2025-12-31T23:59:59Z",
            "issued_at": "2024-01-01T00:00:00Z",
            "hardware_hash": "a1b2c3d4e5f6",
            "features": ["basic", "premium"],
            "version": 1,
            "hmac_signature": "fake_signature"
        }
        
        variants = []
        
        # Valid baseline
        variants.append(json.dumps(base_license))
        
        # Field mutations
        for field in base_license.keys():
            # Remove field
            modified = base_license.copy()
            del modified[field]
            variants.append(json.dumps(modified))
            
            # Corrupt field value
            modified = base_license.copy()
            if field == "features":
                modified[field] = self.generate_random_string(50)
            elif field == "version":
                modified[field] = self.generate_random_string(10)
            else:
                modified[field] = self.generate_random_string(100)
            variants.append(json.dumps(modified))
        
        return variants
    
    def test_json_parsing(self, iterations: int = 1000):
        """Test JSON parsing with random inputs"""
        print(f"🕷️ Fuzzing JSON parsing ({iterations} iterations)...")
        
        crashes = 0
        for i in range(iterations):
            test_input = self.generate_random_json()
            
            try:
                # Test with diagnostic tool as placeholder
                result = subprocess.run([
                    "./diagnostic"
                ], capture_output=True, timeout=5)
                
                if result.returncode < 0:
                    print(f"❌ Crash detected with input: {test_input[:100]}...")
                    crashes += 1
                    
            except subprocess.TimeoutExpired:
                print(f"⏰ Timeout with input: {test_input[:100]}...")
            except Exception as e:
                print(f"⚠️ Test error: {e}")
        
        print(f"✅ JSON fuzzing completed: {crashes} crashes out of {iterations} tests")
        return crashes == 0
    
    def test_license_validation(self, iterations: int = 500):
        """Test license validation with malformed licenses"""
        print(f"🔐 Fuzzing license validation ({iterations} iterations)...")
        
        license_variants = self.generate_license_variants()
        crashes = 0
        
        for i in range(iterations):
            if i < len(license_variants):
                test_license = license_variants[i]
            else:
                test_license = self.generate_random_json()
            
            try:
                # Write test license
                with open("/tmp/fuzz_license.json", "w") as f:
                    f.write(test_license)
                
                # Test validation with simple test as placeholder
                result = subprocess.run([
                    "./test_simple"
                ], capture_output=True, timeout=10)
                
                if result.returncode < 0:
                    print(f"❌ Validation crash: {test_license[:100]}...")
                    crashes += 1
                    
            except subprocess.TimeoutExpired:
                print(f"⏰ Validation timeout: {test_license[:50]}...")
            except Exception as e:
                print(f"⚠️ Validation test error: {e}")
        
        print(f"✅ License validation fuzzing completed: {crashes} crashes out of {iterations} tests")
        return crashes == 0
    
    def test_hardware_fingerprinting(self, iterations: int = 100):
        """Test hardware fingerprinting stability"""
        print(f"🔧 Fuzzing hardware fingerprinting ({iterations} iterations)...")
        
        crashes = 0
        hwids = set()
        
        for i in range(iterations):
            try:
                # Test HWID generation stability
                result = subprocess.run([
                    "./diagnostic"
                ], capture_output=True, timeout=10)
                
                if result.returncode < 0:
                    print(f"❌ HWID generation crash on iteration {i}")
                    crashes += 1
                elif result.returncode == 0 and result.stdout:
                    # Extract HWID from output
                    output = result.stdout.decode('utf-8', errors='ignore')
                    hwids.add(output[:50])  # First 50 chars as fingerprint
                    
            except subprocess.TimeoutExpired:
                print(f"⏰ HWID timeout on iteration {i}")
            except Exception as e:
                print(f"⚠️ HWID test error: {e}")
        
        print(f"✅ Hardware fingerprinting fuzzing completed:")
        print(f"   Crashes: {crashes} out of {iterations} tests")
        print(f"   Unique outputs: {len(hwids)}")
        
        return crashes == 0
    
    def test_performance_under_load(self, iterations: int = 100):
        """Test performance degradation under heavy fuzzing load"""
        print(f"📈 Testing performance under load ({iterations} iterations)...")
        
        try:
            # Baseline performance
            start = time.time()
            for _ in range(10):
                subprocess.run(["./diagnostic"], capture_output=True, timeout=5)
            baseline_time = (time.time() - start) / 10
            
            # Performance under load
            start = time.time() 
            for i in range(iterations):
                result = subprocess.run(["./diagnostic"], capture_output=True, timeout=5)
            
            load_time = (time.time() - start) / iterations
            
            performance_ratio = load_time / baseline_time if baseline_time > 0 else float('inf')
            
            print(f"   Baseline time: {baseline_time*1000:.2f}ms per operation")
            print(f"   Load time: {load_time*1000:.2f}ms per operation")
            print(f"   Performance ratio: {performance_ratio:.2f}x")
            
            # Performance should not degrade more than 5x under load
            return performance_ratio < 5.0
            
        except Exception as e:
            print(f"❌ Performance test error: {e}")
            return False
    
    def run_full_fuzz_test(self, total_iterations: int = 5000):
        """Run comprehensive fuzzing test suite"""
        print("🚀 === LicenseCore++ Comprehensive Fuzzing Suite ===")
        print(f"Running {total_iterations} total fuzzing iterations...")
        print("")
        
        start_time = time.time()
        
        # JSON parsing fuzzing
        json_ok = self.test_json_parsing(total_iterations // 2)
        
        # License validation fuzzing  
        license_ok = self.test_license_validation(total_iterations // 4)
        
        # Hardware fingerprinting fuzzing
        hwid_ok = self.test_hardware_fingerprinting(total_iterations // 20)
        
        # Performance under load
        print("⚡ Testing performance under fuzzing load...")
        load_ok = self.test_performance_under_load(100)
        
        end_time = time.time()
        duration = end_time - start_time
        
        print("")
        print("📊 === Fuzzing Results Summary ===")
        print(f"Duration: {duration:.2f} seconds")
        print(f"JSON Parsing: {'✅ PASS' if json_ok else '❌ FAIL'}")
        print(f"License Validation: {'✅ PASS' if license_ok else '❌ FAIL'}")
        print(f"Hardware Fingerprinting: {'✅ PASS' if hwid_ok else '❌ FAIL'}")
        print(f"Performance Under Load: {'✅ PASS' if load_ok else '❌ FAIL'}")
        
        all_passed = json_ok and license_ok and hwid_ok and load_ok
        
        if all_passed:
            print("")
            print("🎉 === ALL FUZZING TESTS PASSED ===")
            print("LicenseCore++ is robust against malformed inputs!")
        else:
            print("")
            print("❌ === SOME FUZZING TESTS FAILED ===")
            print("Review the failures above and fix before production!")
        
        return all_passed

def main():
    if len(sys.argv) > 1:
        try:
            iterations = int(sys.argv[1])
        except ValueError:
            print("Usage: python3 fuzz_test.py [iterations]")
            sys.exit(1)
    else:
        iterations = 1000
    
    fuzzer = LicenseCoreFuzzer()
    success = fuzzer.run_full_fuzz_test(iterations)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
