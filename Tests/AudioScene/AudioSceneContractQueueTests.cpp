// 模块: Tests AudioScene
// 文件: Tests/AudioScene/AudioSceneContractQueueTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Audio/AudioDeviceDesc.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Audio/AudioStatus.h"
#include "YuEngine/Audio/TestAudioDevice.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"
#include "YuEngine/AudioScene/AudioSceneConstants.h"
#include "YuEngine/AudioScene/AudioSceneContractQueue.h"
#include "YuEngine/AudioScene/AudioSceneSourceRecord.h"
#include "YuEngine/AudioScene/AudioSceneSourceState.h"
#include "YuEngine/AudioScene/AudioSceneStatus.h"
#include "YuEngine/AudioScene/AudioSceneSubmitRequest.h"
#include "YuEngine/AudioScene/AudioSceneSubmitResult.h"

using yuengine::asset::AssetHandle;
using yuengine::audio::AudioDeviceDesc;
using yuengine::audio::AudioPcmSamplePacketHandle;
using yuengine::audio::AudioPcmSamplePacketRequest;
using yuengine::audio::AudioPcmStreamQueueHandle;
using yuengine::audio::AudioPcmStreamQueueRequest;
using yuengine::audio::AudioSampleFormat;
using yuengine::audio::AudioStatus;
using yuengine::audio::TestAudioDevice;
using yuengine::audioresource::AudioResourcePcmPacketImportHandle;
using yuengine::audioscene::AUDIO_SCENE_BUS_QUEUE_ID_STRIDE;
using yuengine::audioscene::AUDIO_SCENE_EFFECTS_BUS_ID;
using yuengine::audioscene::AUDIO_SCENE_MASTER_BUS_ID;
using yuengine::audioscene::AudioSceneContractQueue;
using yuengine::audioscene::AudioSceneSourceRecord;
using yuengine::audioscene::AudioSceneSourceState;
using yuengine::audioscene::AudioSceneStatus;
using yuengine::audioscene::AudioSceneSubmitRequest;
using yuengine::audioscene::AudioSceneSubmitResult;

namespace {
constexpr const char *TEST_PLAYING_SOURCE = "AudioScene_PlayingSourceBuildsPcmQueueRequest";
constexpr const char *TEST_BUS_ROUTING = "AudioScene_BusRoutingMapsFixedBusIdsToQueueIds";
constexpr const char *TEST_MISSING_BACKEND = "AudioScene_MissingBackendReturnsExplicitStatus";
constexpr const char *TEST_PAUSED_SOURCE = "AudioScene_PausedSourceDoesNotSubmitQueue";
constexpr const char *TEST_MISSING_SOUND = "AudioScene_MissingSoundAssetDoesNotMutateOutput";
constexpr const char *TEST_INVALID_BUS = "AudioScene_InvalidBusDoesNotMutateOutput";
constexpr const char *TEST_BOUNDARY = "AudioScene_NoNativeOrUpperDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FRAME_ID = 1201U;
constexpr std::uint32_t PACKET_ID = 2201U;
constexpr std::size_t FRAME_COUNT = 8U;
constexpr std::size_t CHANNEL_COUNT = 2U;
constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
constexpr std::size_t BYTE_COUNT = SAMPLE_COUNT * sizeof(std::int16_t);

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

AudioPcmSamplePacketRequest PacketRequest() {
    AudioPcmSamplePacketRequest request{};
    request.packet_id = PACKET_ID;
    request.format = AudioSampleFormat::Signed16;
    request.frame_count = FRAME_COUNT;
    request.interleaved_sample_count = SAMPLE_COUNT;
    request.byte_count = BYTE_COUNT;
    return request;
}

AudioPcmSamplePacketHandle CreatePacket(TestAudioDevice &device) {
    AudioPcmSamplePacketHandle packet{};
    const AudioStatus status = device.CreatePcmSamplePacket(PacketRequest(), packet);
    if (status != AudioStatus::Success) {
        return AudioPcmSamplePacketHandle{};
    }

    return packet;
}

AudioSceneSourceRecord SourceRecord(AudioPcmSamplePacketHandle packet) {
    AudioSceneSourceRecord source{};
    source.source_id = {1U, 1U};
    source.sound_asset = AssetHandle{2U, 1U};
    source.audio_ready.import_handle = AudioResourcePcmPacketImportHandle{3U, 1U};
    source.audio_ready.packet_request = PacketRequest();
    source.audio_ready.is_ready = true;
    source.packet = packet;
    source.state = AudioSceneSourceState::Playing;
    source.is_active = true;
    return source;
}

AudioSceneSubmitRequest SubmitRequest(std::span<const AudioSceneSourceRecord> sources) {
    AudioSceneSubmitRequest request{};
    request.frame_id = FRAME_ID;
    request.sources = sources;
    return request;
}

int AudioScenePlayingSourceBuildsPcmQueueRequest() {
    TestAudioDevice device;
    AudioDeviceDesc desc{};
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("test audio device init failed");
    }

