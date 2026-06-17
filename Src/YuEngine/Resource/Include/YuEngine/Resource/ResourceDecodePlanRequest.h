// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodePlanRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceDecodePlanRequest final {
    ResourceHandle resource;
    ResourceTypeId expected_type;
    std::uint64_t payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    ResourceDecodePlanAssetClass asset_class = ResourceDecodePlanAssetClass::Unknown;
    std::uint32_t source_byte_count = 0U;
    std::uint32_t expected_decoded_byte_count = 0U;
};
}
