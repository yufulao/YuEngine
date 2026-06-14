#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "yuengine/package/PackageConstants.h"

namespace yuengine::package {
class PackageSourceKey final {
public:
    PackageSourceKey();
    explicit PackageSourceKey(std::string_view value);

    std::string_view Value() const;
    std::size_t ByteLength() const;
    bool IsValid() const;
    bool IsWithinBounds() const;
    bool Equals(const PackageSourceKey& other) const;

private:
    std::array<char, MAX_PACKAGE_SOURCE_KEY_BYTES> _bytes;
    std::size_t _length;
    bool _isValid;
    bool _isWithinBounds;
};
}
