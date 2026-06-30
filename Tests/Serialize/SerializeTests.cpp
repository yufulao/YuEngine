// 模块：Tests Serialize
// 文件：Tests/Serialize/SerializeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <unordered_map>

#include "StreamFixture.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/RuntimeConfigRecord.h"
#include "YuEngine/Serialize/RuntimeConfigStream.h"
#include "YuEngine/Serialize/RuntimeProfileBoundary.h"
#include "YuEngine/Serialize/RuntimeProfileBoundaryKind.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeWriter.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::serialize::RuntimeConfigRecord;
using yuengine::serialize::RuntimeConfigStream;
using yuengine::serialize::RuntimeProfileBoundary;
using yuengine::serialize::RuntimeProfileBoundaryKind;
using SerializeFieldId = yuengine::serialize::SerializeFieldId;
using SerializeReader = yuengine::serialize::SerializeReader;
using SerializeRecordId = yuengine::serialize::SerializeRecordId;
using SerializeSnapshot = yuengine::serialize::SerializeSnapshot;
using yuengine::serialize::SerializeStatus;
using yuengine::serialize::SerializeTypeTag;
using SerializeWriter = yuengine::serialize::SerializeWriter;
using yuengine::serialize::Tests::StreamFixture;
using yuengine::serialize::FIELD_HEADER_BYTE_COUNT;
using yuengine::serialize::MAX_FIELD_PAYLOAD_BYTE_COUNT;
using yuengine::serialize::MAX_FIELDS_PER_RECORD;
using yuengine::serialize::MAX_RECORDS_PER_STREAM;
using yuengine::serialize::MAX_STREAM_BYTE_COUNT;
using yuengine::serialize::RECORD_HEADER_BYTE_COUNT;
using yuengine::serialize::RUNTIME_CONFIG_SCHEMA_VERSION;
using yuengine::serialize::STREAM_FLAGS;
using yuengine::serialize::STREAM_FLAGS_OFFSET;
using yuengine::serialize::STREAM_HEADER_BYTE_COUNT;
using yuengine::serialize::STREAM_MAGIC;
using yuengine::serialize::STREAM_MAGIC_OFFSET;
using yuengine::serialize::STREAM_MAJOR_VERSION;
using yuengine::serialize::STREAM_MAJOR_VERSION_OFFSET;
using yuengine::serialize::STREAM_MINOR_VERSION;
using yuengine::serialize::STREAM_MINOR_VERSION_OFFSET;
using yuengine::serialize::STREAM_RECORD_COUNT_OFFSET;
using yuengine::serialize::UINT32_PAYLOAD_BYTE_COUNT;

