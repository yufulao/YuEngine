#pragma once

#include <string_view>

namespace yuengine::memory
{
struct MemoryOwnerId
{
    std::string_view Value;

    bool IsValid() const
    {
        return !Value.empty();
    }
};
}
