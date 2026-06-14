#include <array>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include "StreamFixture.h"
#include "yuengine/memory/MemoryAccountingStatus.h"
#include "yuengine/serialize/SerializeConstants.h"
#include "yuengine/serialize/SerializeReader.h"
#include "yuengine/serialize/SerializeWriter.h"

using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;
using SerializeFieldId = yuengine::serialize::SerializeFieldId;
using SerializeReader = yuengine::serialize::SerializeReader;
using SerializeRecordId = yuengine::serialize::SerializeRecordId;
using SerializeSnapshot = yuengine::serialize::SerializeSnapshot;
using SerializeStatus = yuengine::serialize::SerializeStatus;
using SerializeTypeTag = yuengine::serialize::SerializeTypeTag;
using SerializeWriter = yuengine::serialize::SerializeWriter;
using StreamFixture = yuengine::serialize::tests::StreamFixture;
using yuengine::serialize::FIELD_HEADER_BYTE_COUNT;
using yuengine::serialize::MAX_FIELD_PAYLOAD_BYTE_COUNT;
using yuengine::serialize::MAX_FIELDS_PER_RECORD;
using yuengine::serialize::MAX_RECORDS_PER_STREAM;
using yuengine::serialize::MAX_STREAM_BYTE_COUNT;
using yuengine::serialize::RECORD_HEADER_BYTE_COUNT;
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