namespace {
constexpr const char* TEST_ROUND_TRIP = "Serialize_WriteReadPrimitives_RoundTripsDeterministically";
constexpr const char* TEST_HEADER = "Serialize_StreamHeader_RejectsInvalidMagicOrVersion";
constexpr const char* TEST_RESERVED_FLAGS = "Serialize_StreamHeader_RejectsReservedFlags";
constexpr const char* TEST_WRITER_OVERFLOW = "Serialize_WriterBufferOverflow_ReturnsStatusWithoutOverrun";
constexpr const char* TEST_RECORD_CAPACITY = "Serialize_RecordCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIELD_CAPACITY = "Serialize_FieldCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIXED_BYTES_LIMIT = "Serialize_FixedBytesPayloadLimit_ReturnsExplicitStatus";
constexpr const char* TEST_FIXED_BYTES_FAILURE_COUNT = "Serialize_ReadFixedBytesFailures_ClearOutputCount";
constexpr const char* TEST_TRUNCATED = "Serialize_ReaderRejectsTruncatedStream";
constexpr const char* TEST_INVALID_IDS = "Serialize_ReaderRejectsInvalidRecordOrFieldId";
constexpr const char* TEST_MALFORMED_LENGTH = "Serialize_ReaderRejectsMalformedFieldLength";
constexpr const char* TEST_UNKNOWN_TYPE = "Serialize_ReaderRejectsUnknownTypeTag";
constexpr const char* TEST_TYPE_MISMATCH = "Serialize_ReaderTypeMismatch_ReturnsExplicitStatus";
constexpr const char* TEST_DUPLICATE_FIELD = "Serialize_DuplicateField_ReturnsExplicitStatus";
constexpr const char* TEST_UNKNOWN_FIELD_SKIP = "Serialize_UnknownFieldWithValidLength_CanSkipDeterministically";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Serialize_DisabledDiagnostics_DoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Serialize_NoFilePackageResourceObjectOrGameAdapterDependency";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Serialize_NoHiddenAllocationInReadWritePath";
constexpr const char* TEST_SNAPSHOT = "Serialize_SnapshotReportsCountsAndLastStatus";
constexpr const char* TEST_SUCCESS_LAST_STATUS = "Serialize_SuccessClearsLastStatusAfterFailure";
constexpr const char *TEST_RUNTIME_CONFIG_ROUNDTRIP =
    "Serialize_RuntimeConfigStream_RoundTripsCallerOwnedConfigBoundary";
constexpr const char *TEST_RUNTIME_CONFIG_UNSUPPORTED =
    "Serialize_RuntimeConfigStream_RejectsUnsupportedVersionWithoutMutation";
constexpr const char *TEST_RUNTIME_CONFIG_BOUNDARY =
    "Serialize_RuntimeConfigStream_KeepsPersistencePolicyOutsideCore";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* UNDERSIZED_FIXED_BYTES_READ_MESSAGE = "undersized fixed bytes read did not fail";
constexpr const char* UNDERSIZED_FIXED_BYTES_COUNT_MESSAGE = "undersized fixed bytes read did not clear output byte count";
constexpr const char* UNDERSIZED_FIXED_BYTES_WRITE_MESSAGE = "undersized fixed bytes read wrote into caller buffer";
constexpr const char* FIXED_BYTES_FAILURE_COUNT_MESSAGE = "failed fixed bytes read did not clear output byte count";
constexpr const char* FIXED_BYTES_FAILURE_BUFFER_MESSAGE = "failed fixed bytes read changed caller buffer";
constexpr const char* REOPEN_VALID_STREAM_MESSAGE = "reader did not open valid stream before failed reopen";
constexpr const char* REOPEN_INVALID_MAGIC_MESSAGE = "invalid magic reopen did not return explicit status";
constexpr const char* REOPEN_STALE_STATE_MESSAGE = "failed reopen left reader usable with stale stream state";
constexpr const char* REOPEN_OUTPUT_MUTATION_MESSAGE = "read after failed reopen changed output value";
constexpr const char* ZERO_FIXED_BYTES_WRITE_MESSAGE = "zero-length fixed bytes write failed";
constexpr const char* ZERO_FIXED_BYTES_OPEN_MESSAGE = "zero-length fixed bytes stream did not open";
constexpr const char* ZERO_FIXED_BYTES_READ_MESSAGE = "zero-length fixed bytes read failed";
constexpr const char* ZERO_FIXED_BYTES_COUNT_MESSAGE = "zero-length fixed bytes read returned nonzero byte count";
constexpr const char* TYPE_MISMATCH_OUTPUT_MESSAGE = "reader type mismatch changed output value";

constexpr SerializeRecordId RECORD_MAIN{7U};
constexpr SerializeRecordId RECORD_SECONDARY{8U};
constexpr SerializeFieldId FIELD_U32{11U};
constexpr SerializeFieldId FIELD_I32{12U};
constexpr SerializeFieldId FIELD_U64{13U};
constexpr SerializeFieldId FIELD_I64{14U};
constexpr SerializeFieldId FIELD_BYTES{15U};
constexpr SerializeFieldId FIELD_UNKNOWN{16U};
constexpr SerializeRecordId RUNTIME_CONFIG_RECORD{2601U};
constexpr SerializeRecordId RUNTIME_PROFILE_BOUNDARY_RECORD{2602U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_SCHEMA_VERSION{1U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_FIXED_STEP_MICROSECONDS{2U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_MAX_FRAME_COUNT{3U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_COMMAND_SNAPSHOT_CAPACITY{4U};
constexpr SerializeFieldId RUNTIME_CONFIG_FIELD_DIAGNOSTICS_ENABLED{5U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_PROFILE_ID{1U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_SLOT_ID{2U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_KIND{3U};
constexpr SerializeFieldId RUNTIME_PROFILE_FIELD_CALLER_POLICY_TAG{4U};
constexpr std::uint8_t SENTINEL_BYTE = 0xCDU;
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

void WriteUInt16At(std::uint8_t* buffer, std::uint32_t offset, std::uint16_t value) {
    buffer[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    buffer[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void WriteUInt32At(std::uint8_t* buffer, std::uint32_t offset, std::uint32_t value) {
    buffer[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    buffer[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    buffer[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    buffer[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void WriteValidHeader(std::uint8_t* buffer, std::uint32_t record_count) {
    WriteUInt32At(buffer, STREAM_MAGIC_OFFSET, STREAM_MAGIC);
    WriteUInt16At(buffer, STREAM_MAJOR_VERSION_OFFSET, STREAM_MAJOR_VERSION);
    WriteUInt16At(buffer, STREAM_MINOR_VERSION_OFFSET, STREAM_MINOR_VERSION);
    WriteUInt32At(buffer, STREAM_FLAGS_OFFSET, STREAM_FLAGS);
    WriteUInt32At(buffer, STREAM_RECORD_COUNT_OFFSET, record_count);
}

std::uint32_t WriteRecordHeader(std::uint8_t* buffer, std::uint32_t offset, SerializeRecordId record, std::uint32_t field_count) {
    WriteUInt32At(buffer, offset, record.value);
    WriteUInt32At(buffer, offset + sizeof(std::uint32_t), field_count);
    return offset + RECORD_HEADER_BYTE_COUNT;
}

std::uint32_t WriteFieldHeader(
    std::uint8_t* buffer,
    std::uint32_t offset,
    SerializeFieldId field,
    std::uint32_t type,
    std::uint32_t byte_count) {
    WriteUInt32At(buffer, offset, field.value);
    WriteUInt32At(buffer, offset + sizeof(std::uint32_t), type);
    WriteUInt32At(buffer, offset + (sizeof(std::uint32_t) * 2U), byte_count);
    return offset + FIELD_HEADER_BYTE_COUNT;
}

bool BytesMatch(const std::uint8_t* left, const std::uint8_t* right, std::uint32_t byte_count) {
    std::uint32_t index = 0U;
    while (index < byte_count) {
        if (left[index] != right[index]) {
            return false;
        }

        ++index;
    }

    return true;
}

bool BytesFilledWith(const std::uint8_t* bytes, std::uint32_t byte_count, std::uint8_t value) {
    std::uint32_t index = 0U;
    while (index < byte_count) {
        if (bytes[index] != value) {
            return false;
        }

        ++index;
    }

    return true;
}

bool SnapshotsMatch(const SerializeSnapshot& left, const SerializeSnapshot& right) {
    if (left.major_version != right.major_version) {
        return false;
    }

    if (left.minor_version != right.minor_version) {
        return false;
    }

    if (left.committed_byte_count != right.committed_byte_count) {
        return false;
    }

    if (left.record_count != right.record_count) {
        return false;
    }

    if (left.field_count != right.field_count) {
        return false;
    }

    if (left.accepted_operation_count != right.accepted_operation_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

int BuildRoundTripFixture(StreamFixture &fixture) {
    fixture.buffer.fill(SENTINEL_BYTE);
    SerializeWriter writer(fixture.buffer.data(), static_cast<std::uint32_t>(fixture.buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    const std::array<std::uint8_t, 5U> bytes{1U, 3U, 5U, 7U, 9U};
    if (writer.WriteUInt32(FIELD_U32, 0xAABBCCDDU) != SerializeStatus::Success) {
        return Fail("write uint32 failed");
    }

    if (writer.WriteInt32(FIELD_I32, -25) != SerializeStatus::Success) {
        return Fail("write int32 failed");
    }

    if (writer.WriteUInt64(FIELD_U64, 0x1122334455667788ULL) != SerializeStatus::Success) {
        return Fail("write uint64 failed");
    }

    if (writer.WriteInt64(FIELD_I64, -6000000000LL) != SerializeStatus::Success) {
        return Fail("write int64 failed");
    }

    if (writer.WriteFixedBytes(FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size())) != SerializeStatus::Success) {
        return Fail("write fixed bytes failed");
    }

    fixture.snapshot = writer.Snapshot();
    fixture.byte_count = fixture.snapshot.committed_byte_count;
    return 0;
}

RuntimeConfigRecord BuildRuntimeConfigRecord() {
    RuntimeConfigRecord record{};
    record.fixed_step_microseconds = 16666U;
    record.max_frame_count = 8U;
    record.command_snapshot_capacity = 4U;
    record.diagnostics_enabled = true;
    return record;
}

RuntimeProfileBoundary BuildRuntimeProfileBoundary() {
    RuntimeProfileBoundary boundary{};
    boundary.profile_id = 23U;
    boundary.slot_id = 2U;
    boundary.kind = RuntimeProfileBoundaryKind::SaveSnapshot;
    boundary.caller_policy_tag = 77U;
    return boundary;
}

bool RuntimeConfigRecordsMatch(const RuntimeConfigRecord &left, const RuntimeConfigRecord &right) {
    if (left.schema_version != right.schema_version) {
        return false;
    }

    if (left.fixed_step_microseconds != right.fixed_step_microseconds) {
        return false;
    }

    if (left.max_frame_count != right.max_frame_count) {
        return false;
    }

    if (left.command_snapshot_capacity != right.command_snapshot_capacity) {
        return false;
    }

    return left.diagnostics_enabled == right.diagnostics_enabled;
}

bool RuntimeProfileBoundariesMatch(const RuntimeProfileBoundary &left, const RuntimeProfileBoundary &right) {
    if (left.profile_id != right.profile_id) {
        return false;
    }

    if (left.slot_id != right.slot_id) {
        return false;
    }

    if (left.kind != right.kind) {
        return false;
    }

    return left.caller_policy_tag == right.caller_policy_tag;
}

int WriteUnsupportedRuntimeConfigStream(StreamFixture &fixture) {
    fixture.buffer.fill(SENTINEL_BYTE);
    SerializeWriter writer(fixture.buffer.data(), static_cast<std::uint32_t>(fixture.buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin unsupported stream failed");
    }

    if (writer.BeginRecord(RUNTIME_CONFIG_RECORD) != SerializeStatus::Success) {
        return Fail("begin unsupported config record failed");
    }

    if (writer.WriteUInt32(RUNTIME_CONFIG_FIELD_SCHEMA_VERSION, RUNTIME_CONFIG_SCHEMA_VERSION + 1U) != SerializeStatus::Success) {
        return Fail("write unsupported config schema failed");
    }

    if (writer.WriteUInt32(RUNTIME_CONFIG_FIELD_FIXED_STEP_MICROSECONDS, 16666U) != SerializeStatus::Success) {
        return Fail("write unsupported config fixed step failed");
    }

    if (writer.WriteUInt32(RUNTIME_CONFIG_FIELD_MAX_FRAME_COUNT, 8U) != SerializeStatus::Success) {
        return Fail("write unsupported config frame count failed");
    }

    if (writer.WriteUInt32(RUNTIME_CONFIG_FIELD_COMMAND_SNAPSHOT_CAPACITY, 4U) != SerializeStatus::Success) {
        return Fail("write unsupported config command capacity failed");
    }

    if (writer.WriteUInt32(RUNTIME_CONFIG_FIELD_DIAGNOSTICS_ENABLED, 1U) != SerializeStatus::Success) {
        return Fail("write unsupported config diagnostics failed");
    }

    if (writer.BeginRecord(RUNTIME_PROFILE_BOUNDARY_RECORD) != SerializeStatus::Success) {
        return Fail("begin unsupported boundary record failed");
    }

    if (writer.WriteUInt32(RUNTIME_PROFILE_FIELD_PROFILE_ID, 23U) != SerializeStatus::Success) {
        return Fail("write unsupported boundary profile failed");
    }

    if (writer.WriteUInt32(RUNTIME_PROFILE_FIELD_SLOT_ID, 2U) != SerializeStatus::Success) {
        return Fail("write unsupported boundary slot failed");
    }

    if (writer.WriteUInt32(RUNTIME_PROFILE_FIELD_KIND, static_cast<std::uint32_t>(RuntimeProfileBoundaryKind::SaveSnapshot)) != SerializeStatus::Success) {
        return Fail("write unsupported boundary kind failed");
    }

    if (writer.WriteUInt32(RUNTIME_PROFILE_FIELD_CALLER_POLICY_TAG, 77U) != SerializeStatus::Success) {
        return Fail("write unsupported boundary policy tag failed");
    }

    fixture.snapshot = writer.Snapshot();
    fixture.byte_count = fixture.snapshot.committed_byte_count;
    return 0;
}

int SerializeWriteReadPrimitivesRoundTripsDeterministically() {
    StreamFixture first_fixture;
    StreamFixture second_fixture;
    if (BuildRoundTripFixture(first_fixture) != 0) {
        return 1;
    }

    if (BuildRoundTripFixture(second_fixture) != 0) {
        return 1;
    }

    if (first_fixture.byte_count != second_fixture.byte_count) {
        return Fail("deterministic stream byte counts did not match");
    }

    if (!BytesMatch(first_fixture.buffer.data(), second_fixture.buffer.data(), first_fixture.byte_count)) {
        return Fail("deterministic stream bytes did not match");
    }

    SerializeReader reader(first_fixture.buffer.data(), first_fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader did not open stream");
    }

    std::array<std::uint8_t, 4U> undersized_bytes{};
    undersized_bytes.fill(SENTINEL_BYTE);
    std::uint32_t rejected_byte_count = 77U;
    if (reader.ReadFixedBytes(
            RECORD_MAIN,
            FIELD_BYTES,
            undersized_bytes.data(),
            static_cast<std::uint32_t>(undersized_bytes.size()),
            rejected_byte_count) != SerializeStatus::BufferTooSmall) {
        return Fail(UNDERSIZED_FIXED_BYTES_READ_MESSAGE);
    }

    if (rejected_byte_count != 0U) {
        return Fail(UNDERSIZED_FIXED_BYTES_COUNT_MESSAGE);
    }

    if (undersized_bytes[0U] != SENTINEL_BYTE || undersized_bytes[3U] != SENTINEL_BYTE) {
        return Fail(UNDERSIZED_FIXED_BYTES_WRITE_MESSAGE);
    }

    std::uint32_t u32Value = 0U;
    std::int32_t i32Value = 0;
    std::uint64_t u64Value = 0U;
    std::int64_t i64Value = 0;
    std::array<std::uint8_t, 5U> bytes{};
    std::uint32_t byte_count = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, u32Value) != SerializeStatus::Success) {
        return Fail("read uint32 failed");
    }

    if (reader.ReadInt32(RECORD_MAIN, FIELD_I32, i32Value) != SerializeStatus::Success) {
        return Fail("read int32 failed");
    }

    if (reader.ReadUInt64(RECORD_MAIN, FIELD_U64, u64Value) != SerializeStatus::Success) {
        return Fail("read uint64 failed");
    }

    if (reader.ReadInt64(RECORD_MAIN, FIELD_I64, i64Value) != SerializeStatus::Success) {
        return Fail("read int64 failed");
    }

    if (reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size()), byte_count) !=
        SerializeStatus::Success) {
        return Fail("read fixed bytes failed");
    }

    if (u32Value != 0xAABBCCDDU || i32Value != -25 || u64Value != 0x1122334455667788ULL || i64Value != -6000000000LL) {
        return Fail("roundtrip primitive values did not match");
    }

    const std::array<std::uint8_t, 5U> expected_bytes{1U, 3U, 5U, 7U, 9U};
    if (byte_count != expected_bytes.size() || !BytesMatch(bytes.data(), expected_bytes.data(), byte_count)) {
        return Fail("roundtrip fixed bytes did not match");
    }

    return 0;
}

int SerializeStreamHeaderRejectsInvalidMagicOrVersion() {
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    fixture.buffer[0U] = 0U;
    SerializeReader invalid_magic_reader(fixture.buffer.data(), fixture.byte_count);
    if (invalid_magic_reader.OpenStream() != SerializeStatus::InvalidHeader) {
        return Fail("invalid magic did not return explicit status");
    }

    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    SerializeReader reopened_reader(fixture.buffer.data(), fixture.byte_count);
    if (reopened_reader.OpenStream() != SerializeStatus::Success) {
        return Fail(REOPEN_VALID_STREAM_MESSAGE);
    }

    fixture.buffer[0U] = 0U;
    if (reopened_reader.OpenStream() != SerializeStatus::InvalidHeader) {
        return Fail(REOPEN_INVALID_MAGIC_MESSAGE);
    }

    std::uint32_t stale_value = 99U;
    if (reopened_reader.ReadUInt32(RECORD_MAIN, FIELD_U32, stale_value) != SerializeStatus::InvalidHeader) {
        return Fail(REOPEN_STALE_STATE_MESSAGE);
    }

    if (stale_value != 99U) {
        return Fail(REOPEN_OUTPUT_MUTATION_MESSAGE);
    }

    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    WriteUInt16At(fixture.buffer.data(), STREAM_MAJOR_VERSION_OFFSET, 99U);
    SerializeReader unsupported_version_reader(fixture.buffer.data(), fixture.byte_count);
    if (unsupported_version_reader.OpenStream() != SerializeStatus::UnsupportedVersion) {
        return Fail("unsupported major version did not return explicit status");
    }

    return 0;
}

int SerializeStreamHeaderRejectsReservedFlags() {
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    WriteUInt32At(fixture.buffer.data(), STREAM_FLAGS_OFFSET, 1U);
    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::InvalidHeader) {
        return Fail("reserved stream flags did not return explicit header status");
    }

    return 0;
}

int SerializeWriterBufferOverflowReturnsStatusWithoutOverrun() {
    std::array<std::uint8_t, 8U> tiny_buffer{};
    tiny_buffer.fill(SENTINEL_BYTE);
    SerializeWriter tiny_writer(tiny_buffer.data(), static_cast<std::uint32_t>(tiny_buffer.size()));
    if (tiny_writer.BeginStream() != SerializeStatus::BufferTooSmall) {
        return Fail("undersized stream header did not fail");
    }

    if (tiny_buffer[0U] != SENTINEL_BYTE || tiny_buffer[7U] != SENTINEL_BYTE) {
        return Fail("undersized stream header wrote into caller buffer");
    }

    std::array<std::uint8_t, 39U> field_buffer{};
    field_buffer.fill(SENTINEL_BYTE);
    SerializeWriter field_writer(field_buffer.data(), static_cast<std::uint32_t>(field_buffer.size()));
    if (field_writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (field_writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    const SerializeSnapshot before_snapshot = field_writer.Snapshot();
    if (field_writer.WriteUInt32(FIELD_U32, 10U) != SerializeStatus::BufferTooSmall) {
        return Fail("undersized field payload did not fail");
    }

    if (field_writer.Snapshot().committed_byte_count != before_snapshot.committed_byte_count) {
        return Fail("undersized field write changed committed bytes");
    }

    if (field_buffer[before_snapshot.committed_byte_count] != SENTINEL_BYTE) {
        return Fail("undersized field write modified uncommitted bytes");
    }

    return 0;
}

int SerializeRecordCapacityOverflowDoesNotMutate() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    std::uint32_t record_index = 0U;
    while (record_index < MAX_RECORDS_PER_STREAM) {
        if (writer.BeginRecord(SerializeRecordId{record_index + 1U}) != SerializeStatus::Success) {
            return Fail("record within capacity failed");
        }

        ++record_index;
    }

    const SerializeSnapshot before_snapshot = writer.Snapshot();
    if (writer.BeginRecord(SerializeRecordId{100U}) != SerializeStatus::RecordCapacityExceeded) {
        return Fail("record capacity overflow did not return explicit status");
    }

    const SerializeSnapshot after_snapshot = writer.Snapshot();
    if (after_snapshot.record_count != before_snapshot.record_count) {
        return Fail("record overflow changed record count");
    }

    if (after_snapshot.committed_byte_count != before_snapshot.committed_byte_count) {
        return Fail("record overflow changed committed byte count");
    }

    return 0;
}

int SerializeFieldCapacityOverflowDoesNotMutate() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    std::uint32_t field_index = 0U;
    while (field_index < MAX_FIELDS_PER_RECORD) {
        if (writer.WriteUInt32(SerializeFieldId{field_index + 1U}, field_index) != SerializeStatus::Success) {
            return Fail("field within capacity failed");
        }

        ++field_index;
    }

    const SerializeSnapshot before_snapshot = writer.Snapshot();
    if (writer.WriteUInt32(SerializeFieldId{100U}, 100U) != SerializeStatus::FieldCapacityExceeded) {
        return Fail("field capacity overflow did not return explicit status");
    }

    const SerializeSnapshot after_snapshot = writer.Snapshot();
    if (after_snapshot.field_count != before_snapshot.field_count) {
        return Fail("field overflow changed field count");
    }

    if (after_snapshot.committed_byte_count != before_snapshot.committed_byte_count) {
        return Fail("field overflow changed committed byte count");
    }

    return 0;
}

int SerializeFixedBytesPayloadLimitReturnsExplicitStatus() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    std::array<std::uint8_t, MAX_FIELD_PAYLOAD_BYTE_COUNT + 1U> bytes{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    const SerializeSnapshot before_snapshot = writer.Snapshot();
    if (writer.WriteFixedBytes(FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size())) !=
        SerializeStatus::FieldPayloadTooLarge) {
        return Fail("oversized fixed bytes did not return explicit status");
    }

    if (writer.Snapshot().committed_byte_count != before_snapshot.committed_byte_count) {
        return Fail("oversized fixed bytes changed committed byte count");
    }

    if (writer.WriteFixedBytes(FIELD_BYTES, nullptr, 0U) != SerializeStatus::Success) {
        return Fail(ZERO_FIXED_BYTES_WRITE_MESSAGE);
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail(ZERO_FIXED_BYTES_OPEN_MESSAGE);
    }

    std::uint32_t byte_count = 7U;
    if (reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, nullptr, 0U, byte_count) != SerializeStatus::Success) {
        return Fail(ZERO_FIXED_BYTES_READ_MESSAGE);
    }

    if (byte_count != 0U) {
        return Fail(ZERO_FIXED_BYTES_COUNT_MESSAGE);
    }

    return 0;
}

int SerializeReadFixedBytesFailuresClearOutputCount() {
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open failed");
    }

    std::array<std::uint8_t, 5U> output{};
    output.fill(SENTINEL_BYTE);
    std::uint32_t byte_count = 99U;
    SerializeStatus status = reader.ReadFixedBytes(
        RECORD_MAIN,
        FIELD_UNKNOWN,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        byte_count);
    if (status != SerializeStatus::FieldNotFound) {
        return Fail("missing fixed bytes field did not return explicit status");
    }

    if (byte_count != 0U) {
        return Fail(FIXED_BYTES_FAILURE_COUNT_MESSAGE);
    }

    if (!BytesFilledWith(output.data(), static_cast<std::uint32_t>(output.size()), SENTINEL_BYTE)) {
        return Fail(FIXED_BYTES_FAILURE_BUFFER_MESSAGE);
    }

    output.fill(SENTINEL_BYTE);
    byte_count = 99U;
    status = reader.ReadFixedBytes(
        RECORD_MAIN,
        FIELD_U32,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        byte_count);
    if (status != SerializeStatus::TypeMismatch) {
        return Fail("fixed bytes type mismatch did not return explicit status");
    }

    if (byte_count != 0U) {
        return Fail(FIXED_BYTES_FAILURE_COUNT_MESSAGE);
    }

    if (!BytesFilledWith(output.data(), static_cast<std::uint32_t>(output.size()), SENTINEL_BYTE)) {
        return Fail(FIXED_BYTES_FAILURE_BUFFER_MESSAGE);
    }

    output.fill(SENTINEL_BYTE);
    byte_count = 99U;
    status = reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, output.data(), 4U, byte_count);
    if (status != SerializeStatus::BufferTooSmall) {
        return Fail(UNDERSIZED_FIXED_BYTES_READ_MESSAGE);
    }

    if (byte_count != 0U) {
        return Fail(FIXED_BYTES_FAILURE_COUNT_MESSAGE);
    }

    if (!BytesFilledWith(output.data(), static_cast<std::uint32_t>(output.size()), SENTINEL_BYTE)) {
        return Fail(FIXED_BYTES_FAILURE_BUFFER_MESSAGE);
    }

    byte_count = 99U;
    status = reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, nullptr, 5U, byte_count);
    if (status != SerializeStatus::BufferTooSmall) {
        return Fail("null fixed bytes output did not return explicit status");
    }

    if (byte_count != 0U) {
        return Fail(FIXED_BYTES_FAILURE_COUNT_MESSAGE);
    }

    return 0;
}

int SerializeReaderRejectsTruncatedStream() {
    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::TruncatedStream) {
        return Fail("truncated stream did not return explicit status");
    }

    return 0;
}

int SerializeReaderRejectsInvalidRecordOrFieldId() {
    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT + RECORD_HEADER_BYTE_COUNT>
        invalid_record_buffer{};
    WriteValidHeader(invalid_record_buffer.data(), 1U);
    WriteRecordHeader(
        invalid_record_buffer.data(),
        STREAM_HEADER_BYTE_COUNT,
        SerializeRecordId{0U},
        0U);
    SerializeReader invalid_record_reader(
        invalid_record_buffer.data(),
        static_cast<std::uint32_t>(invalid_record_buffer.size()));
    if (invalid_record_reader.OpenStream() != SerializeStatus::InvalidHeader) {
        return Fail("invalid record id did not return explicit header status");
    }

    std::array<std::uint8_t, 40U> invalid_field_buffer{};
    WriteValidHeader(invalid_field_buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(invalid_field_buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(
        invalid_field_buffer.data(),
        offset,
        SerializeFieldId{0U},
        static_cast<std::uint32_t>(SerializeTypeTag::UInt32),
        UINT32_PAYLOAD_BYTE_COUNT);
    WriteUInt32At(invalid_field_buffer.data(), offset, 1U);
    SerializeReader invalid_field_reader(invalid_field_buffer.data(), static_cast<std::uint32_t>(invalid_field_buffer.size()));
    if (invalid_field_reader.OpenStream() != SerializeStatus::InvalidHeader) {
        return Fail("invalid field id did not return explicit header status");
    }

    return 0;
}

int SerializeReaderRejectsMalformedFieldLength() {
    std::array<std::uint8_t, 44U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, static_cast<std::uint32_t>(SerializeTypeTag::UInt32), 8U);
    WriteUInt32At(buffer.data(), offset, 1U);
    WriteUInt32At(buffer.data(), offset + sizeof(std::uint32_t), 2U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::MalformedFieldLength) {
        return Fail("malformed field length did not return explicit status");
    }

    return 0;
}

int SerializeReaderRejectsUnknownTypeTag() {
    std::array<std::uint8_t, 40U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, 99U, UINT32_PAYLOAD_BYTE_COUNT);
    WriteUInt32At(buffer.data(), offset, 1U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::UnknownTypeTag) {
        return Fail("unknown type tag did not return explicit status");
    }

    return 0;
}

int SerializeReaderTypeMismatchReturnsExplicitStatus() {
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open failed");
    }

    std::int32_t value = -333;
    if (reader.ReadInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::TypeMismatch) {
        return Fail("reader type mismatch did not return explicit status");
    }

    if (value != -333) {
        return Fail(TYPE_MISMATCH_OUTPUT_MESSAGE);
    }

    return 0;
}

int SerializeDuplicateFieldReturnsExplicitStatus() {
    std::array<std::uint8_t, 56U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 2U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, static_cast<std::uint32_t>(SerializeTypeTag::UInt32), 4U);
    WriteUInt32At(buffer.data(), offset, 1U);
    offset += 4U;
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, static_cast<std::uint32_t>(SerializeTypeTag::UInt32), 4U);
    WriteUInt32At(buffer.data(), offset, 2U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::DuplicateField) {
        return Fail("duplicate field did not return explicit status");
    }

    return 0;
}

int SerializeUnknownFieldWithValidLengthCanSkipDeterministically() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_SECONDARY) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    const std::array<std::uint8_t, 3U> unknown_payload{2U, 4U, 6U};
    if (writer.WriteFixedBytes(FIELD_UNKNOWN, unknown_payload.data(), static_cast<std::uint32_t>(unknown_payload.size())) !=
        SerializeStatus::Success) {
        return Fail("write unknown field failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 55U) != SerializeStatus::Success) {
        return Fail("write target field failed");
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_SECONDARY, FIELD_U32, value) != SerializeStatus::Success) {
        return Fail("reader did not skip unknown field");
    }

    if (value != 55U) {
        return Fail("reader returned unexpected target field value");
    }

    return 0;
}

int SerializeDisabledDiagnosticsDoesNotChangeResults() {
    StreamFixture first_fixture;
    StreamFixture second_fixture;
    if (BuildRoundTripFixture(first_fixture) != 0) {
        return 1;
    }

    if (BuildRoundTripFixture(second_fixture) != 0) {
        return 1;
    }

    if (!SnapshotsMatch(first_fixture.snapshot, second_fixture.snapshot)) {
        return Fail("diagnostics-equivalent writer snapshots diverged");
    }

    SerializeReader first_reader(first_fixture.buffer.data(), first_fixture.byte_count);
    SerializeReader second_reader(second_fixture.buffer.data(), second_fixture.byte_count);
    if (first_reader.OpenStream() != second_reader.OpenStream()) {
        return Fail("diagnostics-equivalent reader open statuses diverged");
    }

    std::uint32_t first_value = 0U;
    std::uint32_t second_value = 0U;
    if (first_reader.ReadUInt32(RECORD_MAIN, FIELD_U32, first_value) != second_reader.ReadUInt32(RECORD_MAIN, FIELD_U32, second_value)) {
        return Fail("diagnostics-equivalent reader statuses diverged");
    }

    if (first_value != second_value) {
        return Fail("diagnostics-equivalent reader values diverged");
    }

    if (!SnapshotsMatch(first_reader.Snapshot(), second_reader.Snapshot())) {
        return Fail("diagnostics-equivalent reader snapshots diverged");
    }

    return 0;
}

int SerializeNoFilePackageResourceObjectOrGameAdapterDependency() {
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("synthetic stream reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::Success) {
        return Fail("synthetic stream read failed");
    }

    if (value != 0xAABBCCDDU) {
        return Fail("synthetic stream value changed");
    }

    return 0;
}

int SerializeNoHiddenAllocationInReadWritePath() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.Snapshot().allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("writer did not expose YuMemory accounting vocabulary");
    }

    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 3U) != SerializeStatus::Success) {
        return Fail("write uint32 failed");
    }

    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    SerializeReader reader(buffer.data(), writer_snapshot.committed_byte_count);
    if (reader.Snapshot().allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("reader did not expose YuMemory accounting vocabulary");
    }

    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::Success) {
        return Fail("reader read failed");
    }

    if (writer.Snapshot().committed_byte_count != writer_snapshot.committed_byte_count) {
        return Fail("read path changed writer committed bytes");
    }

    return 0;
}

