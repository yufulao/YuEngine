// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/SerializeTypeTag.h"

namespace yuengine::serialize {
using yuengine::memory::MemoryAccountingStatus;

struct SerializeSnapshot final {
    std::uint16_t major_version;
    std::uint16_t minor_version;
    std::uint32_t committed_byte_count;
    std::uint32_t record_count;
    std::uint32_t field_count;
    std::uint32_t accepted_operation_count;
    std::uint32_t failed_operation_count;
    MemoryAccountingStatus allocation_accounting_status;
    SerializeStatus last_status;
    std::uint32_t last_required_record_count = 0U;
    std::uint32_t last_required_field_count = 0U;
    SerializeRecordId last_failed_record_id{};
    SerializeRecordId last_failed_field_record_id{};
    SerializeFieldId last_failed_field_id{};
    SerializeTypeTag last_failed_field_type = SerializeTypeTag::UInt32;
    std::uint32_t last_failed_entry_index = 0U;
    std::uint32_t last_failed_field_capacity = 0U;
    std::uint32_t last_failed_field_count = 0U;
};
}
