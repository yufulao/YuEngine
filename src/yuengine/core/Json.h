#pragma once

#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace yu::core {

class JsonError final : public std::runtime_error {
public:
    explicit JsonError(const std::string& message);
};

class JsonValue {
public:
    using Array = std::vector<JsonValue>;
    using Object = std::map<std::string, JsonValue>;

    JsonValue();
    explicit JsonValue(std::nullptr_t);
    explicit JsonValue(bool value);
    explicit JsonValue(double value);
    explicit JsonValue(std::string value);
    explicit JsonValue(Array value);
    explicit JsonValue(Object value);

    bool isNull() const;
    bool isBool() const;
    bool isNumber() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;

    bool asBool() const;
    double asNumber() const;
    const std::string& asString() const;
    const Array& asArray() const;
    const Object& asObject() const;

    bool has(const std::string& key) const;
    const JsonValue& at(const std::string& key) const;
    const JsonValue& at(size_t index) const;
    const JsonValue& getOrNull(const std::string& key) const;

private:
    std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value_;
};

JsonValue parseJson(const std::string& text);
std::string readTextFile(const std::string& path);
std::string jsonEscape(const std::string& value);

} // namespace yu::core