int SerializeSnapshotReportsCountsAndLastStatus() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("begin record failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 1U) != SerializeStatus::Success) {
        return Fail("write first field failed");
    }

    if (writer.WriteInt32(FIELD_I32, -1) != SerializeStatus::Success) {
        return Fail("write second field failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 2U) != SerializeStatus::DuplicateField) {
        return Fail("duplicate field did not fail");
    }

    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.record_count != 1U || writer_snapshot.field_count != 2U) {
        return Fail("writer snapshot did not report record and field counts");
    }

    if (writer_snapshot.accepted_operation_count != 4U || writer_snapshot.failed_operation_count != 1U) {
        return Fail("writer snapshot did not report operation counts");
    }

    if (writer_snapshot.last_status != SerializeStatus::DuplicateField) {
        return Fail("writer snapshot did not report last status");
    }

    SerializeReader reader(buffer.data(), writer_snapshot.committed_byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open failed");
    }

    const SerializeSnapshot reader_snapshot = reader.Snapshot();
    if (reader_snapshot.record_count != 1U || reader_snapshot.field_count != 2U) {
        return Fail("reader snapshot did not report record and field counts");
    }

    if (reader_snapshot.committed_byte_count != writer_snapshot.committed_byte_count) {
        return Fail("reader snapshot did not report committed byte count");
    }

    if (reader_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("reader snapshot did not report YuMemory accounting vocabulary");
    }

    return 0;
}

