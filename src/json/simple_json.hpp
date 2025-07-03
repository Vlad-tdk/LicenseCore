#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <stdexcept>
#include <cstddef>

namespace license_core {
namespace json {

// JSON parsing exceptions
class JsonParsingException : public std::runtime_error {
public:
    explicit JsonParsingException(const std::string& msg) : std::runtime_error(msg) {}
};

class JsonSizeException : public JsonParsingException {
public:
    explicit JsonSizeException(const std::string& msg) : JsonParsingException(msg) {}
};

class JsonDepthException : public JsonParsingException {
public:
    explicit JsonDepthException(const std::string& msg) : JsonParsingException(msg) {}
};

class JsonBoundsException : public JsonParsingException {
public:
    explicit JsonBoundsException(const std::string& msg) : JsonParsingException(msg) {}
};

// Safe JSON parser with validation

// Simple JSON value type
using JsonValue = std::variant<
    std::string,
    int,
    double,
    bool,
    std::vector<std::string>,
    std::unordered_map<std::string, std::string>
>;

// Safe JSON parser with strict validation
class SafeJsonParser {
public:
    // Security limits
    static const size_t MAX_JSON_SIZE = 1024 * 1024;  // 1MB limit
    static const size_t MAX_NESTING_DEPTH = 32;       // Maximum nesting depth
    static const size_t MAX_STRING_LENGTH = 64 * 1024; // 64KB string limit
    static const size_t MAX_ARRAY_SIZE = 1000;        // Maximum array elements
    static const size_t MAX_OBJECT_KEYS = 100;        // Maximum object keys
    
    // Validate bounds and throw if invalid
    static void validate_bounds(size_t pos, size_t length) {
        if (pos >= length) {
            throw JsonBoundsException("JSON parsing: position out of bounds at " + std::to_string(pos));
        }
    }
    
    // Validate JSON size
    static void validate_size(size_t size) {
        if (size > MAX_JSON_SIZE) {
            throw JsonSizeException("JSON size exceeds maximum allowed: " + std::to_string(size) + " > " + std::to_string(MAX_JSON_SIZE));
        }
    }
    
    // Validate nesting depth
    static void validate_depth(size_t depth) {
        if (depth > MAX_NESTING_DEPTH) {
            throw JsonDepthException("JSON nesting depth exceeds maximum: " + std::to_string(depth) + " > " + std::to_string(MAX_NESTING_DEPTH));
        }
    }
    
    // Validate string length
    static void validate_string_length(size_t length) {
        if (length > MAX_STRING_LENGTH) {
            throw JsonParsingException("String length exceeds maximum: " + std::to_string(length) + " > " + std::to_string(MAX_STRING_LENGTH));
        }
    }
    
    // Validate array size
    static void validate_array_size(size_t size) {
        if (size > MAX_ARRAY_SIZE) {
            throw JsonParsingException("Array size exceeds maximum: " + std::to_string(size) + " > " + std::to_string(MAX_ARRAY_SIZE));
        }
    }
    
    // Validate object key count
    static void validate_object_size(size_t size) {
        if (size > MAX_OBJECT_KEYS) {
            throw JsonParsingException("Object key count exceeds maximum: " + std::to_string(size) + " > " + std::to_string(MAX_OBJECT_KEYS));
        }
    }
    
    // Safe character access with bounds checking
    static char safe_char_at(const std::string& str, size_t pos) {
        validate_bounds(pos, str.length());
        return str[pos];
    }
    
    // Skip whitespace safely
    static size_t skip_whitespace(const std::string& str, size_t pos) {
        while (pos < str.length() && std::isspace(str[pos])) {
            pos++;
        }
        return pos;
    }
};

class SimpleJson {
public:
    // Parse JSON string (simplified - handles only our license format)
    static std::unordered_map<std::string, JsonValue> parse(const std::string& json_str);
    
    // Generate JSON string from map
    static std::string stringify(const std::unordered_map<std::string, JsonValue>& data);
    
    // Helper to get string value
    static std::string get_string(const std::unordered_map<std::string, JsonValue>& data, 
                                  const std::string& key, 
                                  const std::string& default_value = "");
    
    // Helper to get string array
    static std::vector<std::string> get_string_array(const std::unordered_map<std::string, JsonValue>& data, 
                                                      const std::string& key);
    
    // Helper to check if key exists
    static bool has_key(const std::unordered_map<std::string, JsonValue>& data, 
                        const std::string& key);

private:
    static std::string trim(const std::string& str);
    static std::string escape_json_string(const std::string& str);
    static std::string unescape_json_string(const std::string& str);
};

} // namespace json
} // namespace license_core
