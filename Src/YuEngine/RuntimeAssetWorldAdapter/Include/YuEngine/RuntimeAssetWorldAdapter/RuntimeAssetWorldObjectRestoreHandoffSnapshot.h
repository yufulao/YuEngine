// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffStatus.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectRestoreHandoffSnapshot final {
    std::uint64_t handoff_attempt_count = 0U;
    std::uint64_t accepted_handoff_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    std::uint64_t rejected_operation_count = 0U;
    std::uint64_t built_identity_count = 0U;
    std::uint64_t built_transform_count = 0U;
    std::uint64_t emitted_gate_record_count = 0U;
    std::uint64_t restored_identity_count = 0U;
    std::uint64_t restored_transform_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    RuntimeAssetWorldObjectRestoreHandoffStatus last_status =
        RuntimeAssetWorldObjectRestoreHandoffStatus::Success;
    RuntimeAssetWorldObjectAdapterStatus last_adapter_status =
        RuntimeAssetWorldObjectAdapterStatus::Success;
    yuengine::world::WorldSceneActiveRestoreGateStatus last_gate_status =
        yuengine::world::WorldSceneActiveRestoreGateStatus::Success;
    yuengine::world::WorldSceneApplyTimeRestoreProofStatus last_proof_status =
        yuengine::world::WorldSceneApplyTimeRestoreProofStatus::Success;
    yuengine::world::WorldSceneAssemblyStatus last_assembly_status =
        yuengine::world::WorldSceneAssemblyStatus::Success;
    yuengine::world::WorldSceneObjectTransformRestoreStatus last_restore_status =
        yuengine::world::WorldSceneObjectTransformRestoreStatus::Success;
};
}