namespace
{
constexpr const char* TEST_ROUND_TRIP = "Serialize_WriteReadPrimitives_RoundTripsDeterministically";
constexpr const char* TEST_HEADER = "Serialize_StreamHeader_RejectsInvalidMagicOrVersion";
constexpr const char* TEST_RESERVED_FLAGS = "Serialize_StreamHeader_RejectsReservedFlags";
constexpr const char* TEST_WRITER_OVERFLOW = "Serialize_WriterBufferOverflow_ReturnsStatusWithoutOverrun";
constexpr const char* TEST_RECORD_CAPACITY = "Serialize_RecordCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIELD_CAPACITY = "Serialize_FieldCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIXED_BYTES_LIMIT = "Serialize_FixedBytesPayloadLimit_ReturnsExplicitStatus";
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
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* UNDERSIZED_FIXED_BYTES_READ_MESSAGE = "undersized fixed bytes read did not fail";
constexpr const char* UNDERSIZED_FIXED_BYTES_COUNT_MESSAGE = "undersized fixed bytes read changed output byte count";
constexpr const char* UNDERSIZED_FIXED_BYTES_WRITE_MESSAGE = "undersized fixed bytes read wrote into caller buffer";
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
constexpr std::uint8_t SENTINEL_BYTE = 0xCDU;
using TestFunction = int (*)();

int Fail(std::string_view message)
{
    std::cerr << message << '\n';
    return 1;
}

void WriteUInt16At(std::uint8_t* buffer, std::uint32_t offset, std::uint16_t value)
{
    buffer[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    buffer[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
}

void WriteUInt32At(std::uint8_t* buffer, std::uint32_t offset, std::uint32_t value)
{
    buffer[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    buffer[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    buffer[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    buffer[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void WriteValidHeader(std::uint8_t* buffer, std::uint32_t recordCount)
{
    WriteUInt32At(buffer, STREAM_MAGIC_OFFSET, STREAM_MAGIC);
    WriteUInt16At(buffer, STREAM_MAJOR_VERSION_OFFSET, STREAM_MAJOR_VERSION);
    WriteUInt16At(buffer, STREAM_MINOR_VERSION_OFFSET, STREAM_MINOR_VERSION);
    WriteUInt32At(buffer, STREAM_FLAGS_OFFSET, STREAM_FLAGS);
    WriteUInt32At(buffer, STREAM_RECORD_COUNT_OFFSET, recordCount);
}

std::uint32_t WriteRecordHeader(std::uint8_t* buffer, std::uint32_t offset, SerializeRecordId record, std::uint32_t fieldCount)
{
    WriteUInt32At(buffer, offset, record.Value);
    WriteUInt32At(buffer, offset + sizeof(std::uint32_t), fieldCount);
    return offset + RECORD_HEADER_BYTE_COUNT;
}

std::uint32_t WriteFieldHeader(
    std::uint8_t* buffer,
    std::uint32_t offset,
    SerializeFieldId field,
    std::uint32_t type,
    std::uint32_t byteCount)
{
    WriteUInt32At(buffer, offset, field.Value);
    WriteUInt32At(buffer, offset + sizeof(std::uint32_t), type);
    WriteUInt32At(buffer, offset + (sizeof(std::uint32_t) * 2U), byteCount);
    return offset + FIELD_HEADER_BYTE_COUNT;
}

bool BytesMatch(const std::uint8_t* left, const std::uint8_t* right, std::uint32_t byteCount)
{
    std::uint32_t index = 0U;
    while (index < byteCount)
    {
        if (left[index] != right[index])
        {
            return false;
        }

        ++index;
    }

    return true;
}

bool SnapshotsMatch(const SerializeSnapshot& left, const SerializeSnapshot& right)
{
    if (left.MajorVersion != right.MajorVersion)
    {
        return false;
    }

    if (left.MinorVersion != right.MinorVersion)
    {
        return false;
    }

    if (left.CommittedByteCount != right.CommittedByteCount)
    {
        return false;
    }

    if (left.RecordCount != right.RecordCount)
    {
        return false;
    }

    if (left.FieldCount != right.FieldCount)
    {
        return false;
    }

    if (left.AcceptedOperationCount != right.AcceptedOperationCount)
    {
        return false;
    }

    if (left.FailedOperationCount != right.FailedOperationCount)
    {
        return false;
    }

    if (left.AllocationAccountingStatus != right.AllocationAccountingStatus)
    {
        return false;
    }

    return left.LastStatus == right.LastStatus;
}

int BuildRoundTripFixture(StreamFixture& fixture)
{
    fixture.Buffer.fill(SENTINEL_BYTE);
    SerializeWriter writer(fixture.Buffer.data(), static_cast<std::uint32_t>(fixture.Buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    const std::array<std::uint8_t, 5U> bytes{1U, 3U, 5U, 7U, 9U};
    if (writer.WriteUInt32(FIELD_U32, 0xAABBCCDDU) != SerializeStatus::Success)
    {
        return Fail("write uint32 failed");
    }

    if (writer.WriteInt32(FIELD_I32, -25) != SerializeStatus::Success)
    {
        return Fail("write int32 failed");
    }

    if (writer.WriteUInt64(FIELD_U64, 0x1122334455667788ULL) != SerializeStatus::Success)
    {
        return Fail("write uint64 failed");
    }

    if (writer.WriteInt64(FIELD_I64, -6000000000LL) != SerializeStatus::Success)
    {
        return Fail("write int64 failed");
    }

    if (writer.WriteFixedBytes(FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size())) != SerializeStatus::Success)
    {
        return Fail("write fixed bytes failed");
    }

    fixture.Snapshot = writer.Snapshot();
    fixture.ByteCount = fixture.Snapshot.CommittedByteCount;
    return 0;
}

int SerializeWriteReadPrimitivesRoundTripsDeterministically()
{
    StreamFixture firstFixture;
    StreamFixture secondFixture;
    if (BuildRoundTripFixture(firstFixture) != 0)
    {
        return 1;
    }

    if (BuildRoundTripFixture(secondFixture) != 0)
    {
        return 1;
    }

    if (firstFixture.ByteCount != secondFixture.ByteCount)
    {
        return Fail("deterministic stream byte counts did not match");
    }

    if (!BytesMatch(firstFixture.Buffer.data(), secondFixture.Buffer.data(), firstFixture.ByteCount))
    {
        return Fail("deterministic stream bytes did not match");
    }

    SerializeReader reader(firstFixture.Buffer.data(), firstFixture.ByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("reader did not open stream");
    }

    std::array<std::uint8_t, 4U> undersizedBytes{};
    undersizedBytes.fill(SENTINEL_BYTE);
    std::uint32_t rejectedByteCount = 77U;
    if (reader.ReadFixedBytes(
            RECORD_MAIN,
            FIELD_BYTES,
            undersizedBytes.data(),
            static_cast<std::uint32_t>(undersizedBytes.size()),
            rejectedByteCount) != SerializeStatus::BufferTooSmall)
    {
        return Fail(UNDERSIZED_FIXED_BYTES_READ_MESSAGE);
    }

    if (rejectedByteCount != 77U)
    {
        return Fail(UNDERSIZED_FIXED_BYTES_COUNT_MESSAGE);
    }

    if (undersizedBytes[0U] != SENTINEL_BYTE || undersizedBytes[3U] != SENTINEL_BYTE)
    {
        return Fail(UNDERSIZED_FIXED_BYTES_WRITE_MESSAGE);
    }

    std::uint32_t u32Value = 0U;
    std::int32_t i32Value = 0;
    std::uint64_t u64Value = 0U;
    std::int64_t i64Value = 0;
    std::array<std::uint8_t, 5U> bytes{};
    std::uint32_t byteCount = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, u32Value) != SerializeStatus::Success)
    {
        return Fail("read uint32 failed");
    }

    if (reader.ReadInt32(RECORD_MAIN, FIELD_I32, i32Value) != SerializeStatus::Success)
    {
        return Fail("read int32 failed");
    }

    if (reader.ReadUInt64(RECORD_MAIN, FIELD_U64, u64Value) != SerializeStatus::Success)
    {
        return Fail("read uint64 failed");
    }

    if (reader.ReadInt64(RECORD_MAIN, FIELD_I64, i64Value) != SerializeStatus::Success)
    {
        return Fail("read int64 failed");
    }

    if (reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size()), byteCount) !=
        SerializeStatus::Success)
    {
        return Fail("read fixed bytes failed");
    }

    if (u32Value != 0xAABBCCDDU || i32Value != -25 || u64Value != 0x1122334455667788ULL || i64Value != -6000000000LL)
    {
        return Fail("roundtrip primitive values did not match");
    }

    const std::array<std::uint8_t, 5U> expectedBytes{1U, 3U, 5U, 7U, 9U};
    if (byteCount != expectedBytes.size() || !BytesMatch(bytes.data(), expectedBytes.data(), byteCount))
    {
        return Fail("roundtrip fixed bytes did not match");
    }

    return 0;
}

int SerializeStreamHeaderRejectsInvalidMagicOrVersion()
{
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    fixture.Buffer[0U] = 0U;
    SerializeReader invalidMagicReader(fixture.Buffer.data(), fixture.ByteCount);
    if (invalidMagicReader.OpenStream() != SerializeStatus::InvalidHeader)
    {
        return Fail("invalid magic did not return explicit status");
    }

    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    SerializeReader reopenedReader(fixture.Buffer.data(), fixture.ByteCount);
    if (reopenedReader.OpenStream() != SerializeStatus::Success)
    {
        return Fail(REOPEN_VALID_STREAM_MESSAGE);
    }

    fixture.Buffer[0U] = 0U;
    if (reopenedReader.OpenStream() != SerializeStatus::InvalidHeader)
    {
        return Fail(REOPEN_INVALID_MAGIC_MESSAGE);
    }

    std::uint32_t staleValue = 99U;
    if (reopenedReader.ReadUInt32(RECORD_MAIN, FIELD_U32, staleValue) != SerializeStatus::InvalidHeader)
    {
        return Fail(REOPEN_STALE_STATE_MESSAGE);
    }

    if (staleValue != 99U)
    {
        return Fail(REOPEN_OUTPUT_MUTATION_MESSAGE);
    }

    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    WriteUInt16At(fixture.Buffer.data(), STREAM_MAJOR_VERSION_OFFSET, 99U);
    SerializeReader unsupportedVersionReader(fixture.Buffer.data(), fixture.ByteCount);
    if (unsupportedVersionReader.OpenStream() != SerializeStatus::UnsupportedVersion)
    {
        return Fail("unsupported major version did not return explicit status");
    }

    return 0;
}

int SerializeStreamHeaderRejectsReservedFlags()
{
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    WriteUInt32At(fixture.Buffer.data(), STREAM_FLAGS_OFFSET, 1U);
    SerializeReader reader(fixture.Buffer.data(), fixture.ByteCount);
    if (reader.OpenStream() != SerializeStatus::InvalidHeader)
    {
        return Fail("reserved stream flags did not return explicit header status");
    }

    return 0;
}

int SerializeWriterBufferOverflowReturnsStatusWithoutOverrun()
{
    std::array<std::uint8_t, 8U> tinyBuffer{};
    tinyBuffer.fill(SENTINEL_BYTE);
    SerializeWriter tinyWriter(tinyBuffer.data(), static_cast<std::uint32_t>(tinyBuffer.size()));
    if (tinyWriter.BeginStream() != SerializeStatus::BufferTooSmall)
    {
        return Fail("undersized stream header did not fail");
    }

    if (tinyBuffer[0U] != SENTINEL_BYTE || tinyBuffer[7U] != SENTINEL_BYTE)
    {
        return Fail("undersized stream header wrote into caller buffer");
    }

    std::array<std::uint8_t, 39U> fieldBuffer{};
    fieldBuffer.fill(SENTINEL_BYTE);
    SerializeWriter fieldWriter(fieldBuffer.data(), static_cast<std::uint32_t>(fieldBuffer.size()));
    if (fieldWriter.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (fieldWriter.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    const SerializeSnapshot beforeSnapshot = fieldWriter.Snapshot();
    if (fieldWriter.WriteUInt32(FIELD_U32, 10U) != SerializeStatus::BufferTooSmall)
    {
        return Fail("undersized field payload did not fail");
    }

    if (fieldWriter.Snapshot().CommittedByteCount != beforeSnapshot.CommittedByteCount)
    {
        return Fail("undersized field write changed committed bytes");
    }

    if (fieldBuffer[beforeSnapshot.CommittedByteCount] != SENTINEL_BYTE)
    {
        return Fail("undersized field write modified uncommitted bytes");
    }

    return 0;
}

int SerializeRecordCapacityOverflowDoesNotMutate()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    std::uint32_t recordIndex = 0U;
    while (recordIndex < MAX_RECORDS_PER_STREAM)
    {
        if (writer.BeginRecord(SerializeRecordId{recordIndex + 1U}) != SerializeStatus::Success)
        {
            return Fail("record within capacity failed");
        }

        ++recordIndex;
    }

    const SerializeSnapshot beforeSnapshot = writer.Snapshot();
    if (writer.BeginRecord(SerializeRecordId{100U}) != SerializeStatus::RecordCapacityExceeded)
    {
        return Fail("record capacity overflow did not return explicit status");
    }

    const SerializeSnapshot afterSnapshot = writer.Snapshot();
    if (afterSnapshot.RecordCount != beforeSnapshot.RecordCount)
    {
        return Fail("record overflow changed record count");
    }

    if (afterSnapshot.CommittedByteCount != beforeSnapshot.CommittedByteCount)
    {
        return Fail("record overflow changed committed byte count");
    }

    return 0;
}

int SerializeFieldCapacityOverflowDoesNotMutate()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    std::uint32_t fieldIndex = 0U;
    while (fieldIndex < MAX_FIELDS_PER_RECORD)
    {
        if (writer.WriteUInt32(SerializeFieldId{fieldIndex + 1U}, fieldIndex) != SerializeStatus::Success)
        {
            return Fail("field within capacity failed");
        }

        ++fieldIndex;
    }

    const SerializeSnapshot beforeSnapshot = writer.Snapshot();
    if (writer.WriteUInt32(SerializeFieldId{100U}, 100U) != SerializeStatus::FieldCapacityExceeded)
    {
        return Fail("field capacity overflow did not return explicit status");
    }

    const SerializeSnapshot afterSnapshot = writer.Snapshot();
    if (afterSnapshot.FieldCount != beforeSnapshot.FieldCount)
    {
        return Fail("field overflow changed field count");
    }

    if (afterSnapshot.CommittedByteCount != beforeSnapshot.CommittedByteCount)
    {
        return Fail("field overflow changed committed byte count");
    }

    return 0;
}

int SerializeFixedBytesPayloadLimitReturnsExplicitStatus()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    std::array<std::uint8_t, MAX_FIELD_PAYLOAD_BYTE_COUNT + 1U> bytes{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    const SerializeSnapshot beforeSnapshot = writer.Snapshot();
    if (writer.WriteFixedBytes(FIELD_BYTES, bytes.data(), static_cast<std::uint32_t>(bytes.size())) !=
        SerializeStatus::FieldPayloadTooLarge)
    {
        return Fail("oversized fixed bytes did not return explicit status");
    }

    if (writer.Snapshot().CommittedByteCount != beforeSnapshot.CommittedByteCount)
    {
        return Fail("oversized fixed bytes changed committed byte count");
    }

    if (writer.WriteFixedBytes(FIELD_BYTES, nullptr, 0U) != SerializeStatus::Success)
    {
        return Fail(ZERO_FIXED_BYTES_WRITE_MESSAGE);
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().CommittedByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail(ZERO_FIXED_BYTES_OPEN_MESSAGE);
    }

    std::uint32_t byteCount = 7U;
    if (reader.ReadFixedBytes(RECORD_MAIN, FIELD_BYTES, nullptr, 0U, byteCount) != SerializeStatus::Success)
    {
        return Fail(ZERO_FIXED_BYTES_READ_MESSAGE);
    }

    if (byteCount != 0U)
    {
        return Fail(ZERO_FIXED_BYTES_COUNT_MESSAGE);
    }

    return 0;
}

int SerializeReaderRejectsTruncatedStream()
{
    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::TruncatedStream)
    {
        return Fail("truncated stream did not return explicit status");
    }

    return 0;
}

int SerializeReaderRejectsInvalidRecordOrFieldId()
{
    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT + RECORD_HEADER_BYTE_COUNT>
        invalidRecordBuffer{};
    WriteValidHeader(invalidRecordBuffer.data(), 1U);
    WriteRecordHeader(
        invalidRecordBuffer.data(),
        STREAM_HEADER_BYTE_COUNT,
        SerializeRecordId{0U},
        0U);
    SerializeReader invalidRecordReader(
        invalidRecordBuffer.data(),
        static_cast<std::uint32_t>(invalidRecordBuffer.size()));
    if (invalidRecordReader.OpenStream() != SerializeStatus::InvalidHeader)
    {
        return Fail("invalid record id did not return explicit header status");
    }

    std::array<std::uint8_t, 40U> invalidFieldBuffer{};
    WriteValidHeader(invalidFieldBuffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(invalidFieldBuffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(
        invalidFieldBuffer.data(),
        offset,
        SerializeFieldId{0U},
        static_cast<std::uint32_t>(SerializeTypeTag::UInt32),
        UINT32_PAYLOAD_BYTE_COUNT);
    WriteUInt32At(invalidFieldBuffer.data(), offset, 1U);
    SerializeReader invalidFieldReader(invalidFieldBuffer.data(), static_cast<std::uint32_t>(invalidFieldBuffer.size()));
    if (invalidFieldReader.OpenStream() != SerializeStatus::InvalidHeader)
    {
        return Fail("invalid field id did not return explicit header status");
    }

    return 0;
}

int SerializeReaderRejectsMalformedFieldLength()
{
    std::array<std::uint8_t, 44U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, static_cast<std::uint32_t>(SerializeTypeTag::UInt32), 8U);
    WriteUInt32At(buffer.data(), offset, 1U);
    WriteUInt32At(buffer.data(), offset + sizeof(std::uint32_t), 2U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::MalformedFieldLength)
    {
        return Fail("malformed field length did not return explicit status");
    }

    return 0;
}

int SerializeReaderRejectsUnknownTypeTag()
{
    std::array<std::uint8_t, 40U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, 99U, UINT32_PAYLOAD_BYTE_COUNT);
    WriteUInt32At(buffer.data(), offset, 1U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::UnknownTypeTag)
    {
        return Fail("unknown type tag did not return explicit status");
    }

    return 0;
}

int SerializeReaderTypeMismatchReturnsExplicitStatus()
{
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    SerializeReader reader(fixture.Buffer.data(), fixture.ByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("reader open failed");
    }

    std::int32_t value = -333;
    if (reader.ReadInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::TypeMismatch)
    {
        return Fail("reader type mismatch did not return explicit status");
    }

    if (value != -333)
    {
        return Fail(TYPE_MISMATCH_OUTPUT_MESSAGE);
    }

    return 0;
}

int SerializeDuplicateFieldReturnsExplicitStatus()
{
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
    if (reader.OpenStream() != SerializeStatus::DuplicateField)
    {
        return Fail("duplicate field did not return explicit status");
    }

    return 0;
}

int SerializeUnknownFieldWithValidLengthCanSkipDeterministically()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_SECONDARY) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    const std::array<std::uint8_t, 3U> unknownPayload{2U, 4U, 6U};
    if (writer.WriteFixedBytes(FIELD_UNKNOWN, unknownPayload.data(), static_cast<std::uint32_t>(unknownPayload.size())) !=
        SerializeStatus::Success)
    {
        return Fail("write unknown field failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 55U) != SerializeStatus::Success)
    {
        return Fail("write target field failed");
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().CommittedByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_SECONDARY, FIELD_U32, value) != SerializeStatus::Success)
    {
        return Fail("reader did not skip unknown field");
    }

    if (value != 55U)
    {
        return Fail("reader returned unexpected target field value");
    }

    return 0;
}

int SerializeDisabledDiagnosticsDoesNotChangeResults()
{
    StreamFixture firstFixture;
    StreamFixture secondFixture;
    if (BuildRoundTripFixture(firstFixture) != 0)
    {
        return 1;
    }

    if (BuildRoundTripFixture(secondFixture) != 0)
    {
        return 1;
    }

    if (!SnapshotsMatch(firstFixture.Snapshot, secondFixture.Snapshot))
    {
        return Fail("diagnostics-equivalent writer snapshots diverged");
    }

    SerializeReader firstReader(firstFixture.Buffer.data(), firstFixture.ByteCount);
    SerializeReader secondReader(secondFixture.Buffer.data(), secondFixture.ByteCount);
    if (firstReader.OpenStream() != secondReader.OpenStream())
    {
        return Fail("diagnostics-equivalent reader open statuses diverged");
    }

    std::uint32_t firstValue = 0U;
    std::uint32_t secondValue = 0U;
    if (firstReader.ReadUInt32(RECORD_MAIN, FIELD_U32, firstValue) != secondReader.ReadUInt32(RECORD_MAIN, FIELD_U32, secondValue))
    {
        return Fail("diagnostics-equivalent reader statuses diverged");
    }

    if (firstValue != secondValue)
    {
        return Fail("diagnostics-equivalent reader values diverged");
    }

    if (!SnapshotsMatch(firstReader.Snapshot(), secondReader.Snapshot()))
    {
        return Fail("diagnostics-equivalent reader snapshots diverged");
    }

    return 0;
}

int SerializeNoFilePackageResourceObjectOrGameAdapterDependency()
{
    StreamFixture fixture;
    if (BuildRoundTripFixture(fixture) != 0)
    {
        return 1;
    }

    SerializeReader reader(fixture.Buffer.data(), fixture.ByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("synthetic stream reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::Success)
    {
        return Fail("synthetic stream read failed");
    }

    if (value != 0xAABBCCDDU)
    {
        return Fail("synthetic stream value changed");
    }

    return 0;
}

int SerializeNoHiddenAllocationInReadWritePath()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.Snapshot().AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("writer did not expose YuMemory accounting vocabulary");
    }

    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 3U) != SerializeStatus::Success)
    {
        return Fail("write uint32 failed");
    }

    const SerializeSnapshot writerSnapshot = writer.Snapshot();
    SerializeReader reader(buffer.data(), writerSnapshot.CommittedByteCount);
    if (reader.Snapshot().AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("reader did not expose YuMemory accounting vocabulary");
    }

    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("reader open failed");
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::Success)
    {
        return Fail("reader read failed");
    }

