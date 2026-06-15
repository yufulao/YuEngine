// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/PackageSourceKey.h

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Package/PackageConstants.h"

namespace yuengine::package {
class PackageSourceKey final {
public:
    /**
     * @comment Constructs a PackageSourceKey instance.
     */
    PackageSourceKey();
    /**
     * @comment Constructs a PackageSourceKey instance.
     * @param value Input value.
     */
    explicit PackageSourceKey(std::string_view value);

    /**
     * @comment Returns the stored value.
     * @return Value value.
     */
    std::string_view Value() const;
    /**
     * @comment Returns the stored byte length.
     * @return Byte length value.
     */
    std::size_t ByteLength() const;
    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const;
    /**
     * @comment Checks whether the value is within bounds.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsWithinBounds() const;
    /**
     * @comment Checks whether the values are equal.
     * @param other Input other.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Equals(const PackageSourceKey& other) const;

private:
    std::array<char, MAX_PACKAGE_SOURCE_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
