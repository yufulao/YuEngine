// 模块：YuEngine AudioResource
// 文件：Tests/AudioResource/AudioResourceTests.cpp

#include <array>
#include <cstdio>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportBridge.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportBridgeDesc.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportConstants.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportOperation.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRequest.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportSnapshot.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultRecord.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

using yuengine::audio::AudioPcmSamplePacketRequest;
using yuengine::audio::AudioSampleFormat;
using yuengine::audioresource::AudioResourcePcmPacketImportBridge;
using yuengine::audioresource::AudioResourcePcmPacketImportBridgeDesc;
using yuengine::audioresource::AudioResourcePcmPacketImportHandle;
using yuengine::audioresource::AudioResourcePcmPacketImportOperation;
using yuengine::audioresource::AudioResourcePcmPacketImportRecord;
using yuengine::audioresource::AudioResourcePcmPacketImportRequest;
using yuengine::audioresource::AudioResourcePcmPacketImportSnapshot;
using yuengine::audioresource::AudioResourcePcmPacketImportStatus;
using yuengine::audioresource::MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodeResultOperation;
using yuengine::resource::ResourceDecodeResultRecord;
using yuengine::resource::ResourceDecodeResultStatus;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceTypeId;

namespace {
constexpr const char *TEST_MAPS_RESOURCE_DECODE_RESULT = "AudioResource_PcmPacketImportBridge_MapsResourceDecodeResult";
constexpr const char *TEST_REJECTS_NON_AUDIO_DECODE_RESULT = "AudioResource_PcmPacketImportBridge_RejectsNonAudioDecodeResult";
constexpr const char *TEST_REJECTS_DECODED_BYTE_MISMATCH = "AudioResource_PcmPacketImportBridge_RejectsDecodedByteMismatch";
constexpr const char *TEST_REJECTS_INVALID_PACKET_SHAPE = "AudioResource_PcmPacketImportBridge_RejectsInvalidPacketShape";
constexpr const char *TEST_REJECTS_DUPLICATE_IMPORT_AND_PACKET_IDS = "AudioResource_PcmPacketImportBridge_RejectsDuplicateImportAndPacketIds";
constexpr const char *TEST_REJECTS_CAPACITY_OVERFLOW = "AudioResource_PcmPacketImportBridge_RejectsCapacityOverflow";
constexpr const char *TEST_CAPACITY_IDENTITY_CLEARS_AFTER_NON_CAPACITY_RESULTS = "AudioResource_PcmPacketImportBridge_CapacityIdentityClearsAfterNonCapacityResults";
constexpr const char *TEST_QUERY_AND_RELEASE_INVALIDATE_HANDLE = "AudioResource_PcmPacketImportBridge_QueryAndReleaseInvalidateHandle";
constexpr const char *TEST_PUBLIC_CONTRACTS_ARE_PLAIN_VALUES = "AudioResource_PcmPacketImportBridge_PublicContractsArePlainValues";

constexpr ResourceTypeId TYPE_AUDIO{29U};
constexpr ResourceTypeId TYPE_TEXTURE{30U};
constexpr ResourceHandle AUDIO_RESOURCE{2U, 3U};
constexpr std::uint64_t PAYLOAD_ID = 3001U;
constexpr std::uint64_t DECODE_PLAN_ID = 4001U;
constexpr std::uint64_t DECODE_RESULT_ID = 5001U;
constexpr std::uint64_t IMPORT_ID = 6001U;
constexpr std::uint32_t PACKET_ID = 7001U;
constexpr std::size_t FRAME_COUNT = 4U;
constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * yuengine::audio::CHANNEL_COUNT;
constexpr std::size_t BYTE_COUNT = SAMPLE_COUNT * sizeof(std::int16_t);
using TestFunction = int (*)();

struct TestCase final {
    const char *name = nullptr;
    TestFunction function = nullptr;
};

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

AudioPcmSamplePacketRequest PacketRequest(std::uint32_t packet_id=PACKET_ID) {
    AudioPcmSamplePacketRequest request;
    request.packet_id = packet_id;
    request.format = AudioSampleFormat::Signed16;
    request.sample_rate = yuengine::audio::SAMPLE_RATE;
    request.channel_count = yuengine::audio::CHANNEL_COUNT;
    request.frame_count = FRAME_COUNT;
    request.interleaved_sample_count = SAMPLE_COUNT;
    request.byte_count = BYTE_COUNT;
    return request;
}

ResourceDecodeResultRecord DecodeResultRecord(
    ResourceDecodePlanAssetClass asset_class=ResourceDecodePlanAssetClass::Audio,
    ResourceDecodeResultClass result_class=ResourceDecodeResultClass::Audio,
    std::uint32_t decoded_byte_count=static_cast<std::uint32_t>(BYTE_COUNT)) {
    ResourceDecodeResultRecord record;
    record.operation = ResourceDecodeResultOperation::Commit;
    record.resource = AUDIO_RESOURCE;
    record.expected_type = TYPE_AUDIO;
    record.payload_id = PAYLOAD_ID;
    record.decode_plan_id = DECODE_PLAN_ID;
    record.decode_result_id = DECODE_RESULT_ID;
    record.asset_class = asset_class;
    record.result_class = result_class;
    record.decoded_byte_count = decoded_byte_count;
    record.status = ResourceDecodeResultStatus::Success;
    record.is_active = true;
    return record;
}

AudioResourcePcmPacketImportRequest ImportRequest(
    std::uint64_t import_id=IMPORT_ID,
    std::uint32_t packet_id=PACKET_ID) {
    AudioResourcePcmPacketImportRequest request;
    request.import_id = import_id;
    request.resource = AUDIO_RESOURCE;
    request.expected_type = TYPE_AUDIO;
    request.payload_id = PAYLOAD_ID;
    request.decode_plan_id = DECODE_PLAN_ID;
    request.decode_result_id = DECODE_RESULT_ID;
    request.packet_request = PacketRequest(packet_id);
    return request;
}

bool ExpectRejectedWithoutActiveMutation(
    AudioResourcePcmPacketImportBridge &bridge,
    const ResourceDecodeResultRecord &record,
    const AudioResourcePcmPacketImportRequest &request,
    AudioResourcePcmPacketImportStatus expected_status) {
    const AudioResourcePcmPacketImportSnapshot before_snapshot = bridge.Snapshot();
    AudioResourcePcmPacketImportHandle handle;
    AudioPcmSamplePacketRequest packet_request;
    const AudioResourcePcmPacketImportStatus status = bridge.ImportPcmPacket(
        record,
        request,
        &handle,
        &packet_request);
    if (status != expected_status) {
        return false;
    }

    const AudioResourcePcmPacketImportSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_import_count != before_snapshot.active_import_count) {
        return false;
    }