int SerializeSuccessClearsLastStatusAfterFailure() {
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::InvalidHeader) {
        return Fail("writer pre-stream record did not fail");
    }

    SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.last_status != SerializeStatus::InvalidHeader || writer_snapshot.failed_operation_count != 1U) {
        return Fail("writer did not record initial failure status");
    }

    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("writer success did not recover after failure");
    }

    writer_snapshot = writer.Snapshot();
    if (writer_snapshot.last_status != SerializeStatus::Success || writer_snapshot.failed_operation_count != 1U) {
        return Fail("writer begin stream did not clear last failure status");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success) {
        return Fail("writer begin record failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 1U) != SerializeStatus::Success) {
        return Fail("writer first field failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 2U) != SerializeStatus::DuplicateField) {
        return Fail("writer duplicate field did not fail");
    }

    writer_snapshot = writer.Snapshot();
    if (writer_snapshot.last_status != SerializeStatus::DuplicateField || writer_snapshot.failed_operation_count != 2U) {
        return Fail("writer did not preserve duplicate field failure");
    }

    if (writer.WriteInt32(FIELD_I32, -2) != SerializeStatus::Success) {
        return Fail("writer second field did not recover after failure");
    }

    writer_snapshot = writer.Snapshot();
    if (writer_snapshot.last_status != SerializeStatus::Success || writer_snapshot.failed_operation_count != 2U) {
        return Fail("writer field success did not clear last failure status");
    }

    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0) {
        return 1;
    }

    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    std::uint32_t value = 77U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::InvalidHeader) {
        return Fail("reader pre-open read did not fail");
    }

    SerializeSnapshot reader_snapshot = reader.Snapshot();
    if (reader_snapshot.last_status != SerializeStatus::InvalidHeader || reader_snapshot.failed_operation_count != 1U) {
        return Fail("reader did not record initial failure status");
    }

    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("reader open did not recover after failure");
    }

    reader_snapshot = reader.Snapshot();
    if (reader_snapshot.last_status != SerializeStatus::Success || reader_snapshot.failed_operation_count != 1U) {
        return Fail("reader open did not clear last failure status");
    }

    std::int32_t signed_value = -77;
    if (reader.ReadInt32(RECORD_MAIN, FIELD_U32, signed_value) != SerializeStatus::TypeMismatch) {
        return Fail("reader type mismatch did not fail");
    }

    reader_snapshot = reader.Snapshot();
    if (reader_snapshot.last_status != SerializeStatus::TypeMismatch || reader_snapshot.failed_operation_count != 2U) {
        return Fail("reader did not preserve type mismatch failure");
    }

    value = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::Success) {
        return Fail("reader successful read did not recover after failure");
    }

    if (value != 0xAABBCCDDU) {
        return Fail("reader recovered read returned unexpected value");
    }

    reader_snapshot = reader.Snapshot();
    if (reader_snapshot.last_status != SerializeStatus::Success || reader_snapshot.failed_operation_count != 2U) {
        return Fail("reader read success did not clear last failure status");
    }

    return 0;
}

