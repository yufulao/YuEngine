// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h

#pragma once

#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType type = RhiCommandType::BeginFrame;
    RhiTextureHandle target{};
    RhiColor color{};
    RhiPipelineHandle pipeline{};
    RhiVertexBufferView vertex_buffer{};
    RhiDrawDesc draw{};
};
}