    if (after_snapshot.imported_packet_count != before_snapshot.imported_packet_count) {
        return false;
    }

    if (after_snapshot.rejected_import_count != before_snapshot.rejected_import_count + 1U) {
        return false;
    }

    return true;
}

bool ImportBasicMapping(
    AudioResourcePcmPacketImportBridge &bridge,
    AudioResourcePcmPacketImportHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    AudioPcmSamplePacketRequest packet_request;
    const AudioResourcePcmPacketImportStatus status = bridge.ImportPcmPacket(
        DecodeResultRecord(),
        ImportRequest(),
        out_handle,
        &packet_request);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return false;
    }

    return packet_request.packet_id == PACKET_ID;
}

bool CapacityFailureIdentityMatches(
    const AudioResourcePcmPacketImportSnapshot &snapshot,
    const AudioResourcePcmPacketImportRequest &request) {
    if (snapshot.last_failed_import_id != request.import_id) {
        return false;
    }

    if (snapshot.last_failed_decode_result_id != request.decode_result_id) {
        return false;
    }

    if (snapshot.last_failed_packet_id != request.packet_request.packet_id) {
        return false;
    }

    if (snapshot.last_failed_resource_slot != request.resource.slot) {
        return false;
    }

    return snapshot.last_failed_resource_generation == request.resource.generation;
}

bool CapacityFailureIdentityIsClear(const AudioResourcePcmPacketImportSnapshot &snapshot) {
    if (snapshot.last_failed_import_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_decode_result_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_packet_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_resource_slot != 0U) {
        return false;
    }

    return snapshot.last_failed_resource_generation == 0U;
}

bool FillImportCapacity(
    AudioResourcePcmPacketImportBridge &bridge,
    AudioResourcePcmPacketImportHandle *first_handle) {
    if (first_handle == nullptr) {
        return false;
    }

    std::uint32_t index = 0U;
    while (index < MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS) {
        AudioResourcePcmPacketImportHandle handle{};
        AudioPcmSamplePacketRequest packet_request{};
        AudioResourcePcmPacketImportRequest request = ImportRequest(
            IMPORT_ID + index,
            PACKET_ID + index);
        const AudioResourcePcmPacketImportStatus status = bridge.ImportPcmPacket(
            DecodeResultRecord(),
            request,
            &handle,
            &packet_request);
        if (status != AudioResourcePcmPacketImportStatus::Success) {
            return false;
        }

        if (index == 0U) {
            *first_handle = handle;
        }

        ++index;
    }

    return true;
}

