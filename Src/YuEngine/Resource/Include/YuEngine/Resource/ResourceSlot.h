// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceSlot.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceSlot final {
    ResourceTypeId type;
    ResourceLogicalKey logical_key;
    ResourceLoadState load_state = ResourceLoadState::Unloaded;
    ResourceResidencyState residency_state = ResourceResidencyState::Unloaded;
    std::uint32_t generation = INVALID_RESOURCE_GENERATION;
    std::uint32_t reference_count = 0U;
    std::uint64_t last_load_commit_id = 0U;
    std::uint64_t last_upload_id = 0U;
    std::uint64_t last_staging_request_id = 0U;
    std::uint32_t loaded_byte_count = 0U;
    bool is_active = false;
};
}
