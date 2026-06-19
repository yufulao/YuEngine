// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Src/RuntimeConfigStream.cpp

#include "YuEngine/Serialize/RuntimeConfigStream.h"

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeWriter.h"

namespace yuengine::serialize {
namespace {
constexpr SerializeRecordId RUNTIME_CONFIG_RECORD_ID{2601U};
constexpr SerializeRecordId RUNTIME_PROFILE_BOUNDARY_RECORD_ID{2602U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_SCHEMA_VERSION{1U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_FIXED_STEP_MICROSECONDS{2U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_MAX_FRAME_COUNT{3U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_COMMAND_SNAPSHOT_CAPACITY{4U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_DIAGNOSTICS_ENABLED{5U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_PROFILE_ID{1U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_SLOT_ID{2U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_KIND{3U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_CALLER_POLICY_TAG{4U};
constexpr std::uint32_t RUNTIME_CONFIG_RECORD_COUNT = 2U;
constexpr std::uint32_t RUNTIME_CONFIG_FIELD_COUNT = 9U;
constexpr std::uint32_t RUNTIME_CONFIG_REQUIRED_BYTE_COUNT =
    (RUNTIME_CONFIG_RECORD_COUNT * RECORD_HEADER_BYTE_COUNT) +
    (RUNTIME_CONFIG_FIELD_COUNT * (FIELD_HEADER_BYTE_COUNT + UINT32_PAYLOAD_BYTE_COUNT));
}

SerializeStatus RuntimeConfigStream::WriteRuntimeConfig(
    SerializeWriter *writer,
    const RuntimeConfigRecord &config,
    const RuntimeProfileBoundary &boundary) const {
    if (writer == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    SerializeStatus status = ValidateConfig(config);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = ValidateBoundary(boundary);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = ValidateWriteBudget(*writer);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->BeginRecord(RUNTIME_CONFIG_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_CONFIG_FIELD_SCHEMA_VERSION, config.schema_version);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_CONFIG_FIELD_FIXED_STEP_MICROSECONDS, config.fixed_step_microseconds);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_CONFIG_FIELD_MAX_FRAME_COUNT, config.max_frame_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_CONFIG_FIELD_COMMAND_SNAPSHOT_CAPACITY, config.command_snapshot_capacity);
    if (status != SerializeStatus::Success) {
        return status;
    }

    std::uint32_t diagnostics_enabled = 0U;
    if (config.diagnostics_enabled) {
        diagnostics_enabled = 1U;
    }

    status = writer->WriteUInt32(RUNTIME_CONFIG_FIELD_DIAGNOSTICS_ENABLED, diagnostics_enabled);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->BeginRecord(RUNTIME_PROFILE_BOUNDARY_RECORD_ID);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_PROFILE_FIELD_PROFILE_ID, boundary.profile_id);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_PROFILE_FIELD_SLOT_ID, boundary.slot_id);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = writer->WriteUInt32(RUNTIME_PROFILE_FIELD_KIND, static_cast<std::uint32_t>(boundary.kind));
    if (status != SerializeStatus::Success) {
        return status;
    }

    return writer->WriteUInt32(RUNTIME_PROFILE_FIELD_CALLER_POLICY_TAG, boundary.caller_policy_tag);
}

SerializeStatus RuntimeConfigStream::ReadRuntimeConfig(
    SerializeReader *reader,
    RuntimeConfigRecord *out_config,
    RuntimeProfileBoundary *out_boundary) const {
    if (reader == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    if (out_config == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    if (out_boundary == nullptr) {
        return SerializeStatus::InvalidArgument;
    }

    RuntimeConfigRecord config{};
    RuntimeProfileBoundary boundary{};
    std::uint32_t diagnostics_enabled = 0U;
    std::uint32_t boundary_kind = 0U;
    SerializeStatus status = reader->ReadUInt32(
        RUNTIME_CONFIG_RECORD_ID,
        RUNTIME_CONFIG_FIELD_SCHEMA_VERSION,
        config.schema_version);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_CONFIG_RECORD_ID,
        RUNTIME_CONFIG_FIELD_FIXED_STEP_MICROSECONDS,
        config.fixed_step_microseconds);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_CONFIG_RECORD_ID,
        RUNTIME_CONFIG_FIELD_MAX_FRAME_COUNT,
        config.max_frame_count);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_CONFIG_RECORD_ID,
        RUNTIME_CONFIG_FIELD_COMMAND_SNAPSHOT_CAPACITY,
        config.command_snapshot_capacity);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_CONFIG_RECORD_ID,
        RUNTIME_CONFIG_FIELD_DIAGNOSTICS_ENABLED,
        diagnostics_enabled);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_PROFILE_BOUNDARY_RECORD_ID,
        RUNTIME_PROFILE_FIELD_PROFILE_ID,
        boundary.profile_id);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_PROFILE_BOUNDARY_RECORD_ID,
        RUNTIME_PROFILE_FIELD_SLOT_ID,
        boundary.slot_id);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_PROFILE_BOUNDARY_RECORD_ID,
        RUNTIME_PROFILE_FIELD_KIND,
        boundary_kind);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = reader->ReadUInt32(
        RUNTIME_PROFILE_BOUNDARY_RECORD_ID,
        RUNTIME_PROFILE_FIELD_CALLER_POLICY_TAG,
        boundary.caller_policy_tag);
    if (status != SerializeStatus::Success) {
        return status;
    }

    if (diagnostics_enabled > 1U) {
        return SerializeStatus::InvalidArgument;
    }

    boundary.kind = static_cast<RuntimeProfileBoundaryKind>(boundary_kind);
    config.diagnostics_enabled = diagnostics_enabled != 0U;
    status = ValidateConfig(config);
    if (status != SerializeStatus::Success) {
        return status;
    }

    status = ValidateBoundary(boundary);
    if (status != SerializeStatus::Success) {
        return status;
    }

    *out_config = config;
    *out_boundary = boundary;
    return SerializeStatus::Success;
}

SerializeStatus RuntimeConfigStream::ValidateConfig(const RuntimeConfigRecord &config) const {
    if (config.schema_version != RUNTIME_CONFIG_SCHEMA_VERSION) {
        return SerializeStatus::UnsupportedVersion;
    }

    if (config.fixed_step_microseconds == 0U) {
        return SerializeStatus::InvalidArgument;
    }

    if (config.max_frame_count == 0U) {
        return SerializeStatus::InvalidArgument;
    }

    if (config.command_snapshot_capacity == 0U) {
        return SerializeStatus::InvalidArgument;
    }

    return SerializeStatus::Success;
}

SerializeStatus RuntimeConfigStream::ValidateBoundary(const RuntimeProfileBoundary &boundary) const {
    if (boundary.profile_id == 0U) {
        return SerializeStatus::InvalidArgument;
    }

    if (!IsBoundaryKindValid(boundary.kind)) {
        return SerializeStatus::InvalidArgument;
    }

    return SerializeStatus::Success;
}

SerializeStatus RuntimeConfigStream::ValidateWriteBudget(const SerializeWriter &writer) const {
    const SerializeSnapshot snapshot = writer.Snapshot();
    if (snapshot.major_version != STREAM_MAJOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (snapshot.minor_version != STREAM_MINOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (snapshot.committed_byte_count < STREAM_HEADER_BYTE_COUNT) {
        return SerializeStatus::InvalidHeader;
    }

    if (snapshot.record_count > MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count = MAX_RECORDS_PER_STREAM - snapshot.record_count;
    if (RUNTIME_CONFIG_RECORD_COUNT > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    if (snapshot.field_count > MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count = MAX_FIELDS_PER_STREAM - snapshot.field_count;
    if (RUNTIME_CONFIG_FIELD_COUNT > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    if (RUNTIME_CONFIG_REQUIRED_BYTE_COUNT > writer.GetRemainingByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(RUNTIME_CONFIG_REQUIRED_BYTE_COUNT)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

bool RuntimeConfigStream::IsBoundaryKindValid(RuntimeProfileBoundaryKind kind) const {
    if (kind == RuntimeProfileBoundaryKind::RuntimeConfig) {
        return true;
    }

    if (kind == RuntimeProfileBoundaryKind::SaveSnapshot) {
        return true;
    }

    return kind == RuntimeProfileBoundaryKind::ProfileSnapshot;
}
}
