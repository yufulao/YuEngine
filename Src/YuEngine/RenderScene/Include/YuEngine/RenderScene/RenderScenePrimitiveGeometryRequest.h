// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::renderscene {
struct RenderScenePrimitiveGeometryRequest final {
    yuengine::asset::AssetHandle geometry_asset{};
    RenderScenePrimitiveGeometryKind kind = RenderScenePrimitiveGeometryKind::Cube;
    std::uint32_t segment_count = 16U;
    std::uint32_t draw_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiVertexBufferView vertex_buffer{};
    yuengine::rhi::RhiIndexBufferView index_buffer{};
};
}
