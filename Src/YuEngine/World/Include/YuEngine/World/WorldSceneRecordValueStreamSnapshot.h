// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneRecordValueStreamSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSceneRecordValueStreamStatus.h"

namespace yuengine::world {
struct WorldSceneRecordValueStreamSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint64_t write_count = 0U;
    std::uint64_t read_count = 0U;
    std::uint64_t written_identity_count = 0U;
    std::uint64_t written_transform_count = 0U;
    std::uint64_t written_attachment_count = 0U;
    std::uint64_t written_binding_count = 0U;
    std::uint64_t read_identity_count = 0U;
    std::uint64_t read_transform_count = 0U;
    std::uint64_t read_attachment_count = 0U;
    std::uint64_t read_binding_count = 0U;
    std::uint32_t rejected_record_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::serialize::SerializeStatus last_serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldSceneRecordValueStreamStatus last_status =
        WorldSceneRecordValueStreamStatus::Success;
};
}