    if (writer.Snapshot().CommittedByteCount != writerSnapshot.CommittedByteCount)
    {
        return Fail("read path changed writer committed bytes");
    }

    return 0;
}

int SerializeSnapshotReportsCountsAndLastStatus()
{
    std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    if (writer.BeginRecord(RECORD_MAIN) != SerializeStatus::Success)
    {
        return Fail("begin record failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 1U) != SerializeStatus::Success)
    {
        return Fail("write first field failed");
    }

    if (writer.WriteInt32(FIELD_I32, -1) != SerializeStatus::Success)
    {
        return Fail("write second field failed");
    }

    if (writer.WriteUInt32(FIELD_U32, 2U) != SerializeStatus::DuplicateField)
    {
        return Fail("duplicate field did not fail");
    }

    const SerializeSnapshot writerSnapshot = writer.Snapshot();
    if (writerSnapshot.RecordCount != 1U || writerSnapshot.FieldCount != 2U)
    {
        return Fail("writer snapshot did not report record and field counts");
    }

    if (writerSnapshot.AcceptedOperationCount != 4U || writerSnapshot.FailedOperationCount != 1U)
    {
        return Fail("writer snapshot did not report operation counts");
    }

    if (writerSnapshot.LastStatus != SerializeStatus::DuplicateField)
    {
        return Fail("writer snapshot did not report last status");
    }

    SerializeReader reader(buffer.data(), writerSnapshot.CommittedByteCount);
    if (reader.OpenStream() != SerializeStatus::Success)
    {
        return Fail("reader open failed");
    }

    const SerializeSnapshot readerSnapshot = reader.Snapshot();
    if (readerSnapshot.RecordCount != 1U || readerSnapshot.FieldCount != 2U)
    {
        return Fail("reader snapshot did not report record and field counts");
    }

    if (readerSnapshot.CommittedByteCount != writerSnapshot.CommittedByteCount)
    {
        return Fail("reader snapshot did not report committed byte count");
    }

    if (readerSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("reader snapshot did not report YuMemory accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    static const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_ROUND_TRIP, SerializeWriteReadPrimitivesRoundTripsDeterministically},
        {TEST_HEADER, SerializeStreamHeaderRejectsInvalidMagicOrVersion},
        {TEST_RESERVED_FLAGS, SerializeStreamHeaderRejectsReservedFlags},
        {TEST_WRITER_OVERFLOW, SerializeWriterBufferOverflowReturnsStatusWithoutOverrun},
        {TEST_RECORD_CAPACITY, SerializeRecordCapacityOverflowDoesNotMutate},
        {TEST_FIELD_CAPACITY, SerializeFieldCapacityOverflowDoesNotMutate},
        {TEST_FIXED_BYTES_LIMIT, SerializeFixedBytesPayloadLimitReturnsExplicitStatus},
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
        {TEST_SNAPSHOT, SerializeSnapshotReportsCountsAndLastStatus}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end())
    {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
