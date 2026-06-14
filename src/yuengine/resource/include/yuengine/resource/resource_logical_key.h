#pragma once

#include <array>
#include <cstddef>
#include <string_view>

#include "yuengine/resource/resource_constants.h"

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
    std::array<char, MAX_LOGICAL_KEY_BYTES> _bytes;
    std::size_t _length;
    bool _isValid;
    bool _isWithinBounds;
};
}