    const AudioPcmSamplePacketHandle packet = CreatePacket(device);
    if (packet.generation == 0U) {
        return Fail("test audio packet create failed");
    }

    const std::array<AudioSceneSourceRecord, 1U> sources{SourceRecord(packet)};
    std::array<AudioPcmStreamQueueRequest, 1U> requests{};
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(SubmitRequest(sources), requests, &result);
    if (status != AudioSceneStatus::Success) {
        return Fail("audio scene source submit failed");
    }

    if (result.queue_request_count != 1U) {
        return Fail("audio scene queue request count mismatch");
    }

    if (requests[0].expected_packet_id != PACKET_ID) {
        return Fail("audio scene packet id mismatch");
    }

    AudioPcmStreamQueueHandle stream_queue{};
    if (device.CreatePcmStreamQueue(requests[0], stream_queue) != AudioStatus::Success) {
        return Fail("audio scene request did not fit test backend");
    }

    return 0;
}

int AudioSceneBusRoutingMapsFixedBusIdsToQueueIds() {
    TestAudioDevice device;
    AudioDeviceDesc desc{};
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("test audio device init failed");
    }

    const AudioPcmSamplePacketHandle packet = CreatePacket(device);
    if (packet.generation == 0U) {
        return Fail("test audio packet create failed");
    }

    std::array<AudioSceneSourceRecord, 2U> sources{SourceRecord(packet), SourceRecord(packet)};
    sources[0].source_id = {1U, 1U};
    sources[0].bus_id = AUDIO_SCENE_MASTER_BUS_ID;
    sources[1].source_id = {2U, 1U};
    sources[1].bus_id = AUDIO_SCENE_EFFECTS_BUS_ID;
    std::array<AudioPcmStreamQueueRequest, 2U> requests{};
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(SubmitRequest(sources), requests, &result);
    if (status != AudioSceneStatus::Success) {
        return Fail("audio scene bus route submit failed");
    }

    if (result.queue_request_count != 2U) {
        return Fail("audio scene bus route count mismatch");
    }

    const std::uint32_t master_queue_id =
        AUDIO_SCENE_MASTER_BUS_ID * AUDIO_SCENE_BUS_QUEUE_ID_STRIDE + sources[0].source_id.slot;
    if (requests[0].queue_id != master_queue_id) {
        return Fail("audio scene master bus queue id mismatch");
    }

    const std::uint32_t effects_queue_id =
        AUDIO_SCENE_EFFECTS_BUS_ID * AUDIO_SCENE_BUS_QUEUE_ID_STRIDE + sources[1].source_id.slot;
    if (requests[1].queue_id != effects_queue_id) {
        return Fail("audio scene effects bus queue id mismatch");
    }

    if (result.last_bus_id != AUDIO_SCENE_EFFECTS_BUS_ID) {
        return Fail("audio scene result bus id mismatch");
    }

    if (queue.Snapshot().last_bus_id != AUDIO_SCENE_EFFECTS_BUS_ID) {
        return Fail("audio scene snapshot bus id mismatch");
    }

    AudioPcmStreamQueueHandle master_queue{};
    if (device.CreatePcmStreamQueue(requests[0], master_queue) != AudioStatus::Success) {
        return Fail("audio scene master bus did not fit test backend");
    }

    AudioPcmStreamQueueHandle effects_queue{};
    if (device.CreatePcmStreamQueue(requests[1], effects_queue) != AudioStatus::Success) {
        return Fail("audio scene effects bus did not fit test backend");
    }

    return 0;
}