bool RejectCapacityOverflow(
    AudioResourcePcmPacketImportBridge &bridge,
    const AudioResourcePcmPacketImportRequest &request) {
    return ExpectRejectedWithoutActiveMutation(
        bridge,
        DecodeResultRecord(),
        request,
        AudioResourcePcmPacketImportStatus::CapacityExceeded);
}

int AudioResourcePcmPacketImportBridgeMapsResourceDecodeResult() {
    AudioResourcePcmPacketImportBridge bridge;
    AudioResourcePcmPacketImportHandle handle;
    AudioPcmSamplePacketRequest packet_request;
    const AudioResourcePcmPacketImportStatus status = bridge.ImportPcmPacket(
        DecodeResultRecord(),
        ImportRequest(),
        &handle,
        &packet_request);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return Fail("pcm packet import did not accept active audio decode result");
    }

    if (!handle.IsValid()) {
        return Fail("pcm packet import did not write valid handle");
    }

    if (packet_request.packet_id != PACKET_ID) {
        return Fail("pcm packet import wrote wrong packet id");
    }

    if (packet_request.frame_count != FRAME_COUNT) {
        return Fail("pcm packet import wrote wrong frame count");
    }

    if (packet_request.interleaved_sample_count != SAMPLE_COUNT) {
        return Fail("pcm packet import wrote wrong sample count");
    }

    if (packet_request.byte_count != BYTE_COUNT) {
        return Fail("pcm packet import wrote wrong byte count");
    }

    const AudioResourcePcmPacketImportSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_import_count != 1U || snapshot.imported_packet_count != 1U) {
        return Fail("pcm packet import did not track accepted count");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeRejectsNonAudioDecodeResult() {
    AudioResourcePcmPacketImportBridge bridge;
    ResourceDecodeResultRecord texture_record = DecodeResultRecord(
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture);
    const AudioResourcePcmPacketImportRequest request = ImportRequest();
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            texture_record,
            request,
            AudioResourcePcmPacketImportStatus::AssetClassMismatch)) {
        return Fail("non audio decode result was not rejected without mutation");
    }

    ResourceDecodeResultRecord result_mismatch = DecodeResultRecord(
        ResourceDecodePlanAssetClass::Audio,
        ResourceDecodeResultClass::Texture);
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            result_mismatch,
            request,
            AudioResourcePcmPacketImportStatus::ResultClassMismatch)) {
        return Fail("non audio result class was not rejected without mutation");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeRejectsDecodedByteMismatch() {
    AudioResourcePcmPacketImportBridge bridge;
    ResourceDecodeResultRecord record = DecodeResultRecord();
    record.decoded_byte_count = static_cast<std::uint32_t>(BYTE_COUNT - 1U);
    const AudioResourcePcmPacketImportRequest request = ImportRequest();
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::DecodeResultByteCountMismatch)) {
        return Fail("decoded byte mismatch was not rejected without mutation");
    }

    record = DecodeResultRecord();
    record.decoded_byte_count = 0U;
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::DecodeResultByteCountMismatch)) {
        return Fail("zero decoded byte count was not rejected without mutation");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeRejectsInvalidPacketShape() {
    AudioResourcePcmPacketImportBridge bridge;
    const ResourceDecodeResultRecord record = DecodeResultRecord();
    AudioResourcePcmPacketImportRequest request = ImportRequest();
    request.packet_request.frame_count = 0U;
    request.packet_request.interleaved_sample_count = 0U;
    request.packet_request.byte_count = 0U;
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::InvalidFrameCount)) {
        return Fail("invalid frame count was not rejected");
    }

    request = ImportRequest();
    request.packet_request.interleaved_sample_count -= 1U;
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::SampleCountMismatch)) {
        return Fail("sample count mismatch was not rejected");
    }

    request = ImportRequest();
    request.packet_request.sample_rate = yuengine::audio::SAMPLE_RATE - 1U;
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::UnsupportedSampleRate)) {
        return Fail("sample rate mismatch was not rejected");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeRejectsDuplicateImportAndPacketIds() {
    AudioResourcePcmPacketImportBridge bridge;
    AudioResourcePcmPacketImportHandle handle;
    if (!ImportBasicMapping(bridge, &handle)) {
        return Fail("duplicate baseline import failed");
    }

    ResourceDecodeResultRecord record = DecodeResultRecord();
    AudioResourcePcmPacketImportRequest request = ImportRequest(IMPORT_ID, PACKET_ID + 1U);
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::DuplicateImportId)) {
        return Fail("duplicate import id was not rejected");
    }

    request = ImportRequest(IMPORT_ID + 1U, PACKET_ID);
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            record,
            request,
            AudioResourcePcmPacketImportStatus::DuplicatePacketId)) {
        return Fail("duplicate packet id was not rejected");
    }

    const AudioResourcePcmPacketImportSnapshot snapshot = bridge.Snapshot();
    if (snapshot.duplicate_import_rejected_count != 1U || snapshot.duplicate_packet_rejected_count != 1U) {
        return Fail("duplicate rejection counters were unexpected");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeRejectsCapacityOverflow() {
    AudioResourcePcmPacketImportBridge bridge;
    AudioResourcePcmPacketImportHandle first_handle{};
    if (!FillImportCapacity(bridge, &first_handle)) {
        return Fail("capacity setup import failed");
    }

    AudioResourcePcmPacketImportRequest request = ImportRequest(IMPORT_ID + 100U, PACKET_ID + 100U);
    if (!RejectCapacityOverflow(bridge, request)) {
        return Fail("capacity overflow was not rejected");
    }

    const AudioResourcePcmPacketImportSnapshot snapshot = bridge.Snapshot();
    if (snapshot.capacity_rejected_count != 1U) {
        return Fail("capacity rejection counter was unexpected");
    }

    constexpr std::uint32_t EXPECTED_REQUIRED_IMPORT_COUNT = MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS + 1U;
    if (snapshot.last_required_import_count != EXPECTED_REQUIRED_IMPORT_COUNT) {
        return Fail("capacity required import count was unexpected");
    }

    if (!CapacityFailureIdentityMatches(snapshot, request)) {
        return Fail("capacity failed import identity was unexpected");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeCapacityIdentityClearsAfterNonCapacityResults() {
    AudioResourcePcmPacketImportBridge bridge;
    AudioResourcePcmPacketImportHandle first_handle{};
    if (!FillImportCapacity(bridge, &first_handle)) {
        return Fail("capacity identity setup import failed");
    }

    const AudioResourcePcmPacketImportRequest capacity_request = ImportRequest(
        IMPORT_ID + 100U,
        PACKET_ID + 100U);
    if (!RejectCapacityOverflow(bridge, capacity_request)) {
        return Fail("capacity identity baseline reject failed");
    }

    AudioResourcePcmPacketImportRecord record{};
    if (bridge.QueryPcmPacketImport(first_handle, &record) != AudioResourcePcmPacketImportStatus::Success) {
        return Fail("query after capacity reject failed");
    }

    AudioResourcePcmPacketImportSnapshot snapshot = bridge.Snapshot();
    if (!CapacityFailureIdentityIsClear(snapshot)) {
        return Fail("query success did not clear capacity identity");
    }

    if (!RejectCapacityOverflow(bridge, capacity_request)) {
        return Fail("second capacity identity reject failed");
    }

    AudioResourcePcmPacketImportRequest duplicate_request = ImportRequest(IMPORT_ID, PACKET_ID + 200U);
    if (!ExpectRejectedWithoutActiveMutation(
            bridge,
            DecodeResultRecord(),
            duplicate_request,
            AudioResourcePcmPacketImportStatus::DuplicateImportId)) {
        return Fail("duplicate import after capacity reject failed");
    }

    snapshot = bridge.Snapshot();
    if (!CapacityFailureIdentityIsClear(snapshot)) {
        return Fail("duplicate import did not clear capacity identity");
    }

    if (!RejectCapacityOverflow(bridge, capacity_request)) {
        return Fail("third capacity identity reject failed");
    }

    AudioResourcePcmPacketImportHandle invalid_handle{};
    if (bridge.QueryPcmPacketImport(invalid_handle, &record) != AudioResourcePcmPacketImportStatus::InvalidHandle) {
        return Fail("invalid handle after capacity reject failed");
    }

    snapshot = bridge.Snapshot();
    if (!CapacityFailureIdentityIsClear(snapshot)) {
        return Fail("handle failure did not clear capacity identity");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgeQueryAndReleaseInvalidateHandle() {
    AudioResourcePcmPacketImportBridge bridge;
    AudioResourcePcmPacketImportHandle handle;
    if (!ImportBasicMapping(bridge, &handle)) {
        return Fail("query release baseline import failed");
    }

    AudioResourcePcmPacketImportRecord record;
    if (bridge.QueryPcmPacketImport(handle, &record) != AudioResourcePcmPacketImportStatus::Success) {
        return Fail("query import record failed");
    }

    if (record.import_id != IMPORT_ID || record.packet_request.packet_id != PACKET_ID) {
        return Fail("query returned wrong import record");
    }

    if (bridge.ReleasePcmPacketImport(handle) != AudioResourcePcmPacketImportStatus::Success) {
        return Fail("release import record failed");
    }

    if (bridge.QueryPcmPacketImport(handle, &record) != AudioResourcePcmPacketImportStatus::GenerationMismatch) {
        return Fail("released import query did not reject stale handle");
    }

    if (bridge.ReleasePcmPacketImport(handle) != AudioResourcePcmPacketImportStatus::GenerationMismatch) {
        return Fail("released import release did not reject stale handle");
    }

    const AudioResourcePcmPacketImportSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_import_count != 0U || snapshot.queried_import_count != 1U || snapshot.released_import_count != 1U) {
        return Fail("query release counters were unexpected");
    }

    if (snapshot.stale_import_rejected_count != 2U) {
        return Fail("stale import rejection counter was unexpected");
    }

    return 0;
}

int AudioResourcePcmPacketImportBridgePublicContractsArePlainValues() {
    if (!std::is_standard_layout_v<AudioResourcePcmPacketImportHandle>) {
        return Fail("pcm packet import handle must be standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioResourcePcmPacketImportHandle>) {
        return Fail("pcm packet import handle must be trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioResourcePcmPacketImportRequest>) {
        return Fail("pcm packet import request must be standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioResourcePcmPacketImportRequest>) {
        return Fail("pcm packet import request must be trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioResourcePcmPacketImportRecord>) {
        return Fail("pcm packet import record must be standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioResourcePcmPacketImportRecord>) {
        return Fail("pcm packet import record must be trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioResourcePcmPacketImportSnapshot>) {
        return Fail("pcm packet import snapshot must be standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioResourcePcmPacketImportSnapshot>) {
        return Fail("pcm packet import snapshot must be trivially copyable");
    }

    if (!std::is_enum_v<AudioResourcePcmPacketImportStatus>) {
        return Fail("pcm packet import status must be enum");
    }

    if (!std::is_enum_v<AudioResourcePcmPacketImportOperation>) {
        return Fail("pcm packet import operation must be enum");
    }

    AudioResourcePcmPacketImportBridgeDesc desc;
    if (desc.import_capacity != MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS) {
        return Fail("pcm packet import desc default was unexpected");
    }

    return 0;
}

constexpr std::array<TestCase, 9U> TESTS = {{
    {TEST_MAPS_RESOURCE_DECODE_RESULT, AudioResourcePcmPacketImportBridgeMapsResourceDecodeResult},
    {TEST_REJECTS_NON_AUDIO_DECODE_RESULT, AudioResourcePcmPacketImportBridgeRejectsNonAudioDecodeResult},
    {TEST_REJECTS_DECODED_BYTE_MISMATCH, AudioResourcePcmPacketImportBridgeRejectsDecodedByteMismatch},
    {TEST_REJECTS_INVALID_PACKET_SHAPE, AudioResourcePcmPacketImportBridgeRejectsInvalidPacketShape},
    {TEST_REJECTS_DUPLICATE_IMPORT_AND_PACKET_IDS, AudioResourcePcmPacketImportBridgeRejectsDuplicateImportAndPacketIds},
    {TEST_REJECTS_CAPACITY_OVERFLOW, AudioResourcePcmPacketImportBridgeRejectsCapacityOverflow},
    {TEST_CAPACITY_IDENTITY_CLEARS_AFTER_NON_CAPACITY_RESULTS, AudioResourcePcmPacketImportBridgeCapacityIdentityClearsAfterNonCapacityResults},
    {TEST_QUERY_AND_RELEASE_INVALIDATE_HANDLE, AudioResourcePcmPacketImportBridgeQueryAndReleaseInvalidateHandle},
    {TEST_PUBLIC_CONTRACTS_ARE_PLAIN_VALUES, AudioResourcePcmPacketImportBridgePublicContractsArePlainValues}
}};
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected exactly one test name");
    }

    const std::string_view requested_test = argv[1];
    for (const TestCase &test : TESTS) {
        if (requested_test != test.name) {
            continue;
        }

        return test.function();
    }

    return Fail("unknown test name");
}
