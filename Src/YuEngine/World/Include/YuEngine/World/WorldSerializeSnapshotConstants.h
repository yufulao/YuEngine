// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotConstants.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"

namespace yuengine::world {
inline constexpr std::uint32_t WORLD_SERIALIZE_WORLD_SNAPSHOT_FIELD_COUNT = 14U;
inline constexpr std::uint32_t WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_FIELD_COUNT = 7U;
inline constexpr std::uint32_t WORLD_SERIALIZE_PHASE_TRACE_FIELD_COUNT = 4U;
inline constexpr std::uint32_t WORLD_SERIALIZE_UINT32_FIELD_BYTE_COUNT =
    yuengine::serialize::FIELD_HEADER_BYTE_COUNT + yuengine::serialize::UINT32_PAYLOAD_BYTE_COUNT;
inline constexpr std::uint32_t WORLD_SERIALIZE_UINT64_FIELD_BYTE_COUNT =
    yuengine::serialize::FIELD_HEADER_BYTE_COUNT + yuengine::serialize::UINT64_PAYLOAD_BYTE_COUNT;
inline constexpr std::uint32_t WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_BYTE_COUNT =
    yuengine::serialize::RECORD_HEADER_BYTE_COUNT +
    (8U * WORLD_SERIALIZE_UINT32_FIELD_BYTE_COUNT) +
    (6U * WORLD_SERIALIZE_UINT64_FIELD_BYTE_COUNT);
inline constexpr std::uint32_t WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_BYTE_COUNT =
    yuengine::serialize::RECORD_HEADER_BYTE_COUNT +
    (5U * WORLD_SERIALIZE_UINT32_FIELD_BYTE_COUNT) +
    (2U * WORLD_SERIALIZE_UINT64_FIELD_BYTE_COUNT);
inline constexpr std::uint32_t WORLD_SERIALIZE_PHASE_TRACE_RECORD_BYTE_COUNT =
    yuengine::serialize::RECORD_HEADER_BYTE_COUNT +
    (3U * WORLD_SERIALIZE_UINT32_FIELD_BYTE_COUNT) +
    WORLD_SERIALIZE_UINT64_FIELD_BYTE_COUNT;
inline constexpr std::uint32_t MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT =
    (yuengine::serialize::MAX_FIELDS_PER_STREAM -
        WORLD_SERIALIZE_WORLD_SNAPSHOT_FIELD_COUNT -
        WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_FIELD_COUNT) /
    WORLD_SERIALIZE_PHASE_TRACE_FIELD_COUNT;

inline constexpr yuengine::serialize::SerializeRecordId WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID{1U};
inline constexpr yuengine::serialize::SerializeRecordId WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID{2U};
inline constexpr std::uint32_t WORLD_SERIALIZE_PHASE_TRACE_RECORD_ID_BASE = 100U;

inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_OBJECT_CAPACITY{1U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_PHASE_TRACE_CAPACITY{2U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_REGISTERED_OBJECT_COUNT{3U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_ACTIVE_OBJECT_COUNT{4U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_FRAME_COUNT{5U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_PHASE_EXECUTION_COUNT{6U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_SKIPPED_OBJECT_COUNT{7U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_LAST_FRAME_INDEX{8U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_LAST_FIXED_STEP_DURATION{9U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_LAST_FRAME_DELTA_DURATION{10U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_PHASE_TRACE_COUNT{11U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_ALLOCATION_STATUS{12U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_LIFECYCLE_STATE{13U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_FIELD_LAST_STATUS{14U};

inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_BRIDGE_CAPACITY{1U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_RECORD_COUNT{2U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_UPDATED_RECORD_COUNT{3U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_REMOVED_RECORD_COUNT{4U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_FAILED_OPERATION_COUNT{5U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_ALLOCATION_STATUS{6U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRANSFORM_FIELD_LAST_STATUS{7U};

inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRACE_FIELD_PHASE{1U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRACE_FIELD_FRAME_INDEX{2U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRACE_FIELD_ACTIVE_OBJECT_COUNT{3U};
inline constexpr yuengine::serialize::SerializeFieldId WORLD_SERIALIZE_TRACE_FIELD_SKIPPED_OBJECT_COUNT{4U};
}
