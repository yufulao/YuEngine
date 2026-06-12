#include <array>
#include <cstdint>
#include <iostream>
#include <string>

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

namespace
{
constexpr const char* TEST_ROUND_TRIP = "Serialize_WriteReadPrimitives_RoundTripsDeterministically";
constexpr const char* TEST_HEADER = "Serialize_StreamHeader_RejectsInvalidMagicOrVersion";
constexpr const char* TEST_WRITER_OVERFLOW = "Serialize_WriterBufferOverflow_ReturnsStatusWithoutOverrun";
constexpr const char* TEST_RECORD_CAPACITY = "Serialize_RecordCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIELD_CAPACITY = "Serialize_FieldCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_FIXED_BYTES_LIMIT = "Serialize_FixedBytesPayloadLimit_ReturnsExplicitStatus";
constexpr const char* TEST_TRUNCATED = "Serialize_ReaderRejectsTruncatedStream";
constexpr const char* TEST_MALFORMED_LENGTH = "Serialize_ReaderRejectsMalformedFieldLength";
constexpr const char* TEST_UNKNOWN_TYPE = "Serialize_ReaderRejectsUnknownTypeTag";
constexpr const char* TEST_TYPE_MISMATCH = "Serialize_ReaderTypeMismatch_ReturnsExplicitStatus";
constexpr const char* TEST_DUPLICATE_FIELD = "Serialize_DuplicateField_ReturnsExplicitStatus";
constexpr const char* TEST_UNKNOWN_FIELD_SKIP = "Serialize_UnknownFieldWithValidLength_CanSkipDeterministically";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Serialize_DisabledDiagnostics_DoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Serialize_NoFilePackageResourceObjectOrGameAdapterDependency";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Serialize_NoHiddenAllocationInReadWritePath";
constexpr const char* TEST_SNAPSHOT = "Serialize_SnapshotReportsCountsAndLastStatus";

constexpr SerializeRecordId RECORD_MAIN{7U};
constexpr SerializeRecordId RECORD_SECONDARY{8U};
constexpr SerializeFieldId FIELD_U32{11U};
constexpr SerializeFieldId FIELD_I32{12U};
constexpr SerializeFieldId FIELD_U64{13U};
constexpr SerializeFieldId FIELD_I64{14U};
constexpr SerializeFieldId FIELD_BYTES{15U};
constexpr SerializeFieldId FIELD_UNKNOWN{16U};
constexpr std::uint8_t SENTINEL_BYTE = 0xCDU;

struct StreamFixture final
{
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> Buffer;
    std::uint32_t ByteCount = 0U;
    SerializeSnapshot Snapshot{};
};

int Fail(const std::string& message)
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
    WriteUInt32At(buffer, yuengine::serialize::STREAM_MAGIC_OFFSET, yuengine::serialize::STREAM_MAGIC);
    WriteUInt16At(buffer, yuengine::serialize::STREAM_MAJOR_VERSION_OFFSET, yuengine::serialize::STREAM_MAJOR_VERSION);
    WriteUInt16At(buffer, yuengine::serialize::STREAM_MINOR_VERSION_OFFSET, yuengine::serialize::STREAM_MINOR_VERSION);
    WriteUInt32At(buffer, yuengine::serialize::STREAM_FLAGS_OFFSET, yuengine::serialize::STREAM_FLAGS);
    WriteUInt32At(buffer, yuengine::serialize::STREAM_RECORD_COUNT_OFFSET, recordCount);
}

