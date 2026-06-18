// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Src/ResourceLogicalKey.cpp

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
    : bytes_{},
      length_(0U),
      is_valid_(false),
      is_within_bounds_(true) {
}

ResourceLogicalKey::ResourceLogicalKey(std::string_view value)
    : bytes_{},
      length_(0U),
      is_valid_(false),
      is_within_bounds_(value.size() <= MAX_LOGICAL_KEY_BYTES) {
    if (value.empty()) {
        return;
    }

    if (!is_within_bounds_) {
        return;
    }

    for (const char character : value) {
        if (!IsLowercaseAsciiFixtureCharacter(character)) {
            return;
        }

        bytes_[length_] = character;
        ++length_;
    }

    is_valid_ = true;
}

std::string_view ResourceLogicalKey::Value() const {
    return std::string_view(bytes_.data(), length_);
}

std::size_t ResourceLogicalKey::ByteLength() const {
    return length_;
}

bool ResourceLogicalKey::IsValid() const {
    return is_valid_;
}

bool ResourceLogicalKey::IsWithinBounds() const {
    return is_within_bounds_;
}

bool ResourceLogicalKey::Equals(const ResourceLogicalKey& other) const {
    if (length_ != other.length_) {
        return false;
    }

    for (std::size_t index = 0U; index < length_; ++index) {
        if (bytes_[index] != other.bytes_[index]) {
            return false;
        }
    }

    return true;
}
}
