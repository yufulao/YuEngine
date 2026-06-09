#pragma once

#include <cstdint>

namespace yuengine::rhi
{
struct RhiColor final
{
    std::uint8_t R = 0U;
    std::uint8_t G = 0U;
    std::uint8_t B = 0U;
    std::uint8_t A = 0U;
};
}
