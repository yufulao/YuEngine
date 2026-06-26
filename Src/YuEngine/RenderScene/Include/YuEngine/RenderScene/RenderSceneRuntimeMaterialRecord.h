// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialConstants.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"

namespace yuengine::renderscene {
struct RenderSceneRuntimeMaterialRecord final {
    yuengine::asset::AssetHandle material_asset{};
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    std::array<
        RenderSceneRuntimeMaterialTextureSlot,
        MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> texture_slots{};
    std::size_t texture_slot_count = 0U;
    std::array<std::uint8_t, MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES> material_constant_bytes{};
    std::size_t material_constant_byte_count = 0U;
    bool is_resolved = false;
};
}