int SerializeRuntimeConfigStreamRoundTripsCallerOwnedConfigBoundary() {
    RuntimeConfigStream stream;
    RuntimeConfigRecord input_config = BuildRuntimeConfigRecord();
    RuntimeProfileBoundary input_boundary = BuildRuntimeProfileBoundary();
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("runtime config begin stream failed");
    }

    if (stream.WriteRuntimeConfig(&writer, input_config, input_boundary) != SerializeStatus::Success) {
        return Fail("runtime config write failed");
    }

    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.record_count != 2U) {
        return Fail("runtime config record count mismatch");
    }

    if (writer_snapshot.field_count != 9U) {
        return Fail("runtime config field count mismatch");
    }

    SerializeReader reader(buffer.data(), writer_snapshot.committed_byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("runtime config reader open failed");
    }

    RuntimeConfigRecord output_config{};
    RuntimeProfileBoundary output_boundary{};
    if (stream.ReadRuntimeConfig(&reader, &output_config, &output_boundary) != SerializeStatus::Success) {
        return Fail("runtime config read failed");
    }

    if (!RuntimeConfigRecordsMatch(input_config, output_config)) {
        return Fail("runtime config record mismatch");
    }

    if (!RuntimeProfileBoundariesMatch(input_boundary, output_boundary)) {
        return Fail("runtime config boundary mismatch");
    }

    return 0;
}

