#include "YuEngine/Package/PackageSourceKey.h"

namespace yuengine::package {
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

    if (character == '.') {
        return true;
    }

    return character == '/';
}
}

PackageSourceKey::PackageSourceKey()
    : bytes_{},
      length_(0U),
      is_valid_(false),
      is_within_bounds_(true) {
}

PackageSourceKey::PackageSourceKey(std::string_view value)
    : bytes_{},
      length_(0U),
      is_valid_(false),
      is_within_bounds_(value.size() <= MAX_PACKAGE_SOURCE_KEY_BYTES) {
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

std::string_view PackageSourceKey::Value() const {
    return std::string_view(bytes_.data(), length_);
}

std::size_t PackageSourceKey::ByteLength() const {
    return length_;
}

bool PackageSourceKey::IsValid() const {
    return is_valid_;
}

bool PackageSourceKey::IsWithinBounds() const {
    return is_within_bounds_;
}

bool PackageSourceKey::Equals(const PackageSourceKey& other) const {
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
