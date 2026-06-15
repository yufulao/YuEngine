// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceLogicalKey.h

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
class ResourceLogicalKey final {
public:
    /**
     * @comment Constructs a ResourceLogicalKey instance.
     */
    ResourceLogicalKey();
    /**
     * @comment Constructs a ResourceLogicalKey instance.
     * @param value Input value.
     */
    explicit ResourceLogicalKey(std::string_view value);

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
    bool Equals(const ResourceLogicalKey& other) const;

private:
    std::array<char, MAX_LOGICAL_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
