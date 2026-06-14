#include "YuEngine/Resource/ResourceLogicalKey.h"

namespace yuengine::resource {
namespace {
bool IsLowercaseAsciiFixtureCharacter(char character) {
    if (character >= 'a' && character <= 'z') {
        return true;
    }

    if (character >= '0' && character <= '9') {
        return true;
    }

    if (character == '_') {
        return true;
    }

    if (character == '-') {
        return true;
    }

    return character == '.';
}
}

ResourceLogicalKey::ResourceLogicalKey()
    : _bytes{},
      _length(0U),
      _isValid(false),
      _isWithinBounds(true) {
}

ResourceLogicalKey::ResourceLogicalKey(std::string_view value)
    : _bytes{},
      _length(0U),
      _isValid(false),
      _isWithinBounds(value.size() <= MAX_LOGICAL_KEY_BYTES) {
    if (value.empty()) {
        return;
    }

    if (!_isWithinBounds) {
        return;
    }

    for (const char character : value) {
        if (!IsLowercaseAsciiFixtureCharacter(character)) {
            return;
        }

        _bytes[_length] = character;
        ++_length;
    }

    _isValid = true;
}

std::string_view ResourceLogicalKey::Value() const {
    return std::string_view(_bytes.data(), _length);
}

std::size_t ResourceLogicalKey::ByteLength() const {
    return _length;
}

bool ResourceLogicalKey::IsValid() const {
    return _isValid;
}

bool ResourceLogicalKey::IsWithinBounds() const {
    return _isWithinBounds;
}

bool ResourceLogicalKey::Equals(const ResourceLogicalKey& other) const {
    if (_length != other._length) {
        return false;
    }

    for (std::size_t index = 0U; index < _length; ++index) {
        if (_bytes[index] != other._bytes[index]) {
            return false;
        }
    }

    return true;
}
}
