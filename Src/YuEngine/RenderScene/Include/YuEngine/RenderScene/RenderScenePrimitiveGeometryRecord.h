// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"

namespace yuengine::renderscene {
struct RenderScenePrimitiveGeometryRecord final {
    yuengine::asset::AssetHandle geometry_asset{};
    RenderScenePrimitiveGeometryKind kind = RenderScenePrimitiveGeometryKind::Cube;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    std::uint32_t first_vertex = 0U;
    std::uint32_t first_index = 0U;
    std::uint32_t segment_count = 0U;
    std::size_t vertex_stride_bytes = 0U;
    std::size_t index_stride_bytes = 0U;
    yuengine::rendercore::RenderDrawPacketRequest draw{};
    bool is_resolved = false;
};
}
