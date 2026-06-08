#include "yuengine/core/Json.h"

#include <fstream>
#include <sstream>

namespace yu::core {
namespace {

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text) {}

    JsonValue parse()
    {
        skipWhitespace();
        JsonValue value = parseValue();
        skipWhitespace();
        if (!eof()) {
            fail("unexpected trailing input");
        }
        return value;
    }

private:
    const std::string& text_;
    size_t pos_ = 0;

    bool eof() const { return pos_ >= text_.size(); }

    char peek() const
    {
        if (eof()) {
            return '\0';
        }
        return text_[pos_];
    }

    char take()
    {
        if (eof()) {
            fail("unexpected end of input");
        }
        return text_[pos_++];
    }

    void fail(const std::string& message) const
    {
        throw JsonError(message + " at byte " + std::to_string(pos_));
    }

    void skipWhitespace()
    {
        while (!eof()) {
            char c = peek();
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                return;
            }
            ++pos_;
        }
    }

    bool consume(const std::string& literal)
    {
        if (text_.compare(pos_, literal.size(), literal) != 0) {
            return false;
        }
        pos_ += literal.size();
        return true;
    }

    JsonValue parseValue()
    {
        skipWhitespace();
        char c = peek();
        if (c == '"') {
            return JsonValue(parseString());
        }
        if (c == '{') {
            return JsonValue(parseObject());
        }
        if (c == '[') {
            return JsonValue(parseArray());
        }
        if (c == '-' || (c >= '0' && c <= '9')) {
            return JsonValue(parseNumber());
        }
        if (consume("true")) {
            return JsonValue(true);
        }
        if (consume("false")) {
            return JsonValue(false);
        }
        if (consume("null")) {
            return JsonValue(nullptr);
        }
        fail("expected JSON value");
        return JsonValue();
    }

    std::string parseString()
    {
        if (take() != '"') {
            fail("expected string");
        }
        std::string out;
        while (!eof()) {
            char c = take();
            if (c == '"') {
                return out;
            }
            if (c == '\\') {
                char esc = take();
                switch (esc) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                default: fail("unsupported string escape");
                }
                continue;
            }
            out.push_back(c);
        }
        fail("unterminated string");
        return out;
    }

    double parseNumber()
    {
        size_t start = pos_;
        if (peek() == '-') {
            ++pos_;
        }
        while (!eof() && peek() >= '0' && peek() <= '9') {
            ++pos_;
        }
        if (!eof() && peek() == '.') {
            ++pos_;
            while (!eof() && peek() >= '0' && peek() <= '9') {
                ++pos_;
            }
        }
        if (!eof() && (peek() == 'e' || peek() == 'E')) {
            ++pos_;
            if (!eof() && (peek() == '+' || peek() == '-')) {
                ++pos_;
            }
            while (!eof() && peek() >= '0' && peek() <= '9') {
                ++pos_;
            }
        }
        return std::stod(text_.substr(start, pos_ - start));
    }

    JsonValue::Array parseArray()
    {
        JsonValue::Array array;
        take();
        skipWhitespace();
        if (peek() == ']') {
            take();
            return array;
        }
        while (true) {
            array.push_back(parseValue());
            skipWhitespace();
            char c = take();
            if (c == ']') {
                return array;
            }
            if (c != ',') {
                fail("expected comma or array end");
            }
        }
    }

    JsonValue::Object parseObject()
    {
        JsonValue::Object object;
        take();
        skipWhitespace();
        if (peek() == '}') {
            take();
            return object;
        }
        while (true) {
            skipWhitespace();
            std::string key = parseString();
            skipWhitespace();
            if (take() != ':') {
                fail("expected object colon");
            }
            object.emplace(std::move(key), parseValue());
            skipWhitespace();
            char c = take();
            if (c == '}') {
                return object;
            }
            if (c != ',') {
                fail("expected comma or object end");
            }
        }
    }
};

const JsonValue nullValue;

} // namespace

JsonError::JsonError(const std::string& message) : std::runtime_error(message) {}

JsonValue::JsonValue() : value_(nullptr) {}
JsonValue::JsonValue(std::nullptr_t) : value_(nullptr) {}
JsonValue::JsonValue(bool value) : value_(value) {}
JsonValue::JsonValue(double value) : value_(value) {}
JsonValue::JsonValue(std::string value) : value_(std::move(value)) {}
JsonValue::JsonValue(Array value) : value_(std::move(value)) {}
JsonValue::JsonValue(Object value) : value_(std::move(value)) {}

bool JsonValue::isNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
bool JsonValue::isBool() const { return std::holds_alternative<bool>(value_); }
bool JsonValue::isNumber() const { return std::holds_alternative<double>(value_); }
bool JsonValue::isString() const { return std::holds_alternative<std::string>(value_); }
bool JsonValue::isArray() const { return std::holds_alternative<Array>(value_); }
bool JsonValue::isObject() const { return std::holds_alternative<Object>(value_); }

bool JsonValue::asBool() const { return std::get<bool>(value_); }
double JsonValue::asNumber() const { return std::get<double>(value_); }
const std::string& JsonValue::asString() const { return std::get<std::string>(value_); }
const JsonValue::Array& JsonValue::asArray() const { return std::get<Array>(value_); }
const JsonValue::Object& JsonValue::asObject() const { return std::get<Object>(value_); }

bool JsonValue::has(const std::string& key) const
{
    return isObject() && asObject().contains(key);
}

const JsonValue& JsonValue::at(const std::string& key) const
{
    return asObject().at(key);
}

const JsonValue& JsonValue::at(size_t index) const
{
    return asArray().at(index);
}

const JsonValue& JsonValue::getOrNull(const std::string& key) const
{
    if (!isObject()) {
        return nullValue;
    }
    const auto& object = asObject();
    auto it = object.find(key);
    if (it == object.end()) {
        return nullValue;
    }
    return it->second;
}

JsonValue parseJson(const std::string& text)
{
    return Parser(text).parse();
}

std::string readTextFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("failed to read file: " + path);
    }
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

std::string jsonEscape(const std::string& value)
{
    std::string out;
    for (char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default: out.push_back(c); break;
        }
    }
    return out;
}

} // namespace yu::core
