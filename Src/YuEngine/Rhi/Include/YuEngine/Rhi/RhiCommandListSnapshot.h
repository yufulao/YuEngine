// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandListSnapshot.h

#pragma once

#include <cstddef>

namespace yuengine::rhi {
struct RhiCommandListSnapshot final {
    std::size_t capacity = 0U;
    std::size_t command_count = 0U;
    bool is_recording = false;
    bool is_complete = false;
};
}
