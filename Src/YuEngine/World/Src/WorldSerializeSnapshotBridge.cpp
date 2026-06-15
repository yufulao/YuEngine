// Module: YuEngine World
// File: Src/YuEngine/World/Src/WorldSerializeSnapshotBridge.cpp

#include "YuEngine/World/WorldSerializeSnapshotBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldLifecycleState.h"
#include "YuEngine/World/WorldStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::serialize::SerializeFieldId;
using yuengine::serialize::SerializeReader;
using yuengine::serialize::SerializeRecordId;
using yuengine::serialize::SerializeSnapshot;
using yuengine::serialize::SerializeStatus;
using yuengine::serialize::SerializeWriter;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

SerializeRecordId PhaseTraceRecordId(std::uint32_t trace_index) {
    return SerializeRecordId{WORLD_SERIALIZE_PHASE_TRACE_RECORD_ID_BASE + trace_index};
}

bool IsMemoryAccountingStatusValid(std::uint32_t value) {
    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::Success)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::ExplicitlyTrackedOnly)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::InvalidOwner)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::InvalidTag)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::InvalidSize)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::InvalidAlignment)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::InvalidBudgetClass)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::BudgetExceeded)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::CapacityExceeded)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::UnmatchedFree)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(MemoryAccountingStatus::OwnerTagMismatch)) {
        return true;
    }

    return false;
}

bool IsWorldLifecycleStateValid(std::uint32_t value) {
    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Created)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Starting)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Running)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Stopping)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Stopped)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldLifecycleState::Failed)) {
        return true;
    }

    return false;
}

bool IsWorldStatusValid(std::uint32_t value) {
    if (value == static_cast<std::uint32_t>(WorldStatus::Success)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidObjectCapacity)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidPhaseTraceCapacity)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidObjectId)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::DuplicateObjectId)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::ObjectNotFound)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::CapacityExceeded)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidLifecycleState)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidTimeStep)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldStatus::InvalidTraceBuffer)) {
        return true;
    }

    return false;
}

bool IsWorldUpdatePhaseValid(std::uint32_t value) {
    if (value == static_cast<std::uint32_t>(WorldUpdatePhase::BeginFrame)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldUpdatePhase::FixedStep)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldUpdatePhase::FrameStep)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldUpdatePhase::EndFrame)) {
        return true;
    }

    return false;
}

bool IsWorldTransformStatusValid(std::uint32_t value) {
    if (value == static_cast<std::uint32_t>(WorldTransformStatus::Success)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::InvalidBridgeCapacity)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::InvalidWorldObjectId)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::MissingWorldObject)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::DuplicateWorldObjectId)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::CapacityExceeded)) {
        return true;
    }

    if (value == static_cast<std::uint32_t>(WorldTransformStatus::TransformNotFound)) {
        return true;
    }

    return false;
}

std::uint32_t RequiredWriteRecordCount(std::uint32_t phase_trace_count, bool has_transform_snapshot) {
    std::uint32_t result = 1U + phase_trace_count;
    if (has_transform_snapshot) {
        ++result;
    }

    return result;
}

std::uint32_t RequiredWriteFieldCount(std::uint32_t phase_trace_count, bool has_transform_snapshot) {
    std::uint32_t result = WORLD_SERIALIZE_WORLD_SNAPSHOT_FIELD_COUNT;
    result += phase_trace_count * WORLD_SERIALIZE_PHASE_TRACE_FIELD_COUNT;
    if (has_transform_snapshot) {
        result += WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_FIELD_COUNT;
    }

    return result;
}

std::uint32_t RequiredWriteByteCount(std::uint32_t phase_trace_count, bool has_transform_snapshot) {
    std::uint32_t result = WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_BYTE_COUNT;
    result += phase_trace_count * WORLD_SERIALIZE_PHASE_TRACE_RECORD_BYTE_COUNT;
    if (has_transform_snapshot) {
        result += WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_BYTE_COUNT;
    }

    return result;
}