int SerializeRuntimeConfigStreamRejectsUnsupportedVersionWithoutMutation() {
    RuntimeConfigStream stream;
    StreamFixture fixture;
    if (WriteUnsupportedRuntimeConfigStream(fixture) != 0) {
        return 1;
    }

    SerializeReader reader(fixture.buffer.data(), fixture.byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("unsupported runtime config reader open failed");
    }

    RuntimeConfigRecord output_config = BuildRuntimeConfigRecord();
    output_config.fixed_step_microseconds = 30000U;
    RuntimeConfigRecord original_config = output_config;
    RuntimeProfileBoundary output_boundary = BuildRuntimeProfileBoundary();
    output_boundary.profile_id = 99U;
    RuntimeProfileBoundary original_boundary = output_boundary;
    if (stream.ReadRuntimeConfig(&reader, &output_config, &output_boundary) != SerializeStatus::UnsupportedVersion) {
        return Fail("unsupported runtime config version did not reject");
    }

    if (!RuntimeConfigRecordsMatch(original_config, output_config)) {
        return Fail("unsupported runtime config mutated output config");
    }

    if (!RuntimeProfileBoundariesMatch(original_boundary, output_boundary)) {
        return Fail("unsupported runtime config mutated output boundary");
    }

    return 0;
}

