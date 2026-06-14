#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/rhi/rhi_backend_kind.h"
#include "yuengine/rhi/rhi_format.h"

namespace yuengine::rhi {
struct RhiCapabilities final {
    RhiBackendKind BackendKind = RhiBackendKind::Null;
    RhiFormat ColorFormat = RhiFormat::Rgba8Unorm;
    std::size_t ColorTargetCapacity = 0U;
    std::size_t CommandListCapacity = 0U;
    std::uint16_t MaxColorTargetExtent = 0U;
    std::uint16_t MaxCaptureFixtureExtent = 0U;
    bool SupportsCapture = false;
};
}
