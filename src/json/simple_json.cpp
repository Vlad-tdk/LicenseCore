#include "simple_json.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <limits>

namespace license_core {
namespace json {

std::unordered_map<std::string, JsonValue> SimpleJson::parse(const std::string& json_str) {
    std::unordered_map<std::string, JsonValue> result;
    
    try {
        // Security: Validate JSON size
        SafeJsonParser::validate_size(json_str.size());
        
        // Remove outer braces and whitespace
        std::string content = trim(json_str);
        if (content.empty() || content.front() != '{' || content.back() != '}') {
            throw JsonParsingException("Invalid JSON format: missing outer braces");
        }
        
        content = content.substr(1, content.length() - 2);
        
        // Parse with security checks
        size_t pos = 0;
        size_t object_key_count = 0;
        
        while (pos < content.length()) {
            // Security: Check object size limit
            SafeJsonParser::validate_object_size(object_key_count);
            
            // Skip whitespace safely
            pos = SafeJsonParser::skip_whitespace(content, pos);
            
            if (pos >= content.length()) break;
            
            // Find key with bounds checking
            if (SafeJsonParser::safe_char_at(content, pos) != '"') {
                // Skip to next comma or end
                while (pos < content.length() && content[pos] != ',') pos++;
                if (pos < content.length()) pos++;
                continue;
            }
            
            pos++; // skip opening quote
            size_t key_start = pos;
            
            // Find closing quote with escape handling
            while (pos < content.length() && content[pos] != '"') {
                if (content[pos] == '\\' && pos + 1 < content.length()) {
                    pos += 2; // skip escaped character
                } else {
                    pos++;
                }
            }
            
            if (pos >= content.length()) {
                throw JsonParsingException("Unterminated string in JSON key");
            }
            
            std::string key = content.substr(key_start, pos - key_start);
            SafeJsonParser::validate_string_length(key.length());
            pos++; // skip closing quote
            
            // Skip whitespace and colon
            pos = SafeJsonParser::skip_whitespace(content, pos);
            if (pos >= content.length() || content[pos] != ':') {
                throw JsonParsingException("Missing colon after JSON key");
            }
            pos++; // skip colon
            pos = SafeJsonParser::skip_whitespace(content, pos);
            
            if (pos >= content.length()) {
                throw JsonParsingException("Missing value after JSON key");
            }
            
            // Parse value with security checks
            if (SafeJsonParser::safe_char_at(content, pos) == '"') {
                // String value
                pos++;
                size_t value_start = pos;
                while (pos < content.length() && content[pos] != '"') {
                    if (content[pos] == '\\' && pos + 1 < content.length()) {
                        pos += 2; // skip escaped character
                    } else {
                        pos++;
                    }
                }
                
                if (pos >= content.length()) {
                    throw JsonParsingException("Unterminated string in JSON value");
                }
                
                std::string str_val = content.substr(value_start, pos - value_start);
                SafeJsonParser::validate_string_length(str_val.length());
                result[key] = unescape_json_string(str_val);
                pos++; // skip closing quote
            }
            else if (SafeJsonParser::safe_char_at(content, pos) == '[') {
                // Array value
                pos++;
                std::vector<std::string> array_values;
                
                while (pos < content.length() && content[pos] != ']') {
                    SafeJsonParser::validate_array_size(array_values.size());
                    
                    pos = SafeJsonParser::skip_whitespace(content, pos);
                    
                    if (pos < content.length() && content[pos] == '"') {
                        pos++;
                        size_t item_start = pos;
                        while (pos < content.length() && content[pos] != '"') {
                            if (content[pos] == '\\' && pos + 1 < content.length()) {
                                pos += 2;
                            } else {
                                pos++;
                            }
                        }
                        
                        if (pos >= content.length()) {
                            throw JsonParsingException("Unterminated string in JSON array");
                        }
                        
                        std::string item = content.substr(item_start, pos - item_start);
                        SafeJsonParser::validate_string_length(item.length());
                        array_values.push_back(unescape_json_string(item));
                        pos++;
                    }
                    
                    pos = SafeJsonParser::skip_whitespace(content, pos);
                    if (pos < content.length() && content[pos] == ',') {
                        pos++;
                    }
                }
                
                result[key] = array_values;
                if (pos < content.length()) pos++; // skip ]
            }
            else {
                // Number, boolean, or other
                size_t value_start = pos;
                while (pos < content.length() && content[pos] != ',' && content[pos] != '}') {
                    pos++;
                }
                std::string value_str = trim(content.substr(value_start, pos - value_start));
                SafeJsonParser::validate_string_length(value_str.length());
                
                if (value_str == "true" || value_str == "false") {
                    result[key] = (value_str == "true");
                } else {
                    result[key] = value_str;
                }
            }
            
            // Skip to next field
            pos = SafeJsonParser::skip_whitespace(content, pos);
            if (pos < content.length() && content[pos] == ',') {
                pos++;
            }
            
            object_key_count++;
        }
        
    } catch (const JsonParsingException&) {
        throw; // Re-throw our JSON exceptions
    } catch (const std::exception& e) {
        throw JsonParsingException("JSON parsing error: " + std::string(e.what()));
    }
    
    return result;
}

std::string SimpleJson::stringify(const std::unordered_map<std::string, JsonValue>& data) {
    std::ostringstream ss;
    ss << "{\n";
    
    bool first = true;
    for (const auto& [key, value] : data) {
        if (!first) ss << ",\n";
        first = false;
        
        ss << "  \"" << escape_json_string(key) << "\": ";
        
        std::visit([&ss](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                ss << "\"" << escape_json_string(v) << "\"";
            }
            else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                ss << "[";
                bool array_first = true;
                for (const auto& item : v) {
                    if (!array_first) ss << ", ";
                    array_first = false;
                    ss << "\"" << escape_json_string(item) << "\"";
                }
                ss << "]";
            }
            else if constexpr (std::is_same_v<T, bool>) {
                ss << (v ? "true" : "false");
            }
            else if constexpr (std::is_same_v<T, int>) {
                ss << v;
            }
            else if constexpr (std::is_same_v<T, double>) {
                ss << v;
            }
            else if constexpr (std::is_same_v<T, std::unordered_map<std::string, std::string>>) {
                // Skip unsupported type for now
                ss << "\"unsupported_map\"";
            }
            else {
                // Fallback - convert to string if possible
                ss << "\"unknown_type\"";
            }
        }, value);
    }
    
