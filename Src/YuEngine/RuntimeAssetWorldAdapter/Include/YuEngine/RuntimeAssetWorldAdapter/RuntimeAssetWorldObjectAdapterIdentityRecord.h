// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterIdentityRecord final {
    std::uint64_t target_id = 0U;
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
};
}
