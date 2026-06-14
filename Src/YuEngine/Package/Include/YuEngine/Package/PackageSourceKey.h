#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Package/PackageConstants.h"

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
    std::array<char, MAX_PACKAGE_SOURCE_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