int AudioSceneMissingBackendReturnsExplicitStatus() {
    const AudioPcmSamplePacketHandle packet{1U, 1U};
    const std::array<AudioSceneSourceRecord, 1U> sources{SourceRecord(packet)};
    std::array<AudioPcmStreamQueueRequest, 1U> requests{};
    requests[0].queue_id = 99U;
    AudioSceneSubmitRequest request = SubmitRequest(sources);
    request.backend_available = false;
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(request, requests, &result);
    if (status != AudioSceneStatus::BackendUnavailable) {
        return Fail("audio scene did not report missing backend");
    }

    if (requests[0].queue_id != 99U) {
        return Fail("audio scene mutated output on missing backend");
    }

    return 0;
}

int AudioScenePausedSourceDoesNotSubmitQueue() {
    const AudioPcmSamplePacketHandle packet{1U, 1U};
    std::array<AudioSceneSourceRecord, 1U> sources{SourceRecord(packet)};
    sources[0].state = AudioSceneSourceState::Paused;
    std::array<AudioPcmStreamQueueRequest, 1U> requests{};
    AudioSceneSubmitRequest request = SubmitRequest(sources);
    request.backend_available = false;
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(request, requests, &result);
    if (status != AudioSceneStatus::Success) {
        return Fail("audio scene paused source should not need backend");
    }

    if (result.queue_request_count != 0U) {
        return Fail("audio scene paused source submitted queue");
    }

    if (result.skipped_source_count != 1U) {
        return Fail("audio scene paused source skip count mismatch");
    }

    return 0;
}

int AudioSceneMissingSoundAssetDoesNotMutateOutput() {
    const AudioPcmSamplePacketHandle packet{1U, 1U};
    std::array<AudioSceneSourceRecord, 1U> sources{SourceRecord(packet)};
    sources[0].sound_asset = AssetHandle{};
    std::array<AudioPcmStreamQueueRequest, 1U> requests{};
    requests[0].queue_id = 55U;
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(SubmitRequest(sources), requests, &result);
    if (status != AudioSceneStatus::MissingSoundAsset) {
        return Fail("audio scene did not report missing sound asset");
    }

    if (requests[0].queue_id != 55U) {
        return Fail("audio scene mutated output on missing sound asset");
    }

    return 0;
}

int AudioSceneInvalidBusDoesNotMutateOutput() {
    const AudioPcmSamplePacketHandle packet{1U, 1U};
    std::array<AudioSceneSourceRecord, 1U> sources{SourceRecord(packet)};
    sources[0].bus_id = 0U;
    std::array<AudioPcmStreamQueueRequest, 1U> requests{};
    requests[0].queue_id = 55U;
    AudioSceneSubmitResult result{};

    AudioSceneContractQueue queue;
    const AudioSceneStatus status = queue.SubmitSourceUpdates(SubmitRequest(sources), requests, &result);
    if (status != AudioSceneStatus::InvalidBusId) {
        return Fail("audio scene did not report invalid bus");
    }

    if (requests[0].queue_id != 55U) {
        return Fail("audio scene mutated output on invalid bus");
    }

    if (queue.Snapshot().last_status != AudioSceneStatus::InvalidBusId) {
        return Fail("audio scene snapshot did not record invalid bus");
    }

    return 0;
}

int AudioSceneNoNativeOrUpperDependency() {
    AudioSceneContractQueue queue;
    if (queue.Snapshot().last_status != AudioSceneStatus::Success) {
        return Fail("audio scene default snapshot status changed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_PLAYING_SOURCE) {
        return AudioScenePlayingSourceBuildsPcmQueueRequest();
    }

    if (name == TEST_BUS_ROUTING) {
        return AudioSceneBusRoutingMapsFixedBusIdsToQueueIds();
    }

    if (name == TEST_MISSING_BACKEND) {
        return AudioSceneMissingBackendReturnsExplicitStatus();
    }

    if (name == TEST_PAUSED_SOURCE) {
        return AudioScenePausedSourceDoesNotSubmitQueue();
    }

    if (name == TEST_MISSING_SOUND) {
        return AudioSceneMissingSoundAssetDoesNotMutateOutput();
    }

    if (name == TEST_INVALID_BUS) {
        return AudioSceneInvalidBusDoesNotMutateOutput();
    }

    if (name == TEST_BOUNDARY) {
        return AudioSceneNoNativeOrUpperDependency();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
