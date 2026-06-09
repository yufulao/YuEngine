#pragma once

#include <cstddef>

namespace yuengine::rhi
{
struct RhiCommandListSnapshot final
{
    std::size_t Capacity = 0U;
    std::size_t CommandCount = 0U;
    bool IsRecording = false;
    bool IsComplete = false;
};
}
