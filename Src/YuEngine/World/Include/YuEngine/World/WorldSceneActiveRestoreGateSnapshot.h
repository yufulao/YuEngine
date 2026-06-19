// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"

namespace yuengine::world {
struct WorldSceneActiveRestoreGateSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t plan_scratch_capacity = 0U;
    std::uint32_t proof_scratch_capacity = 0U;
    std::uint32_t slice_scratch_capacity = 0U;
    std::uint32_t gate_output_capacity = 0U;
    std::uint64_t gate_attempt_count = 0U;
    std::uint64_t accepted_gate_count = 0U;
    std::uint64_t emitted_gate_record_count = 0U;
    std::uint64_t identity_gate_count = 0U;
    std::uint64_t transform_gate_count = 0U;
    std::uint64_t attachment_gate_count = 0U;
    std::uint64_t binding_gate_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldSceneApplyTimeRestoreProofStatus last_proof_status =
        WorldSceneApplyTimeRestoreProofStatus::Success;
    WorldSceneActiveRestoreGateStatus last_status =
        WorldSceneActiveRestoreGateStatus::Success;
};
}