SerializeStatus ValidateWriteBudget(
    const SerializeWriter &writer,
    std::uint32_t phase_trace_count,
    bool has_transform_snapshot) {
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.major_version != yuengine::serialize::STREAM_MAJOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.minor_version != yuengine::serialize::STREAM_MINOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count < yuengine::serialize::STREAM_HEADER_BYTE_COUNT) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count > yuengine::serialize::MAX_STREAM_BYTE_COUNT) {
        return SerializeStatus::BufferTooSmall;
    }

    const std::uint32_t required_record_count = RequiredWriteRecordCount(
        phase_trace_count,
        has_transform_snapshot);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(
        phase_trace_count,
        has_transform_snapshot);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(
        phase_trace_count,
        has_transform_snapshot);
    const std::uint32_t remaining_byte_count = writer.GetRemainingByteCapacity();
    if (required_byte_count > remaining_byte_count) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus WriteWorldSnapshotFields(SerializeWriter &writer, const WorldSnapshot &world_snapshot) {
    SerializeStatus status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_OBJECT_CAPACITY,
        world_snapshot.object_capacity);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_PHASE_TRACE_CAPACITY,
        world_snapshot.phase_trace_capacity);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_REGISTERED_OBJECT_COUNT,
        world_snapshot.registered_object_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_ACTIVE_OBJECT_COUNT,
        world_snapshot.active_object_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_FRAME_COUNT,
        world_snapshot.frame_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_PHASE_EXECUTION_COUNT,
        world_snapshot.phase_execution_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_SKIPPED_OBJECT_COUNT,
        world_snapshot.skipped_object_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FRAME_INDEX,
        world_snapshot.last_frame_index);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FIXED_STEP_DURATION,
        world_snapshot.last_fixed_step_duration);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FRAME_DELTA_DURATION,
        world_snapshot.last_frame_delta_duration);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_PHASE_TRACE_COUNT,
        world_snapshot.phase_trace_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_ALLOCATION_STATUS,
        static_cast<std::uint32_t>(world_snapshot.allocation_accounting_status));
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_FIELD_LIFECYCLE_STATE,
        static_cast<std::uint32_t>(world_snapshot.lifecycle_state));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(WORLD_SERIALIZE_FIELD_LAST_STATUS,
        static_cast<std::uint32_t>(world_snapshot.last_status));
}

SerializeStatus WritePhaseTraceFields(SerializeWriter &writer, const WorldPhaseTrace &phase_trace) {
    SerializeStatus status = writer.WriteUInt32(WORLD_SERIALIZE_TRACE_FIELD_PHASE,
        static_cast<std::uint32_t>(phase_trace.phase));
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_TRACE_FIELD_FRAME_INDEX,
        phase_trace.frame_index);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_TRACE_FIELD_ACTIVE_OBJECT_COUNT,
        phase_trace.active_object_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(WORLD_SERIALIZE_TRACE_FIELD_SKIPPED_OBJECT_COUNT,
        phase_trace.skipped_object_count);
}

SerializeStatus WriteTransformSnapshotFields(SerializeWriter &writer,
    const WorldTransformSnapshot &transform_snapshot) {
    SerializeStatus status = writer.WriteUInt32(WORLD_SERIALIZE_TRANSFORM_FIELD_BRIDGE_CAPACITY,
        transform_snapshot.bridge_capacity);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_TRANSFORM_FIELD_RECORD_COUNT,
        transform_snapshot.record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_TRANSFORM_FIELD_UPDATED_RECORD_COUNT,
        transform_snapshot.updated_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt64(WORLD_SERIALIZE_TRANSFORM_FIELD_REMOVED_RECORD_COUNT,
        transform_snapshot.removed_record_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_TRANSFORM_FIELD_FAILED_OPERATION_COUNT,
        transform_snapshot.failed_operation_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer.WriteUInt32(WORLD_SERIALIZE_TRANSFORM_FIELD_ALLOCATION_STATUS,
        static_cast<std::uint32_t>(transform_snapshot.allocation_accounting_status));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer.WriteUInt32(WORLD_SERIALIZE_TRANSFORM_FIELD_LAST_STATUS,
        static_cast<std::uint32_t>(transform_snapshot.last_status));
}
}

