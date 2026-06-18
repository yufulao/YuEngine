// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"

namespace yuengine::resource {
struct ResourceResidencySnapshot final {
    std::uint32_t budget_byte_capacity = 0U;
    std::uint32_t resident_byte_count = 0U;
    std::uint32_t pinned_byte_count = 0U;
    std::uint32_t evictable_byte_count = 0U;
    std::uint32_t resident_resource_count = 0U;
    std::uint32_t pinned_resource_count = 0U;
    std::uint32_t evicted_resource_count = 0U;
    std::uint32_t residency_record_count = 0U;
    std::uint64_t admitted_resident_count = 0U;
    std::uint64_t pinned_resident_count = 0U;
    std::uint64_t unpinned_resident_count = 0U;
    std::uint64_t evicted_resident_count = 0U;
    std::uint64_t eviction_candidate_count = 0U;
    std::uint64_t eviction_candidate_miss_count = 0U;
    std::uint64_t rejected_residency_request_count = 0U;
    std::uint64_t budget_rejected_residency_count = 0U;
    ResourceResidencyStatus last_status = ResourceResidencyStatus::Success;
    ResourceResidencyState last_state = ResourceResidencyState::Unloaded;
    ResourceHandle last_candidate;
};
}
