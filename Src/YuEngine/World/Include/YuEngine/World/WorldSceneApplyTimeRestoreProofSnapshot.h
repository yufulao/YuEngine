// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneApplyTimeRestoreProofSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t plan_scratch_capacity = 0U;
    std::uint32_t proof_capacity = 0U;
    std::uint32_t slice_capacity = 0U;
    std::uint64_t proof_attempt_count = 0U;
    std::uint64_t proven_identity_count = 0U;
    std::uint64_t proven_transform_count = 0U;
    std::uint64_t proven_attachment_count = 0U;
    std::uint64_t proven_binding_count = 0U;
    std::uint64_t emitted_slice_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldObjectIdentityStatus last_identity_status =
        WorldObjectIdentityStatus::Success;
    WorldTransformStatus last_transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus last_attachment_status =
        WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus last_binding_status =
        WorldComponentResourceBindingStatus::Success;
    yuengine::object::ObjectStatus last_object_status =
        yuengine::object::ObjectStatus::Success;
    yuengine::resource::ResourceStatus last_resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldSceneDecodedRestorePlanStatus last_plan_status =
        WorldSceneDecodedRestorePlanStatus::Success;
    WorldSceneApplyTimeRestoreProofStatus last_status =
        WorldSceneApplyTimeRestoreProofStatus::Success;
};
}