int SerializeRuntimeConfigStreamKeepsPersistencePolicyOutsideCore() {
    RuntimeConfigStream stream;
    RuntimeConfigRecord config = BuildRuntimeConfigRecord();
    RuntimeProfileBoundary boundary = BuildRuntimeProfileBoundary();
    boundary.kind = RuntimeProfileBoundaryKind::ProfileSnapshot;
    boundary.caller_policy_tag = 1001U;
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success) {
        return Fail("profile boundary begin stream failed");
    }

    if (stream.WriteRuntimeConfig(&writer, config, boundary) != SerializeStatus::Success) {
        return Fail("profile boundary write failed");
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (reader.OpenStream() != SerializeStatus::Success) {
        return Fail("profile boundary reader open failed");
    }

    RuntimeConfigRecord output_config{};
    RuntimeProfileBoundary output_boundary{};
    if (stream.ReadRuntimeConfig(&reader, &output_config, &output_boundary) != SerializeStatus::Success) {
        return Fail("profile boundary read failed");
    }

    if (output_boundary.kind != RuntimeProfileBoundaryKind::ProfileSnapshot) {
        return Fail("profile boundary kind mismatch");
    }

    if (output_boundary.caller_policy_tag != boundary.caller_policy_tag) {
        return Fail("profile boundary policy tag mismatch");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_ROUND_TRIP, SerializeWriteReadPrimitivesRoundTripsDeterministically},
        {TEST_HEADER, SerializeStreamHeaderRejectsInvalidMagicOrVersion},
        {TEST_RESERVED_FLAGS, SerializeStreamHeaderRejectsReservedFlags},
        {TEST_WRITER_OVERFLOW, SerializeWriterBufferOverflowReturnsStatusWithoutOverrun},
        {TEST_RECORD_CAPACITY, SerializeRecordCapacityOverflowDoesNotMutate},
        {TEST_FIELD_CAPACITY, SerializeFieldCapacityOverflowDoesNotMutate},
        {TEST_FIXED_BYTES_LIMIT, SerializeFixedBytesPayloadLimitReturnsExplicitStatus},
        {TEST_FIXED_BYTES_FAILURE_COUNT, SerializeReadFixedBytesFailuresClearOutputCount},
        {TEST_TRUNCATED, SerializeReaderRejectsTruncatedStream},
        {TEST_INVALID_IDS, SerializeReaderRejectsInvalidRecordOrFieldId},
        {TEST_MALFORMED_LENGTH, SerializeReaderRejectsMalformedFieldLength},
        {TEST_UNKNOWN_TYPE, SerializeReaderRejectsUnknownTypeTag},
        {TEST_TYPE_MISMATCH, SerializeReaderTypeMismatchReturnsExplicitStatus},
        {TEST_DUPLICATE_FIELD, SerializeDuplicateFieldReturnsExplicitStatus},
        {TEST_UNKNOWN_FIELD_SKIP, SerializeUnknownFieldWithValidLengthCanSkipDeterministically},
        {TEST_DISABLED_DIAGNOSTICS, SerializeDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FORBIDDEN_DEPENDENCY, SerializeNoFilePackageResourceObjectOrGameAdapterDependency},
        {TEST_NO_HIDDEN_ALLOCATION, SerializeNoHiddenAllocationInReadWritePath},
        {TEST_SNAPSHOT, SerializeSnapshotReportsCountsAndLastStatus},
        {TEST_SUCCESS_LAST_STATUS, SerializeSuccessClearsLastStatusAfterFailure},
        {TEST_RUNTIME_CONFIG_ROUNDTRIP, SerializeRuntimeConfigStreamRoundTripsCallerOwnedConfigBoundary},
        {TEST_RUNTIME_CONFIG_UNSUPPORTED, SerializeRuntimeConfigStreamRejectsUnsupportedVersionWithoutMutation},
        {TEST_RUNTIME_CONFIG_BOUNDARY, SerializeRuntimeConfigStreamKeepsPersistencePolicyOutsideCore}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