WorldSerializeSnapshotBridge::WorldSerializeSnapshotBridge(WorldSerializeSnapshotBridgeDesc desc)
    : phase_trace_capacity_(ClampCapacity(desc.phase_trace_capacity, MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT)),
      snapshot_{
          ClampCapacity(desc.phase_trace_capacity, MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success,
          WorldSerializeSnapshotStatus::Success} {
    if (desc.phase_trace_capacity == 0U) {
        snapshot_.last_status = WorldSerializeSnapshotStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::WriteSnapshot(
    SerializeWriter *writer,
    const WorldSnapshot &world_snapshot,
    const WorldPhaseTrace *phase_trace,
    std::uint32_t phase_trace_count,
    const WorldTransformSnapshot *transform_snapshot) {
    if (writer == nullptr) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidWriter);
    }

    const WorldSerializeSnapshotStatus world_status = ValidateWorldSnapshot(world_snapshot, phase_trace_count);
    if (world_status != WorldSerializeSnapshotStatus::Success) {
        return RecordFailure(world_status);
    }

    const WorldSerializeSnapshotStatus trace_status = ValidatePhaseTrace(phase_trace, phase_trace_count);
    if (trace_status != WorldSerializeSnapshotStatus::Success) {
        return RecordFailure(trace_status);
    }

    if (transform_snapshot != nullptr) {
        const WorldSerializeSnapshotStatus transform_status = ValidateTransformSnapshot(*transform_snapshot);
        if (transform_status != WorldSerializeSnapshotStatus::Success) {
            return RecordFailure(transform_status);
        }
    }

    const bool has_transform_snapshot = transform_snapshot != nullptr;
    const SerializeStatus budget_status = ValidateWriteBudget(
        *writer,
        phase_trace_count,
        has_transform_snapshot);
    if (budget_status != SerializeStatus::Success) {
        return RecordSerializeFailure(budget_status);
    }

    SerializeStatus status = writer->BeginRecord(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = WriteWorldSnapshotFields(*writer, world_snapshot);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < phase_trace_count) {
        status = writer->BeginRecord(PhaseTraceRecordId(trace_index));
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = WritePhaseTraceFields(*writer, phase_trace[trace_index]);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        ++trace_index;
    }

    std::uint32_t transform_snapshot_count = 0U;
    if (transform_snapshot == nullptr) {
        ++snapshot_.skipped_optional_record_count;
    }

    if (transform_snapshot != nullptr) {
        status = writer->BeginRecord(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = WriteTransformSnapshotFields(*writer, *transform_snapshot);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        transform_snapshot_count = 1U;
    }

    WorldSerializeSnapshotState state{};
    state.world_snapshot_count = 1U;
    state.phase_trace_count = phase_trace_count;
    state.transform_snapshot_count = transform_snapshot_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::ReadSnapshot(
    SerializeReader *reader,
    WorldSnapshot *out_world_snapshot,
    WorldPhaseTrace *out_phase_trace,
    std::uint32_t out_phase_trace_capacity,
    std::uint32_t *out_phase_trace_count,
    WorldTransformSnapshot *out_transform_snapshot) {
    if (reader == nullptr) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidReader);
    }

    const WorldSerializeSnapshotStatus output_status = ValidateReadOutputs(
        out_world_snapshot,
        out_phase_trace,
        out_phase_trace_capacity,
        out_phase_trace_count);
    if (output_status != WorldSerializeSnapshotStatus::Success) {
        return RecordFailure(output_status);
    }

    std::uint32_t raw_allocation_status = 0U;
    std::uint32_t raw_lifecycle_state = 0U;
    std::uint32_t raw_world_status = 0U;
    WorldSnapshot world_snapshot{};
    SerializeStatus status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_OBJECT_CAPACITY,
        world_snapshot.object_capacity);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_PHASE_TRACE_CAPACITY,
        world_snapshot.phase_trace_capacity);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_REGISTERED_OBJECT_COUNT,
        world_snapshot.registered_object_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_ACTIVE_OBJECT_COUNT,
        world_snapshot.active_object_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_FRAME_COUNT,
        world_snapshot.frame_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_PHASE_EXECUTION_COUNT,
        world_snapshot.phase_execution_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_SKIPPED_OBJECT_COUNT,
        world_snapshot.skipped_object_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_LAST_FRAME_INDEX,
        world_snapshot.last_frame_index);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_LAST_FIXED_STEP_DURATION,
        world_snapshot.last_fixed_step_duration);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt64(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_LAST_FRAME_DELTA_DURATION,
        world_snapshot.last_frame_delta_duration);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_PHASE_TRACE_COUNT,
        world_snapshot.phase_trace_count);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_ALLOCATION_STATUS,
        raw_allocation_status);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_LIFECYCLE_STATE,
        raw_lifecycle_state);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    status = reader->ReadUInt32(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID,
        WORLD_SERIALIZE_FIELD_LAST_STATUS,
        raw_world_status);
    if (status != SerializeStatus::Success) {
        return RecordSerializeFailure(status);
    }

    if (!IsMemoryAccountingStatusValid(raw_allocation_status)) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
    }

    if (!IsWorldLifecycleStateValid(raw_lifecycle_state)) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
    }

    if (!IsWorldStatusValid(raw_world_status)) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
    }

    world_snapshot.allocation_accounting_status = static_cast<MemoryAccountingStatus>(raw_allocation_status);
    world_snapshot.lifecycle_state = static_cast<WorldLifecycleState>(raw_lifecycle_state);
    world_snapshot.last_status = static_cast<WorldStatus>(raw_world_status);

    const WorldSerializeSnapshotStatus world_status = ValidateWorldSnapshot(
        world_snapshot,
        world_snapshot.phase_trace_count);
    if (world_status != WorldSerializeSnapshotStatus::Success) {
        return RecordFailure(world_status);
    }

    if (world_snapshot.phase_trace_count > out_phase_trace_capacity) {
        return RecordFailure(WorldSerializeSnapshotStatus::TraceCapacityExceeded);
    }

    if (world_snapshot.phase_trace_count > 0U && out_phase_trace == nullptr) {
        return RecordFailure(WorldSerializeSnapshotStatus::InvalidOutputTraceBuffer);
    }

    std::array<WorldPhaseTrace, MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT> phase_trace_buffer{};
    std::uint32_t trace_index = 0U;
    while (trace_index < world_snapshot.phase_trace_count) {
        std::uint32_t raw_phase = 0U;
        WorldPhaseTrace &phase_trace = phase_trace_buffer[trace_index];
        status = reader->ReadUInt32(PhaseTraceRecordId(trace_index),
            WORLD_SERIALIZE_TRACE_FIELD_PHASE,
            raw_phase);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        if (!IsWorldUpdatePhaseValid(raw_phase)) {
            return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
        }

        phase_trace.phase = static_cast<WorldUpdatePhase>(raw_phase);
        status = reader->ReadUInt64(PhaseTraceRecordId(trace_index),
            WORLD_SERIALIZE_TRACE_FIELD_FRAME_INDEX,
            phase_trace.frame_index);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(PhaseTraceRecordId(trace_index),
            WORLD_SERIALIZE_TRACE_FIELD_ACTIVE_OBJECT_COUNT,
            phase_trace.active_object_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(PhaseTraceRecordId(trace_index),
            WORLD_SERIALIZE_TRACE_FIELD_SKIPPED_OBJECT_COUNT,
            phase_trace.skipped_object_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        ++trace_index;
    }

    WorldTransformSnapshot transform_snapshot{};
    std::uint32_t transform_snapshot_count = 0U;
    if (out_transform_snapshot == nullptr) {
        ++snapshot_.skipped_optional_record_count;
    }

    if (out_transform_snapshot != nullptr) {
        std::uint32_t raw_transform_allocation_status = 0U;
        std::uint32_t raw_transform_status = 0U;
        status = reader->ReadUInt32(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_BRIDGE_CAPACITY,
            transform_snapshot.bridge_capacity);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_RECORD_COUNT,
            transform_snapshot.record_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt64(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_UPDATED_RECORD_COUNT,
            transform_snapshot.updated_record_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt64(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_REMOVED_RECORD_COUNT,
            transform_snapshot.removed_record_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_FAILED_OPERATION_COUNT,
            transform_snapshot.failed_operation_count);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_ALLOCATION_STATUS,
            raw_transform_allocation_status);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        status = reader->ReadUInt32(WORLD_SERIALIZE_TRANSFORM_SNAPSHOT_RECORD_ID,
            WORLD_SERIALIZE_TRANSFORM_FIELD_LAST_STATUS,
            raw_transform_status);
        if (status != SerializeStatus::Success) {
            return RecordSerializeFailure(status);
        }

        if (!IsMemoryAccountingStatusValid(raw_transform_allocation_status)) {
            return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
        }

        if (!IsWorldTransformStatusValid(raw_transform_status)) {
            return RecordFailure(WorldSerializeSnapshotStatus::InvalidEnumValue);
        }

        transform_snapshot.allocation_accounting_status =
            static_cast<MemoryAccountingStatus>(raw_transform_allocation_status);
        transform_snapshot.last_status = static_cast<WorldTransformStatus>(raw_transform_status);
        const WorldSerializeSnapshotStatus transform_status = ValidateTransformSnapshot(transform_snapshot);
        if (transform_status != WorldSerializeSnapshotStatus::Success) {
            return RecordFailure(transform_status);
        }

        transform_snapshot_count = 1U;
    }

    *out_world_snapshot = world_snapshot;
    trace_index = 0U;
    while (trace_index < world_snapshot.phase_trace_count) {
        out_phase_trace[trace_index] = phase_trace_buffer[trace_index];
        ++trace_index;
    }

    *out_phase_trace_count = world_snapshot.phase_trace_count;
    if (out_transform_snapshot != nullptr) {
        *out_transform_snapshot = transform_snapshot;
    }

    WorldSerializeSnapshotState state{};
    state.world_snapshot_count = 1U;
    state.phase_trace_count = world_snapshot.phase_trace_count;
    state.transform_snapshot_count = transform_snapshot_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldSerializeSnapshotBridgeSnapshot WorldSerializeSnapshotBridge::Snapshot() const {
    return snapshot_;
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::RecordFailure(WorldSerializeSnapshotStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    return WorldSerializeSnapshotResult::Failure(status);
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::RecordSerializeFailure(SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = WorldSerializeSnapshotStatus::SerializeFailure;
    snapshot_.last_serialize_status = status;
    return WorldSerializeSnapshotResult::Failure(WorldSerializeSnapshotStatus::SerializeFailure, status);
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::RecordWriteSuccess(const WorldSerializeSnapshotState &state) {
    snapshot_.last_status = WorldSerializeSnapshotStatus::Success;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.written_snapshot_count += state.world_snapshot_count;
    snapshot_.written_trace_count += state.phase_trace_count;
    snapshot_.written_transform_snapshot_count += state.transform_snapshot_count;
    return WorldSerializeSnapshotResult::Success(state);
}

WorldSerializeSnapshotResult WorldSerializeSnapshotBridge::RecordReadSuccess(const WorldSerializeSnapshotState &state) {
    snapshot_.last_status = WorldSerializeSnapshotStatus::Success;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.read_snapshot_count += state.world_snapshot_count;
    snapshot_.read_trace_count += state.phase_trace_count;
    snapshot_.read_transform_snapshot_count += state.transform_snapshot_count;
    return WorldSerializeSnapshotResult::Success(state);
}

WorldSerializeSnapshotStatus WorldSerializeSnapshotBridge::ValidateWorldSnapshot(
    const WorldSnapshot &world_snapshot,
    std::uint32_t phase_trace_count) const {
    if (phase_trace_capacity_ == 0U) {
        return WorldSerializeSnapshotStatus::InvalidBridgeCapacity;
    }

    if (world_snapshot.object_capacity > MAX_WORLD_OBJECT_COUNT) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (world_snapshot.phase_trace_capacity > MAX_WORLD_PHASE_TRACE_COUNT) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (world_snapshot.registered_object_count > world_snapshot.object_capacity) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (world_snapshot.active_object_count > world_snapshot.registered_object_count) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (world_snapshot.phase_trace_count != phase_trace_count) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (phase_trace_count > phase_trace_capacity_) {
        return WorldSerializeSnapshotStatus::TraceCapacityExceeded;
    }

    if (phase_trace_count > MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT) {
        return WorldSerializeSnapshotStatus::TraceCapacityExceeded;
    }

    if (phase_trace_count > world_snapshot.phase_trace_capacity) {
        return WorldSerializeSnapshotStatus::InvalidWorldSnapshot;
    }

    if (!IsMemoryAccountingStatusValid(static_cast<std::uint32_t>(world_snapshot.allocation_accounting_status))) {
        return WorldSerializeSnapshotStatus::InvalidEnumValue;
    }

    if (!IsWorldLifecycleStateValid(static_cast<std::uint32_t>(world_snapshot.lifecycle_state))) {
        return WorldSerializeSnapshotStatus::InvalidEnumValue;
    }

    if (!IsWorldStatusValid(static_cast<std::uint32_t>(world_snapshot.last_status))) {
        return WorldSerializeSnapshotStatus::InvalidEnumValue;
    }

    return WorldSerializeSnapshotStatus::Success;
}

WorldSerializeSnapshotStatus WorldSerializeSnapshotBridge::ValidateTransformSnapshot(
    const WorldTransformSnapshot &transform_snapshot) const {
    if (transform_snapshot.bridge_capacity > MAX_WORLD_OBJECT_COUNT) {
        return WorldSerializeSnapshotStatus::InvalidTransformSnapshot;
    }

    if (transform_snapshot.record_count > transform_snapshot.bridge_capacity) {
        return WorldSerializeSnapshotStatus::InvalidTransformSnapshot;
    }

    if (!IsMemoryAccountingStatusValid(static_cast<std::uint32_t>(transform_snapshot.allocation_accounting_status))) {
        return WorldSerializeSnapshotStatus::InvalidEnumValue;
    }

    if (!IsWorldTransformStatusValid(static_cast<std::uint32_t>(transform_snapshot.last_status))) {
        return WorldSerializeSnapshotStatus::InvalidEnumValue;
    }

    return WorldSerializeSnapshotStatus::Success;
}

WorldSerializeSnapshotStatus WorldSerializeSnapshotBridge::ValidatePhaseTrace(
    const WorldPhaseTrace *phase_trace,
    std::uint32_t phase_trace_count) const {
    if (phase_trace_count > phase_trace_capacity_) {
        return WorldSerializeSnapshotStatus::TraceCapacityExceeded;
    }

    if (phase_trace_count > MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT) {
        return WorldSerializeSnapshotStatus::TraceCapacityExceeded;
    }

    if (phase_trace_count > 0U && phase_trace == nullptr) {
        return WorldSerializeSnapshotStatus::InvalidTraceBuffer;
    }

    std::uint32_t index = 0U;
    while (index < phase_trace_count) {
        const std::uint32_t phase_value = static_cast<std::uint32_t>(phase_trace[index].phase);
        if (!IsWorldUpdatePhaseValid(phase_value)) {
            return WorldSerializeSnapshotStatus::InvalidEnumValue;
        }

        ++index;
    }

    return WorldSerializeSnapshotStatus::Success;
}

WorldSerializeSnapshotStatus WorldSerializeSnapshotBridge::ValidateReadOutputs(
    WorldSnapshot *out_world_snapshot,
    WorldPhaseTrace *out_phase_trace,
    std::uint32_t out_phase_trace_capacity,
    std::uint32_t *out_phase_trace_count) const {
    if (out_world_snapshot == nullptr) {
        return WorldSerializeSnapshotStatus::InvalidOutputWorldSnapshot;
    }

    if (out_phase_trace_count == nullptr) {
        return WorldSerializeSnapshotStatus::InvalidOutputTraceCount;
    }

    if (out_phase_trace_capacity > 0U && out_phase_trace == nullptr) {
        return WorldSerializeSnapshotStatus::InvalidOutputTraceBuffer;
    }

    return WorldSerializeSnapshotStatus::Success;
}
}
