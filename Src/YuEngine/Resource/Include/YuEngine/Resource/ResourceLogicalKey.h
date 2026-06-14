#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "YuEngine/Resource/ResourceConstants.h"

namespace yuengine::resource {
class ResourceLogicalKey final {
public:
    ResourceLogicalKey();
    explicit ResourceLogicalKey(std::string_view value);

    std::string_view Value() const;
    std::size_t ByteLength() const;
    bool IsValid() const;
    bool IsWithinBounds() const;
    bool Equals(const ResourceLogicalKey& other) const;

private:
    std::array<char, MAX_LOGICAL_KEY_BYTES> bytes_;
    std::size_t length_;
    bool is_valid_;
    bool is_within_bounds_;
};
}
