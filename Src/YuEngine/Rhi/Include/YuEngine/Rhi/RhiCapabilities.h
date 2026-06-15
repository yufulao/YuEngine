// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCapabilities.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiCapabilities final {
    RhiBackendKind backend_kind = RhiBackendKind::Null;
    RhiFormat color_format = RhiFormat::Rgba8Unorm;
    std::size_t color_target_capacity = 0U;
    std::size_t command_list_capacity = 0U;
    std::uint16_t max_color_target_extent = 0U;
    std::uint16_t max_capture_fixture_extent = 0U;
    bool supports_capture = false;
};
}