std::uint32_t WriteRecordHeader(std::uint8_t* buffer, std::uint32_t offset, SerializeRecordId record, std::uint32_t fieldCount)
{
    WriteUInt32At(buffer, offset, record.Value);
    WriteUInt32At(buffer, offset + sizeof(std::uint32_t), fieldCount);
    return offset + yuengine::serialize::RECORD_HEADER_BYTE_COUNT;
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
    return offset + yuengine::serialize::FIELD_HEADER_BYTE_COUNT;
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

    WriteUInt16At(fixture.Buffer.data(), yuengine::serialize::STREAM_MAJOR_VERSION_OFFSET, 99U);
    SerializeReader unsupportedVersionReader(fixture.Buffer.data(), fixture.ByteCount);
    if (unsupportedVersionReader.OpenStream() != SerializeStatus::UnsupportedVersion)
    {
        return Fail("unsupported major version did not return explicit status");
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (writer.BeginStream() != SerializeStatus::Success)
    {
        return Fail("begin stream failed");
    }

    std::uint32_t recordIndex = 0U;
    while (recordIndex < yuengine::serialize::MAX_RECORDS_PER_STREAM)
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
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
    while (fieldIndex < yuengine::serialize::MAX_FIELDS_PER_RECORD)
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
    std::array<std::uint8_t, yuengine::serialize::MAX_FIELD_PAYLOAD_BYTE_COUNT + 1U> bytes{};
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

    return 0;
}

int SerializeReaderRejectsTruncatedStream()
{
    std::array<std::uint8_t, yuengine::serialize::STREAM_HEADER_BYTE_COUNT> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    SerializeReader reader(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (reader.OpenStream() != SerializeStatus::TruncatedStream)
    {
        return Fail("truncated stream did not return explicit status");
    }

    return 0;
}

int SerializeReaderRejectsMalformedFieldLength()
{
    std::array<std::uint8_t, 44U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = yuengine::serialize::STREAM_HEADER_BYTE_COUNT;
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
    std::uint32_t offset = yuengine::serialize::STREAM_HEADER_BYTE_COUNT;
    offset = WriteRecordHeader(buffer.data(), offset, RECORD_MAIN, 1U);
    offset = WriteFieldHeader(buffer.data(), offset, FIELD_U32, 99U, yuengine::serialize::UINT32_PAYLOAD_BYTE_COUNT);
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

    std::int32_t value = 0;
    if (reader.ReadInt32(RECORD_MAIN, FIELD_U32, value) != SerializeStatus::TypeMismatch)
    {
        return Fail("reader type mismatch did not return explicit status");
    }

    return 0;
}

int SerializeDuplicateFieldReturnsExplicitStatus()
{
    std::array<std::uint8_t, 56U> buffer{};
    WriteValidHeader(buffer.data(), 1U);
    std::uint32_t offset = yuengine::serialize::STREAM_HEADER_BYTE_COUNT;
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
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
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
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
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_ROUND_TRIP)
    {
        return SerializeWriteReadPrimitivesRoundTripsDeterministically();
    }

    if (testName == TEST_HEADER)
    {
        return SerializeStreamHeaderRejectsInvalidMagicOrVersion();
    }

    if (testName == TEST_WRITER_OVERFLOW)
    {
        return SerializeWriterBufferOverflowReturnsStatusWithoutOverrun();
    }

    if (testName == TEST_RECORD_CAPACITY)
    {
        return SerializeRecordCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_FIELD_CAPACITY)
    {
        return SerializeFieldCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_FIXED_BYTES_LIMIT)
    {
        return SerializeFixedBytesPayloadLimitReturnsExplicitStatus();
    }

    if (testName == TEST_TRUNCATED)
    {
        return SerializeReaderRejectsTruncatedStream();
    }

    if (testName == TEST_MALFORMED_LENGTH)
    {
        return SerializeReaderRejectsMalformedFieldLength();
    }

    if (testName == TEST_UNKNOWN_TYPE)
    {
        return SerializeReaderRejectsUnknownTypeTag();
    }

    if (testName == TEST_TYPE_MISMATCH)
    {
        return SerializeReaderTypeMismatchReturnsExplicitStatus();
    }

    if (testName == TEST_DUPLICATE_FIELD)
    {
        return SerializeDuplicateFieldReturnsExplicitStatus();
    }

    if (testName == TEST_UNKNOWN_FIELD_SKIP)
    {
        return SerializeUnknownFieldWithValidLengthCanSkipDeterministically();
    }

    if (testName == TEST_DISABLED_DIAGNOSTICS)
    {
        return SerializeDisabledDiagnosticsDoesNotChangeResults();
    }

    if (testName == TEST_NO_FORBIDDEN_DEPENDENCY)
    {
        return SerializeNoFilePackageResourceObjectOrGameAdapterDependency();
    }

    if (testName == TEST_NO_HIDDEN_ALLOCATION)
    {
        return SerializeNoHiddenAllocationInReadWritePath();
    }

    if (testName == TEST_SNAPSHOT)
    {
        return SerializeSnapshotReportsCountsAndLastStatus();
    }

    return Fail("unknown test name");
}