    ss << "\n}";
    return ss.str();
}

std::string SimpleJson::get_string(const std::unordered_map<std::string, JsonValue>& data, 
                                   const std::string& key, 
                                   const std::string& default_value) {
    auto it = data.find(key);
    if (it != data.end()) {
        if (auto* str = std::get_if<std::string>(&it->second)) {
            return *str;
        }
    }
    return default_value;
}

std::vector<std::string> SimpleJson::get_string_array(const std::unordered_map<std::string, JsonValue>& data, 
                                                       const std::string& key) {
    auto it = data.find(key);
    if (it != data.end()) {
        if (auto* arr = std::get_if<std::vector<std::string>>(&it->second)) {
            return *arr;
        }
    }
    return {};
}

bool SimpleJson::has_key(const std::unordered_map<std::string, JsonValue>& data, 
                        const std::string& key) {
    return data.find(key) != data.end();
}

std::string SimpleJson::trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string SimpleJson::escape_json_string(const std::string& str) {
    std::string result;
    result.reserve(str.length() + 10);
    
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    
    return result;
}

std::string SimpleJson::unescape_json_string(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\\' && i + 1 < str.length()) {
            switch (str[i + 1]) {
                case '"': result += '"'; ++i; break;
                case '\\': result += '\\'; ++i; break;
                case 'b': result += '\b'; ++i; break;
                case 'f': result += '\f'; ++i; break;
                case 'n': result += '\n'; ++i; break;
                case 'r': result += '\r'; ++i; break;
                case 't': result += '\t'; ++i; break;
                default: result += str[i]; break;
            }
        } else {
            result += str[i];
        }
    }
    
    return result;
}

} // namespace json
} // namespace license_core
