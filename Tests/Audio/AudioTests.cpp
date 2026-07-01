// 模块：Tests Audio
// 路径：Tests/Audio/AudioTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "YuEngine/Audio/AudioCallbackDevice.h"
#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketOperation.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRecord.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioPcmSamplePacketSnapshot.h"
#include "YuEngine/Audio/AudioPcmStreamQueueChunk.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackBridge.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackResult.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackSubmitter.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueOperation.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRecord.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/Audio/AudioPcmStreamQueueSnapshot.h"
#include "YuEngine/Audio/AudioPcmStreamQueueStatus.h"
#include "YuEngine/Audio/TestAudioDevice.h"

#include "AudioCallbackDeviceWindowsInternal.h"

using yuengine::audio::AudioBackendKind;
using yuengine::audio::AudioCallbackCompletion;
using yuengine::audio::AudioCallbackDevice;
using yuengine::audio::AudioCallbackDeviceDesc;
using yuengine::audio::AudioCallbackSnapshot;
using yuengine::audio::AudioDeviceDesc;
using yuengine::audio::AudioDeviceSnapshot;
using yuengine::audio::AudioMixResult;
using yuengine::audio::AudioPcmSamplePacketHandle;
using yuengine::audio::AudioPcmSamplePacketOperation;
using yuengine::audio::AudioPcmSamplePacketRecord;
using yuengine::audio::AudioPcmSamplePacketRequest;
using yuengine::audio::AudioPcmSamplePacketSnapshot;
using yuengine::audio::AudioPcmStreamQueueChunk;
using yuengine::audio::AudioPcmStreamQueueCallbackResult;
using yuengine::audio::AudioPcmStreamQueueCallbackSubmitter;
using yuengine::audio::AudioPcmStreamQueueHandle;
using yuengine::audio::AudioPcmStreamQueueOperation;
using yuengine::audio::AudioPcmStreamQueueRecord;
using yuengine::audio::AudioPcmStreamQueueRequest;
using yuengine::audio::AudioPcmStreamQueueSnapshot;
using yuengine::audio::AudioPcmStreamQueueStatus;
using yuengine::audio::AudioSampleFormat;
using yuengine::audio::AudioSourceId;
using yuengine::audio::AudioStatus;
using yuengine::audio::AudioVoiceHandle;
using yuengine::audio::SubmitPcmStreamQueueToCallback;
using TestAudioDevice = yuengine::audio::TestAudioDevice;
using yuengine::audio::CHANNEL_COUNT;
using yuengine::audio::MAX_Q15_GAIN;
using yuengine::audio::MAX_PCM_SAMPLE_PACKETS;
using yuengine::audio::MAX_PCM_STREAM_CHUNK_FRAMES;
using yuengine::audio::MAX_PCM_STREAM_QUEUES;
using yuengine::audio::MAX_SOURCES;
using yuengine::audio::MAX_VOICES;
using yuengine::audio::S16_MAX;
using yuengine::audio::S16_MIN;
using yuengine::audio::SAMPLE_RATE;

namespace {
constexpr const char* TEST_CREATE_DEVICE = "Audio_CreateTestDevice_ReturnsCapabilities";
constexpr const char* TEST_UNSUPPORTED_BACKEND = "Audio_CreateDevice_RejectsUnsupportedBackend";
constexpr const char* TEST_UNSUPPORTED_FORMAT = "Audio_CreateDevice_RejectsUnsupportedFormat";
constexpr const char* TEST_REGISTER_SOURCE = "Audio_RegisterSyntheticSource_ReturnsStableId";
constexpr const char* TEST_SOURCE_CAPACITY = "Audio_SourceCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_START_VOICE = "Audio_StartVoice_ReturnsGenerationHandle";
constexpr const char* TEST_MISSING_SOURCE = "Audio_StartVoice_RejectsMissingSource";
constexpr const char* TEST_INVALID_GAIN = "Audio_StartVoice_RejectsInvalidGainWithoutMutation";
constexpr const char* TEST_VOICE_CAPACITY = "Audio_VoiceCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_STOP_STALE = "Audio_StopVoice_InvalidatesStaleHandle";
constexpr const char* TEST_REINITIALIZE_STALE_HANDLES = "Audio_Reinitialize_InvalidatesPriorSourceAndVoiceHandles";
constexpr const char* TEST_SINGLE_VOICE = "Audio_MixSingleVoice_WritesDeterministicS16StereoSamples";
constexpr const char* TEST_UNITY_GAIN = "Audio_MixUnityGain_PreservesS16EdgeSamples";
constexpr const char* TEST_FRACTIONAL_GAIN = "Audio_MixFractionalGain_RoundsTowardZeroDeterministically";
constexpr const char* TEST_MULTIPLE_VOICES = "Audio_MixMultipleVoices_UsesStableOrderAndSaturates";
constexpr const char* TEST_MAX_VOICE_FULL_SCALE = "Audio_MixMaxVoicesFullScaleSaturatesWithoutOverflow";
constexpr const char* TEST_STOPS_AT_END = "Audio_MixStopsVoiceAtEndOfSource";
constexpr const char* TEST_SILENT_TAIL = "Audio_MixEndedVoice_WritesSilentTail";
constexpr const char* TEST_OVERWRITE = "Audio_MixOverwritesPrefilledDestination";
constexpr const char* TEST_UNDERSIZED_OUTPUT = "Audio_MixRejectsUndersizedBufferWithoutWritingSamples";
constexpr const char* TEST_MIX_OUTPUT_CAPACITY_ENTRY = "Audio_MixRejectsUndersizedBufferReportsCapacityEntry";
constexpr const char* TEST_UNINITIALIZED_LIFECYCLE = "Audio_UninitializedDeviceOperationsReturnExplicitStatusWithoutMutation";
constexpr const char* TEST_NO_GROW = "Audio_Mix_DoesNotGrowVoiceStorage";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Audio_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Audio_NoDeviceCodecResourceScriptUiGameAdapterDependency";
constexpr const char* TEST_CALLBACK_DESC = "Audio_CallbackDesc_DefaultValuesAreBounded";
constexpr const char* TEST_CALLBACK_SNAPSHOT = "Audio_CallbackSnapshot_DefaultValuesAreExplicit";
constexpr const char* TEST_CALLBACK_COMPLETION = "Audio_CallbackCompletion_DefaultValuesAreExplicit";
constexpr const char* TEST_CALLBACK_PUBLIC_CONTRACT = "Audio_CallbackPublicContract_UsesExplicitOwnerAndValues";
constexpr const char* TEST_CALLBACK_UNSUPPORTED_BACKEND = "Audio_CallbackDevice_RejectsUnsupportedBackendBeforeHardware";
constexpr const char* TEST_CALLBACK_UNSUPPORTED_FORMAT = "Audio_CallbackDevice_RejectsUnsupportedFormatBeforeHardware";
constexpr const char* TEST_CALLBACK_INVALID_BUFFER_SHAPE = "Audio_CallbackDevice_RejectsInvalidBufferShapeBeforeHardware";
constexpr const char* TEST_CALLBACK_UNINITIALIZED_OPERATIONS = "Audio_CallbackDevice_UninitializedOperationsReturnExplicitStatus";
constexpr const char *TEST_CALLBACK_CONTROLLED_SUBMIT_DRAIN = "Audio_CallbackDevice_ControlledBackendSubmitsAndDrains";
constexpr const char *TEST_CALLBACK_CONTROLLED_SUBMIT_CAPACITY = "Audio_CallbackDevice_ControlledBackendSubmitCapacityRecordsRejectedEntry";
constexpr const char *TEST_CALLBACK_CONTROLLED_STATUS_SPLIT = "Audio_CallbackDevice_ControlledBackendSplitsUnavailableAndBackendError";
constexpr const char *TEST_CALLBACK_CONTROLLED_SUBMIT_ERROR = "Audio_CallbackDevice_ControlledBackendSubmitErrorRollsBack";
constexpr const char *TEST_CALLBACK_CONTROLLED_CALLBACK_ERROR = "Audio_CallbackDevice_ControlledBackendCallbackErrorReportsFailure";
constexpr const char* TEST_PCM_CREATE_QUERY_RELEASE = "Audio_PcmSamplePacket_CreatesQueriesAndReleasesMetadata";
constexpr const char* TEST_PCM_DUPLICATE = "Audio_PcmSamplePacket_RejectsDuplicatePacketIdWithoutMutation";
constexpr const char* TEST_PCM_UNSUPPORTED_FORMAT = "Audio_PcmSamplePacket_RejectsUnsupportedFormatWithoutMutation";
constexpr const char* TEST_PCM_UNSUPPORTED_SAMPLE_RATE = "Audio_PcmSamplePacket_RejectsUnsupportedSampleRateWithoutMutation";
constexpr const char* TEST_PCM_UNSUPPORTED_CHANNEL_COUNT = "Audio_PcmSamplePacket_RejectsUnsupportedChannelCountWithoutMutation";
constexpr const char* TEST_PCM_ZERO_FRAME_COUNT = "Audio_PcmSamplePacket_RejectsZeroFrameCountWithoutMutation";
constexpr const char* TEST_PCM_SAMPLE_COUNT_MISMATCH = "Audio_PcmSamplePacket_RejectsSampleCountMismatchWithoutMutation";
constexpr const char* TEST_PCM_BYTE_COUNT_MISMATCH = "Audio_PcmSamplePacket_RejectsByteCountMismatchWithoutMutation";
constexpr const char* TEST_PCM_CAPACITY = "Audio_PcmSamplePacket_RejectsCapacityOverflowWithoutMutation";
constexpr const char* TEST_PCM_STALE_HANDLE = "Audio_PcmSamplePacket_RejectsStaleHandleWithoutMutation";
constexpr const char* TEST_PCM_SNAPSHOT_COUNTERS = "Audio_PcmSamplePacket_SnapshotTracksCounters";
constexpr const char* TEST_PCM_SOURCE_VOICE_BOUNDARY = "Audio_PcmSamplePacket_RejectionsDoNotMutateSourceVoiceState";
constexpr const char* TEST_PCM_PUBLIC_CONTRACT = "Audio_PcmSamplePacket_PublicContractsArePlainValues";
constexpr const char* TEST_STREAM_QUEUE_LIFECYCLE = "Audio_PcmStreamQueue_QueuesDrainsAndReleasesMetadata";
constexpr const char* TEST_STREAM_QUEUE_INVALID_PACKET = "Audio_PcmStreamQueue_RejectsInvalidPacketHandleWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_RELEASED_PACKET = "Audio_PcmStreamQueue_RejectsReleasedPacketHandleWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_PACKET_ID_MISMATCH = "Audio_PcmStreamQueue_RejectsPacketIdMismatchWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_DUPLICATE = "Audio_PcmStreamQueue_RejectsDuplicateQueueIdWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_RANGE = "Audio_PcmStreamQueue_RejectsFrameRangeOverflowWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_SAMPLE_COUNT = "Audio_PcmStreamQueue_RejectsSampleCountMismatchWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_BYTE_COUNT = "Audio_PcmStreamQueue_RejectsByteCountMismatchWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_CHUNK = "Audio_PcmStreamQueue_RejectsInvalidChunkFrameCountWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_CAPACITY = "Audio_PcmStreamQueue_RejectsCapacityOverflowWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_CAPACITY_ENTRY = "Audio_PcmStreamQueue_CapacityOverflowReportsRejectedEntry";
constexpr const char* TEST_STREAM_QUEUE_SMALL_OUTPUT = "Audio_PcmStreamQueue_DrainRejectsSmallOutputWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_STALE = "Audio_PcmStreamQueue_RejectsStaleHandleWithoutMutation";
constexpr const char* TEST_STREAM_QUEUE_NO_MUTATION = "Audio_PcmStreamQueue_RejectionsDoNotMutatePacketSourceVoiceCallbackState";
constexpr const char* TEST_STREAM_QUEUE_SNAPSHOT = "Audio_PcmStreamQueue_SnapshotTracksCounters";
constexpr const char* TEST_STREAM_QUEUE_PUBLIC_CONTRACT = "Audio_PcmStreamQueue_PublicContractsArePlainValues";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_SUBMIT = "Audio_PcmStreamQueueCallback_SubmitsDrainedChunks";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_PARTIAL = "Audio_PcmStreamQueueCallback_RejectsPartialFinalChunkWithoutDrain";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_SMALL_SPAN = "Audio_PcmStreamQueueCallback_RejectsSmallSampleSpanWithoutDrain";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_SUBMIT_FAILURE = "Audio_PcmStreamQueueCallback_StopsOnSubmitFailure";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_UNSTARTED = "Audio_PcmStreamQueueCallback_RejectsUnstartedCallbackWithoutDrain";
constexpr const char *TEST_STREAM_QUEUE_CALLBACK_PUBLIC_CONTRACT = "Audio_PcmStreamQueueCallback_PublicContractsArePlainValues";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* REINIT_SOURCE_REGISTRATION_MESSAGE = "source registration failed";
constexpr const char* REINIT_VOICE_START_MESSAGE = "voice start failed";
constexpr const char* REINIT_DEVICE_MESSAGE = "device reinitialize failed";
constexpr const char* REINIT_MIX_MESSAGE = "mix after reinitialize failed";
constexpr const char* REINIT_MIX_FRAME_COUNT_MESSAGE = "mix after reinitialize reported wrong frame count";
constexpr const char* REINIT_MIX_STALE_VOICE_MESSAGE = "mix after reinitialize consumed stale voice";
constexpr const char* REINIT_MIX_ACTIVE_VOICE_MESSAGE = "mix after reinitialize observed stale active voice";
constexpr const char* REINIT_ACTIVE_SOURCE_REGISTRATION_MESSAGE = "source registration after reinitialize failed";
constexpr const char* REINIT_ACTIVE_VOICE_START_MESSAGE = "voice start after reinitialize failed";
constexpr const char* REINIT_STALE_VOICE_ACCEPTED_MESSAGE = "stale voice handle from prior initialize was accepted";
constexpr const char* REINIT_STALE_VOICE_COUNT_MESSAGE = "stale voice handle changed active voice count";
constexpr const char* REINIT_STALE_SOURCE_ACCEPTED_MESSAGE = "stale source from prior initialize was accepted";
constexpr const char* REINIT_STALE_SOURCE_COUNT_MESSAGE = "stale source changed active voice count";
constexpr const char* REINIT_ACTIVE_VOICE_MESSAGE = "active voice did not survive stale handle checks";
constexpr std::int16_t PREFILL_SAMPLE = 1234;
constexpr std::int16_t SENTINEL_SAMPLE = -1234;
using TestFunction = int (*)();

struct ScopedAudioCallbackBackendTestConfig final {
    explicit ScopedAudioCallbackBackendTestConfig(const yuengine::audio::AudioCallbackDeviceBackendTestConfig &config) {
        yuengine::audio::SetAudioCallbackDeviceBackendTestConfig(config);
    }

    ~ScopedAudioCallbackBackendTestConfig() {
        yuengine::audio::ClearAudioCallbackDeviceBackendTestConfig();
    }
};

struct FakeCallbackSubmitContext final {
    std::size_t attempted_call_count = 0U;
    std::size_t submitted_call_count = 0U;
    std::size_t submitted_frame_count = 0U;
    std::size_t submitted_sample_count = 0U;
    std::size_t expected_frames_per_buffer = 0U;
    std::size_t fail_on_call = 0U;
    AudioStatus failure_status = AudioStatus::Success;
    std::int16_t first_submitted_sample = 0;
    std::int16_t last_submitted_sample = 0;
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int FailAfterShutdown(AudioCallbackDevice &device, std::string_view message) {
    device.Shutdown();
    return Fail(message);
}

TestAudioDevice CreateInitializedDevice() {
    TestAudioDevice device;
    device.Initialize(AudioDeviceDesc{});
    return device;
}

std::array<std::int16_t, 4U> BasicSourceSamples() {
    return {1000, -1000, 2000, -2000};
}

bool RegisterBasicSource(TestAudioDevice& device, AudioSourceId& out_source) {
    const std::array<std::int16_t, 4U> samples = BasicSourceSamples();
    return device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, out_source) == AudioStatus::Success;
}

bool StartBasicVoice(TestAudioDevice& device, AudioVoiceHandle& out_voice) {
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return false;
    }

    return device.StartVoice(source, MAX_Q15_GAIN, out_voice) == AudioStatus::Success;
}

bool SamplesEqual(std::span<const std::int16_t> left, std::span<const std::int16_t> right) {
    if (left.size() != right.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < left.size(); ++index) {
        if (left[index] != right[index]) {
            return false;
        }
    }

    return true;
}

bool SnapshotsEqual(const AudioDeviceSnapshot& left, const AudioDeviceSnapshot& right) {
    return left.source_capacity == right.source_capacity &&
           left.voice_capacity == right.voice_capacity &&
           left.source_count == right.source_count &&
           left.active_voice_count == right.active_voice_count &&
           left.last_required_source_count == right.last_required_source_count &&
           left.last_required_voice_count == right.last_required_voice_count &&
           left.last_failed_mix_requested_frame_count == right.last_failed_mix_requested_frame_count &&
           left.last_failed_mix_output_sample_capacity == right.last_failed_mix_output_sample_capacity &&
           left.last_failed_mix_required_sample_count == right.last_failed_mix_required_sample_count &&
           left.last_failed_mix_active_voice_count == right.last_failed_mix_active_voice_count &&
           left.voice_storage_capacity_before_mix == right.voice_storage_capacity_before_mix &&
           left.voice_storage_capacity_after_last_mix == right.voice_storage_capacity_after_last_mix &&
           left.registered_source_count == right.registered_source_count &&
           left.started_voice_count == right.started_voice_count &&
           left.stopped_voice_count == right.stopped_voice_count &&
           left.mixed_frame_count == right.mixed_frame_count &&
           left.output_sample_write_count == right.output_sample_write_count &&
           left.failed_operation_count == right.failed_operation_count &&
           left.last_status == right.last_status &&
           left.last_frames_written == right.last_frames_written &&
           left.allocation_accounting_status == right.allocation_accounting_status;
}

bool MixOutputCapacityEntryIsClear(const AudioMixResult &result) {
    return result.failed_mix_requested_frame_count == 0U &&
           result.failed_mix_output_sample_capacity == 0U &&
           result.failed_mix_required_sample_count == 0U &&
           result.failed_mix_active_voice_count == 0U;
}

bool MixOutputCapacityEntryIsClear(const AudioDeviceSnapshot &snapshot) {
    return snapshot.last_failed_mix_requested_frame_count == 0U &&
           snapshot.last_failed_mix_output_sample_capacity == 0U &&
           snapshot.last_failed_mix_required_sample_count == 0U &&
           snapshot.last_failed_mix_active_voice_count == 0U;
}

bool CallbackSnapshotsEqual(const AudioCallbackSnapshot &left, const AudioCallbackSnapshot &right) {
    return left.buffer_capacity == right.buffer_capacity &&
           left.frames_per_buffer == right.frames_per_buffer &&
           left.sample_rate == right.sample_rate &&
           left.channel_count == right.channel_count &&
           left.setup_allocation_count == right.setup_allocation_count &&
           left.submitted_buffer_count == right.submitted_buffer_count &&
           left.completed_callback_count == right.completed_callback_count &&
           left.failed_submission_count == right.failed_submission_count &&
           left.failed_callback_count == right.failed_callback_count &&
           left.underrun_count == right.underrun_count &&
           left.shutdown_callback_count == right.shutdown_callback_count &&
           left.queued_buffer_count == right.queued_buffer_count &&
           left.max_queued_buffer_count == right.max_queued_buffer_count &&
           left.last_failed_submission_sequence == right.last_failed_submission_sequence &&
           left.last_failed_submission_buffer_capacity == right.last_failed_submission_buffer_capacity &&
           left.last_failed_submission_queued_buffer_count == right.last_failed_submission_queued_buffer_count &&
           left.last_failed_submission_frame_count == right.last_failed_submission_frame_count &&
           left.last_failed_submission_sample_count == right.last_failed_submission_sample_count &&
           left.last_required_queued_buffer_count == right.last_required_queued_buffer_count &&
           left.drained_completion_count == right.drained_completion_count &&
           left.last_status == right.last_status &&
           left.initialized == right.initialized &&
           left.started == right.started &&
           left.shutdown == right.shutdown;
}

bool CallbackSubmissionCapacityFieldsAreClear(const AudioCallbackSnapshot &snapshot) {
    return snapshot.last_failed_submission_sequence == 0U &&
           snapshot.last_failed_submission_buffer_capacity == 0U &&
           snapshot.last_failed_submission_queued_buffer_count == 0U &&
           snapshot.last_failed_submission_frame_count == 0U &&
           snapshot.last_failed_submission_sample_count == 0U &&
           snapshot.last_required_queued_buffer_count == 0U;
}

bool CallbackSubmissionCapacityFieldsMatch(const AudioCallbackSnapshot &snapshot,
                                           std::uint64_t sequence,
                                           std::size_t buffer_capacity,
                                           std::size_t queued_buffer_count,
                                           std::size_t frame_count,
                                           std::size_t sample_count,
                                           std::size_t required_queued_buffer_count) {
    return snapshot.last_failed_submission_sequence == sequence &&
           snapshot.last_failed_submission_buffer_capacity == buffer_capacity &&
           snapshot.last_failed_submission_queued_buffer_count == queued_buffer_count &&
           snapshot.last_failed_submission_frame_count == frame_count &&
           snapshot.last_failed_submission_sample_count == sample_count &&
           snapshot.last_required_queued_buffer_count == required_queued_buffer_count;
}

AudioPcmSamplePacketRequest BasicPcmSamplePacketRequest(std::uint32_t packet_id) {
    constexpr std::size_t FRAME_COUNT = 2U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    constexpr std::size_t BYTE_COUNT = SAMPLE_COUNT * sizeof(std::int16_t);
    return AudioPcmSamplePacketRequest{
        packet_id,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        FRAME_COUNT,
        SAMPLE_COUNT,
        BYTE_COUNT};
}

bool CreateBasicPcmSamplePacket(TestAudioDevice& device, std::uint32_t packet_id, AudioPcmSamplePacketHandle& out_packet) {
    const AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(packet_id);
    return device.CreatePcmSamplePacket(request, out_packet) == AudioStatus::Success;
}

bool ExpectPcmCreateRejectedWithoutActiveMutation(TestAudioDevice& device, AudioPcmSamplePacketRequest request, AudioStatus expected_status) {
    const AudioPcmSamplePacketSnapshot before_snapshot = device.PcmSamplePacketSnapshot();
    AudioPcmSamplePacketHandle packet{};
    const AudioStatus status = device.CreatePcmSamplePacket(request, packet);
    if (status != expected_status) {
        return false;
    }

    const AudioPcmSamplePacketSnapshot after_snapshot = device.PcmSamplePacketSnapshot();
    if (after_snapshot.active_packet_count != before_snapshot.active_packet_count) {
        return false;
    }

    if (after_snapshot.created_packet_count != before_snapshot.created_packet_count) {
        return false;
    }

    if (after_snapshot.rejected_packet_count != before_snapshot.rejected_packet_count + 1U) {
        return false;
    }

    return true;
}

AudioPcmStreamQueueRequest BasicPcmStreamQueueRequest(std::uint32_t queue_id, AudioPcmSamplePacketHandle packet, std::uint32_t expected_packet_id) {
    constexpr std::size_t FIRST_FRAME = 0U;
    constexpr std::size_t FRAME_COUNT = 2U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    constexpr std::size_t BYTE_COUNT = SAMPLE_COUNT * sizeof(std::int16_t);
    constexpr std::size_t CHUNK_FRAME_COUNT = 1U;
    return AudioPcmStreamQueueRequest{
        queue_id,
        packet,
        expected_packet_id,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        FIRST_FRAME,
        FRAME_COUNT,
        SAMPLE_COUNT,
        BYTE_COUNT,
        CHUNK_FRAME_COUNT};
}

bool CreateBasicPcmStreamQueue(TestAudioDevice& device, std::uint32_t packet_id, std::uint32_t queue_id, AudioPcmStreamQueueHandle& out_queue) {
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, packet_id, packet)) {
        return false;
    }

    const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(queue_id, packet, packet_id);
    return device.CreatePcmStreamQueue(request, out_queue) == AudioStatus::Success;
}

AudioPcmSamplePacketRequest PcmSamplePacketRequestForFrames(std::uint32_t packet_id, std::size_t frame_count) {
    const std::size_t sample_count = frame_count * CHANNEL_COUNT;
    const std::size_t byte_count = sample_count * sizeof(std::int16_t);
    return AudioPcmSamplePacketRequest{
        packet_id,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        frame_count,
        sample_count,
        byte_count};
}

AudioPcmStreamQueueRequest PcmStreamQueueRequestForFrames(std::uint32_t queue_id,
                                                          AudioPcmSamplePacketHandle packet,
                                                          std::uint32_t expected_packet_id,
                                                          std::size_t frame_count,
                                                          std::size_t chunk_frame_count) {
    const std::size_t sample_count = frame_count * CHANNEL_COUNT;
    const std::size_t byte_count = sample_count * sizeof(std::int16_t);
    return AudioPcmStreamQueueRequest{
        queue_id,
        packet,
        expected_packet_id,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        0U,
        frame_count,
        sample_count,
        byte_count,
        chunk_frame_count};
}

bool CreatePcmStreamQueueForFrames(TestAudioDevice& device,
                                   std::uint32_t packet_id,
                                   std::uint32_t queue_id,
                                   std::size_t frame_count,
                                   std::size_t chunk_frame_count,
                                   AudioPcmStreamQueueHandle& out_queue) {
    const AudioPcmSamplePacketRequest packet_request = PcmSamplePacketRequestForFrames(packet_id, frame_count);
    AudioPcmSamplePacketHandle packet{};
    if (device.CreatePcmSamplePacket(packet_request, packet) != AudioStatus::Success) {
        return false;
    }

    const AudioPcmStreamQueueRequest queue_request = PcmStreamQueueRequestForFrames(queue_id, packet, packet_id, frame_count, chunk_frame_count);
    return device.CreatePcmStreamQueue(queue_request, out_queue) == AudioStatus::Success;
}

AudioStatus FakeSubmitS16Buffer(void *context, std::span<const std::int16_t> interleaved_samples, std::size_t frame_count) {
    if (context == nullptr) {
        return AudioStatus::InvalidDescriptor;
    }

    FakeCallbackSubmitContext *submit_context = static_cast<FakeCallbackSubmitContext *>(context);
    ++submit_context->attempted_call_count;
    if (submit_context->fail_on_call == submit_context->attempted_call_count) {
        return submit_context->failure_status;
    }

    if (frame_count != submit_context->expected_frames_per_buffer) {
        return AudioStatus::InvalidDescriptor;
    }

    const std::size_t expected_sample_count = frame_count * CHANNEL_COUNT;
    if (interleaved_samples.size() != expected_sample_count) {
        return AudioStatus::InvalidDescriptor;
    }

    if (submit_context->submitted_call_count == 0U) {
        submit_context->first_submitted_sample = interleaved_samples.front();
    }

    ++submit_context->submitted_call_count;
    submit_context->submitted_frame_count += frame_count;
    submit_context->submitted_sample_count += interleaved_samples.size();
    submit_context->last_submitted_sample = interleaved_samples.back();
    return AudioStatus::Success;
}

AudioPcmStreamQueueCallbackSubmitter FakeCallbackSubmitter(FakeCallbackSubmitContext *context, std::size_t frames_per_buffer) {
    AudioPcmStreamQueueCallbackSubmitter submitter{};
    submitter.context = context;
    submitter.submit_s16_buffer = FakeSubmitS16Buffer;
    submitter.frames_per_buffer = frames_per_buffer;
    submitter.started = true;
    return submitter;
}

bool ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(TestAudioDevice& device, AudioPcmStreamQueueRequest request, AudioStatus expected_status) {
    const AudioPcmStreamQueueSnapshot before_queue_snapshot = device.PcmStreamQueueSnapshot();
    const AudioPcmSamplePacketSnapshot before_packet_snapshot = device.PcmSamplePacketSnapshot();
    AudioPcmStreamQueueHandle queue{};
    const AudioStatus status = device.CreatePcmStreamQueue(request, queue);
    if (status != expected_status) {
        return false;
    }

    const AudioPcmStreamQueueSnapshot after_queue_snapshot = device.PcmStreamQueueSnapshot();
    if (after_queue_snapshot.active_queue_count != before_queue_snapshot.active_queue_count) {
        return false;
    }

    if (after_queue_snapshot.created_queue_count != before_queue_snapshot.created_queue_count) {
        return false;
    }

    if (after_queue_snapshot.rejected_queue_count != before_queue_snapshot.rejected_queue_count + 1U) {
        return false;
    }

    const AudioPcmSamplePacketSnapshot after_packet_snapshot = device.PcmSamplePacketSnapshot();
    if (after_packet_snapshot.active_packet_count != before_packet_snapshot.active_packet_count) {
        return false;
    }

    if (after_packet_snapshot.created_packet_count != before_packet_snapshot.created_packet_count) {
        return false;
    }

    if (after_packet_snapshot.released_packet_count != before_packet_snapshot.released_packet_count) {
        return false;
    }

    if (after_packet_snapshot.rejected_packet_count != before_packet_snapshot.rejected_packet_count) {
        return false;
    }

    return true;
}

bool PcmStreamQueueCapacityEntryMatches(const AudioPcmStreamQueueSnapshot &snapshot,
                                        const AudioPcmStreamQueueRequest &request,
                                        std::size_t active_queue_count,
                                        std::size_t required_queue_count) {
    if (snapshot.last_required_queue_count != required_queue_count) {
        return false;
    }

    if (snapshot.last_required_output_chunk_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_queue_id != request.queue_id) {
        return false;
    }

    if (snapshot.last_failed_packet_slot != request.packet.slot) {
        return false;
    }

    if (snapshot.last_failed_packet_generation != request.packet.generation) {
        return false;
    }

    if (snapshot.last_failed_packet_id != request.expected_packet_id) {
        return false;
    }

    if (snapshot.last_failed_queue_capacity != MAX_PCM_STREAM_QUEUES) {
        return false;
    }

    if (snapshot.last_failed_active_queue_count != active_queue_count) {
        return false;
    }

    return true;
}

bool PcmStreamQueueCapacityEntryIsClear(const AudioPcmStreamQueueSnapshot &snapshot) {
    if (snapshot.last_required_queue_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_queue_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_packet_slot != 0U) {
        return false;
    }

    if (snapshot.last_failed_packet_generation != 0U) {
        return false;
    }

    if (snapshot.last_failed_packet_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_queue_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_active_queue_count != 0U) {
        return false;
    }

    return true;
}

bool ExpectCallbackInitializeStatusWithoutMutation(const AudioCallbackDeviceDesc &desc, AudioStatus expected_status) {
    AudioCallbackDevice device;
    const AudioStatus status = device.Initialize(desc);
    if (status != expected_status) {
        return false;
    }

    return CallbackSnapshotsEqual(device.Snapshot(), AudioCallbackSnapshot{});
}

bool MixMaxVoicesFullScale(std::int16_t source_sample, std::int16_t expected_sample) {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{source_sample, source_sample};
    AudioSourceId source{};
    if (device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source) != AudioStatus::Success) {
        return false;
    }

    for (std::size_t index = 0U; index < MAX_VOICES; ++index) {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, MAX_Q15_GAIN, voice) != AudioStatus::Success) {
            return false;
        }
    }

    std::array<std::int16_t, 2U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    return result.status == AudioStatus::Success &&
           result.frames_written == 1U &&
           output[0U] == expected_sample &&
           output[1U] == expected_sample;
}

int AudioCreateTestDeviceReturnsCapabilities() {
    TestAudioDevice device;
    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return Fail("test audio device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.backend_kind != AudioBackendKind::Test) {
        return Fail("capabilities did not report test backend");
    }

    if (capabilities.format != AudioSampleFormat::Signed16) {
        return Fail("capabilities did not report S16");
    }

    if (capabilities.sample_rate != SAMPLE_RATE) {
        return Fail("capabilities did not report fixed sample rate");
    }

    if (capabilities.channel_count != CHANNEL_COUNT) {
        return Fail("capabilities did not report stereo");
    }

    if (!capabilities.supports_deterministic_mix) {
        return Fail("capabilities did not report deterministic mix support");
    }

    if (device.Snapshot().last_status != AudioStatus::Success) {
        return Fail("test device initialize status was not success");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedBackend() {
    TestAudioDevice device;
    AudioDeviceDesc desc{};
    desc.backend_kind = AudioBackendKind::Unsupported;
    if (device.Initialize(desc) != AudioStatus::UnsupportedBackend) {
        return Fail("unsupported backend was not rejected");
    }

    if (device.Snapshot().source_capacity != 0U) {
        return Fail("unsupported backend mutated source capacity");
    }

    if (device.Snapshot().last_status != AudioStatus::UnsupportedBackend) {
        return Fail("unsupported backend status was not recorded");
    }

    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return Fail("device did not recover after unsupported backend");
    }

    if (device.Snapshot().last_status != AudioStatus::Success) {
        return Fail("successful initialize did not reset device status");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedFormat() {
    TestAudioDevice device;
    AudioDeviceDesc format_desc{};
    format_desc.format = AudioSampleFormat::Unsupported;
    if (device.Initialize(format_desc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported sample format was not rejected");
    }

    AudioDeviceDesc sample_rate_desc{};
    sample_rate_desc.sample_rate = 44100U;
    if (device.Initialize(sample_rate_desc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported sample rate was not rejected");
    }

    AudioDeviceDesc channel_desc{};
    channel_desc.channel_count = 1U;
    if (device.Initialize(channel_desc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported channel count was not rejected");
    }

    if (device.Snapshot().last_status != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported format status was not recorded");
    }

    return 0;
}

int AudioRegisterSyntheticSourceReturnsStableId() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    if (source.slot != 0U) {
        return Fail("first source used unexpected slot");
    }

    if (source.generation == 0U) {
        return Fail("source generation was invalid");
    }

    if (device.Snapshot().registered_source_count != 1U) {
        return Fail("registered source count was not updated");
    }

    if (device.Snapshot().last_status != AudioStatus::Success) {
        return Fail("source registration did not reset device status");
    }

    return 0;
}

int AudioSourceCapacityOverflowDoesNotMutate() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{1, 1};
    for (std::size_t index = 0U; index < MAX_SOURCES; ++index) {
        AudioSourceId source{};
        const AudioStatus status = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
        if (status != AudioStatus::Success) {
            return Fail("source registration failed before capacity");
        }
    }

    const auto before_snapshot = device.Snapshot();
    AudioSourceId overflow_source{};
    const AudioStatus overflow_status = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, overflow_source);
    if (overflow_status != AudioStatus::CapacityExceeded) {
        return Fail("source overflow did not return capacity status");
    }

    if (device.Snapshot().source_count != before_snapshot.source_count) {
        return Fail("source overflow changed source count");
    }

    if (device.Snapshot().last_status != AudioStatus::CapacityExceeded) {
        return Fail("source overflow status was not recorded");
    }

    const std::size_t required_source_count = before_snapshot.source_count + 1U;
    if (device.Snapshot().last_required_source_count != required_source_count) {
        return Fail("source overflow missed required source count");
    }

    if (device.Snapshot().last_required_voice_count != 0U) {
        return Fail("source overflow reported required voice count");
    }

    return 0;
}

int AudioStartVoiceReturnsGenerationHandle() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    if (voice.slot != 0U) {
        return Fail("first voice used unexpected slot");
    }

    if (voice.generation == 0U) {
        return Fail("voice generation was invalid");
    }

    if (device.Snapshot().active_voice_count != 1U) {
        return Fail("active voice count was not updated");
    }

    if (device.Snapshot().last_status != AudioStatus::Success) {
        return Fail("voice start did not reset device status");
    }

    return 0;
}

int AudioStartVoiceRejectsMissingSource() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (device.StartVoice(AudioSourceId{99U, 1U}, MAX_Q15_GAIN, voice) != AudioStatus::SourceNotFound) {
        return Fail("missing source was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("missing source changed active voice count");
    }

    if (device.Snapshot().last_status != AudioStatus::SourceNotFound) {
        return Fail("missing source status was not recorded");
    }

    return 0;
}

int AudioStartVoiceRejectsInvalidGainWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    AudioVoiceHandle voice{};
    const AudioStatus status = device.StartVoice(source, MAX_Q15_GAIN + 1U, voice);
    if (status != AudioStatus::InvalidGain) {
        return Fail("invalid gain was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("invalid gain changed active voice count");
    }

    if (device.Snapshot().last_status != AudioStatus::InvalidGain) {
        return Fail("invalid gain status was not recorded");
    }

    if (device.StartVoice(source, MAX_Q15_GAIN, voice) != AudioStatus::Success) {
        return Fail("valid voice start after invalid gain failed");
    }

    if (device.Snapshot().last_status != AudioStatus::Success) {
        return Fail("valid voice start did not reset invalid gain status");
    }

    return 0;
}

int AudioVoiceCapacityOverflowDoesNotMutate() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    for (std::size_t index = 0U; index < MAX_VOICES; ++index) {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, MAX_Q15_GAIN, voice) != AudioStatus::Success) {
            return Fail("voice start failed before capacity");
        }
    }

    const auto before_snapshot = device.Snapshot();
    AudioVoiceHandle overflow_voice{};
    if (device.StartVoice(source, MAX_Q15_GAIN, overflow_voice) != AudioStatus::CapacityExceeded) {
        return Fail("voice overflow did not return capacity status");
    }

    if (device.Snapshot().active_voice_count != before_snapshot.active_voice_count) {
        return Fail("voice overflow changed active voice count");
    }

    if (device.Snapshot().last_status != AudioStatus::CapacityExceeded) {
        return Fail("voice overflow status was not recorded");
    }

    const std::size_t required_voice_count = before_snapshot.active_voice_count + 1U;
    if (device.Snapshot().last_required_voice_count != required_voice_count) {
        return Fail("voice overflow missed required voice count");
    }

    if (device.Snapshot().last_required_source_count != 0U) {
        return Fail("voice overflow reported required source count");
    }

    return 0;
}

int AudioStopVoiceInvalidatesStaleHandle() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    if (device.StopVoice(voice) != AudioStatus::Success) {
        return Fail("voice stop failed");
    }

    if (device.StopVoice(voice) != AudioStatus::InvalidHandle) {
        return Fail("stale voice handle was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("stopped voice remained active");
    }

    if (device.Snapshot().last_status != AudioStatus::InvalidHandle) {
        return Fail("stale voice status was not recorded");
    }

    return 0;
}

int AudioReinitializeInvalidatesPriorSourceAndVoiceHandles() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId stale_source{};
    if (!RegisterBasicSource(device, stale_source)) {
        return Fail(REINIT_SOURCE_REGISTRATION_MESSAGE);
    }

    AudioVoiceHandle stale_voice{};
    if (device.StartVoice(stale_source, MAX_Q15_GAIN, stale_voice) != AudioStatus::Success) {
        return Fail(REINIT_VOICE_START_MESSAGE);
    }

    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return Fail(REINIT_DEVICE_MESSAGE);
    }

    std::array<std::int16_t, 4U> stale_mix_output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult stale_mix_result = device.Mix(std::span<std::int16_t>(stale_mix_output.data(), stale_mix_output.size()), 2U);
    if (stale_mix_result.status != AudioStatus::Success) {
        return Fail(REINIT_MIX_MESSAGE);
    }

    if (stale_mix_result.frames_written != 2U) {
        return Fail(REINIT_MIX_FRAME_COUNT_MESSAGE);
    }

    const std::array<std::int16_t, 4U> silent_output{0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(stale_mix_output.data(), stale_mix_output.size()), std::span<const std::int16_t>(silent_output.data(), silent_output.size()))) {
        return Fail(REINIT_MIX_STALE_VOICE_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail(REINIT_MIX_ACTIVE_VOICE_MESSAGE);
    }

    AudioSourceId active_source{};
    if (!RegisterBasicSource(device, active_source)) {
        return Fail(REINIT_ACTIVE_SOURCE_REGISTRATION_MESSAGE);
    }

    AudioVoiceHandle active_voice{};
    if (device.StartVoice(active_source, MAX_Q15_GAIN, active_voice) != AudioStatus::Success) {
        return Fail(REINIT_ACTIVE_VOICE_START_MESSAGE);
    }

    const AudioDeviceSnapshot before_snapshot = device.Snapshot();
    if (device.StopVoice(stale_voice) != AudioStatus::InvalidHandle) {
        return Fail(REINIT_STALE_VOICE_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != before_snapshot.active_voice_count) {
        return Fail(REINIT_STALE_VOICE_COUNT_MESSAGE);
    }

    AudioVoiceHandle stale_source_voice{};
    if (device.StartVoice(stale_source, MAX_Q15_GAIN, stale_source_voice) != AudioStatus::SourceNotFound) {
        return Fail(REINIT_STALE_SOURCE_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != before_snapshot.active_voice_count) {
        return Fail(REINIT_STALE_SOURCE_COUNT_MESSAGE);
    }

    if (device.StopVoice(active_voice) != AudioStatus::Success) {
        return Fail(REINIT_ACTIVE_VOICE_MESSAGE);
    }

    return 0;
}

int AudioMixSingleVoiceWritesDeterministicS16StereoSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::Success) {
        return Fail("mix failed");
    }

    const std::array<std::int16_t, 4U> expected = BasicSourceSamples();
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("single voice output did not match source samples");
    }

    return 0;
}

int AudioMixUnityGainPreservesS16EdgeSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{S16_MIN, S16_MAX, -1, 1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(samples.data(), samples.size()))) {
        return Fail("unity gain did not preserve S16 edge samples");
    }

    return 0;
}

int AudioMixFractionalGainRoundsTowardZeroDeterministically() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{1000, -1000, 1, -1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, 16384U, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const std::array<std::int16_t, 4U> expected{500, -500, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("fractional Q15 gain did not round toward zero deterministically");
    }

    return 0;
}

int AudioMixMultipleVoicesUsesStableOrderAndSaturates() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> first_samples{30000, -32000};
    const std::array<std::int16_t, 2U> second_samples{30000, -32000};
    AudioSourceId first_source{};
    AudioSourceId second_source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(first_samples.data(), first_samples.size()), 1U, first_source);
    device.RegisterSyntheticSource(std::span<const std::int16_t>(second_samples.data(), second_samples.size()), 1U, second_source);
    AudioVoiceHandle first_voice{};
    AudioVoiceHandle second_voice{};
    device.StartVoice(first_source, MAX_Q15_GAIN, first_voice);
    device.StartVoice(second_source, MAX_Q15_GAIN, second_voice);

    std::array<std::int16_t, 2U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    if (output[0U] != S16_MAX) {
        return Fail("positive mix did not saturate");
    }

    if (output[1U] != S16_MIN) {
        return Fail("negative mix did not saturate");
    }

    return 0;
}

int AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow() {
    if (!MixMaxVoicesFullScale(S16_MAX, S16_MAX)) {
        return Fail("positive max-voice full-scale mix did not saturate safely");
    }

    if (!MixMaxVoicesFullScale(S16_MIN, S16_MIN)) {
        return Fail("negative max-voice full-scale mix did not saturate safely");
    }

    return 0;
}

int AudioMixStopsVoiceAtEndOfSource() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (device.StopVoice(voice) != AudioStatus::InvalidHandle) {
        return Fail("ended voice handle was not invalidated");
    }

    if (device.Snapshot().stopped_voice_count != 1U) {
        return Fail("ended voice stop count was not recorded");
    }

    return 0;
}

int AudioMixEndedVoiceWritesSilentTail() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 6U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 3U);
    const std::array<std::int16_t, 6U> expected{100, -100, 0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("ended voice tail was not written as silence");
    }

    return 0;
}

int AudioMixOverwritesPrefilledDestination() {
    TestAudioDevice device = CreateInitializedDevice();
    std::array<std::int16_t, 4U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::Success) {
        return Fail("silent mix failed");
    }

    const std::array<std::int16_t, 4U> expected{0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("mix did not overwrite prefilled destination with silence");
    }

    return 0;
}

int AudioMixRejectsUndersizedBufferWithoutWritingSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 3U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::CapacityExceeded) {
        return Fail("undersized mix did not return capacity status");
    }

    if (result.frames_written != 0U) {
        return Fail("undersized mix reported nonzero frames written");
    }

    for (const std::int16_t sample : output) {
        if (sample != SENTINEL_SAMPLE) {
            return Fail("undersized mix mutated output");
        }
    }

    if (device.Snapshot().last_status != AudioStatus::CapacityExceeded) {
        return Fail("undersized mix status was not recorded");
    }

    return 0;
}

int AudioMixRejectsUndersizedBufferReportsCapacityEntry() {
    constexpr std::size_t REQUESTED_FRAME_COUNT = 2U;
    constexpr std::size_t REQUIRED_SAMPLE_COUNT = REQUESTED_FRAME_COUNT * CHANNEL_COUNT;
    constexpr std::size_t OUTPUT_SAMPLE_CAPACITY = REQUIRED_SAMPLE_COUNT - 1U;

    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, OUTPUT_SAMPLE_CAPACITY> output{};
    output.fill(SENTINEL_SAMPLE);
    std::span<std::int16_t> output_span(output.data(), output.size());
    const AudioMixResult result = device.Mix(output_span, REQUESTED_FRAME_COUNT);
    if (result.status != AudioStatus::CapacityExceeded) {
        return Fail("mix output capacity status was not reported");
    }

    if (result.frames_written != 0U) {
        return Fail("mix output capacity failure reported written frames");
    }

    if (result.failed_mix_requested_frame_count != REQUESTED_FRAME_COUNT) {
        return Fail("mix output capacity result missed requested frame count");
    }

    if (result.failed_mix_output_sample_capacity != OUTPUT_SAMPLE_CAPACITY) {
        return Fail("mix output capacity result missed output capacity");
    }

    if (result.failed_mix_required_sample_count != REQUIRED_SAMPLE_COUNT) {
        return Fail("mix output capacity result missed required sample count");
    }

    if (result.failed_mix_active_voice_count != 1U) {
        return Fail("mix output capacity result missed active voice count");
    }

    for (const std::int16_t sample : output) {
        if (sample != SENTINEL_SAMPLE) {
            return Fail("mix output capacity failure mutated output");
        }
    }

    const AudioDeviceSnapshot capacity_snapshot = device.Snapshot();
    if (capacity_snapshot.last_failed_mix_requested_frame_count != REQUESTED_FRAME_COUNT) {
        return Fail("mix output capacity snapshot missed requested frame count");
    }

    if (capacity_snapshot.last_failed_mix_output_sample_capacity != OUTPUT_SAMPLE_CAPACITY) {
        return Fail("mix output capacity snapshot missed output capacity");
    }

    if (capacity_snapshot.last_failed_mix_required_sample_count != REQUIRED_SAMPLE_COUNT) {
        return Fail("mix output capacity snapshot missed required sample count");
    }

    if (capacity_snapshot.last_failed_mix_active_voice_count != 1U) {
        return Fail("mix output capacity snapshot missed active voice count");
    }

    AudioSourceId invalid_source{};
    const std::array<std::int16_t, REQUIRED_SAMPLE_COUNT> invalid_source_samples{100, -100, 200, -200};
    std::span<const std::int16_t> invalid_source_span(invalid_source_samples.data(), invalid_source_samples.size());
    if (device.RegisterSyntheticSource(invalid_source_span, 0U, invalid_source) != AudioStatus::InvalidDescriptor) {
        return Fail("invalid source descriptor was accepted");
    }

    if (!MixOutputCapacityEntryIsClear(device.Snapshot())) {
        return Fail("invalid source descriptor kept mix output capacity entry");
    }

    AudioVoiceHandle second_voice{};
    if (!StartBasicVoice(device, second_voice)) {
        return Fail("second voice start failed");
    }

    output.fill(SENTINEL_SAMPLE);
    const AudioMixResult second_result = device.Mix(output_span, REQUESTED_FRAME_COUNT);
    if (second_result.status != AudioStatus::CapacityExceeded) {
        return Fail("second mix output capacity status was not reported");
    }

    std::array<std::int16_t, REQUIRED_SAMPLE_COUNT> full_output{};
    std::span<std::int16_t> full_output_span(full_output.data(), full_output.size());
    const AudioMixResult success_result = device.Mix(full_output_span, REQUESTED_FRAME_COUNT);
    if (success_result.status != AudioStatus::Success) {
        return Fail("mix after output capacity failure did not succeed");
    }

    if (!MixOutputCapacityEntryIsClear(success_result)) {
        return Fail("mix success result kept output capacity entry");
    }

    if (!MixOutputCapacityEntryIsClear(device.Snapshot())) {
        return Fail("mix success snapshot kept output capacity entry");
    }

    AudioVoiceHandle third_voice{};
    if (!StartBasicVoice(device, third_voice)) {
        return Fail("third voice start failed");
    }

    output.fill(SENTINEL_SAMPLE);
    const AudioMixResult third_result = device.Mix(output_span, REQUESTED_FRAME_COUNT);
    if (third_result.status != AudioStatus::CapacityExceeded) {
        return Fail("third mix output capacity status was not reported");
    }

    AudioSourceId fill_source{};
    if (!RegisterBasicSource(device, fill_source)) {
        return Fail("voice capacity fill source registration failed");
    }

    for (std::size_t active_voice_count = device.Snapshot().active_voice_count;
         active_voice_count < MAX_VOICES;
         ++active_voice_count) {
        AudioVoiceHandle fill_voice{};
        if (device.StartVoice(fill_source, MAX_Q15_GAIN, fill_voice) != AudioStatus::Success) {
            return Fail("voice capacity fill failed");
        }
    }

    AudioVoiceHandle overflow_voice{};
    if (device.StartVoice(fill_source, MAX_Q15_GAIN, overflow_voice) != AudioStatus::CapacityExceeded) {
        return Fail("voice capacity failure was not reported");
    }

    if (!MixOutputCapacityEntryIsClear(device.Snapshot())) {
        return Fail("voice capacity failure kept mix output capacity entry");
    }

    return 0;
}

int AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation() {
    TestAudioDevice device;
    const AudioDeviceSnapshot before_snapshot = device.Snapshot();
    AudioVoiceHandle voice{};

    if (device.StartVoice(AudioSourceId{0U, 1U}, MAX_Q15_GAIN, voice) != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized start voice did not return explicit status");
    }

    if (device.StopVoice(AudioVoiceHandle{0U, 1U}) != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized stop voice did not return explicit status");
    }

    std::array<std::int16_t, 4U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized mix did not return explicit status");
    }

    if (result.frames_written != 0U) {
        return Fail("uninitialized mix reported written frames");
    }

    for (const std::int16_t sample : output) {
        if (sample != SENTINEL_SAMPLE) {
            return Fail("uninitialized mix wrote output");
        }
    }

    if (!SnapshotsEqual(device.Snapshot(), before_snapshot)) {
        return Fail("uninitialized operations mutated counters");
    }

    return 0;
}

int AudioMixDoesNotGrowVoiceStorage() {
    AudioDeviceDesc desc{};
    desc.voice_capacity = 1U;

    TestAudioDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("minimal voice capacity device failed to initialize");
    }

    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const auto snapshot = device.Snapshot();
    if (snapshot.voice_storage_capacity_before_mix != snapshot.voice_storage_capacity_after_last_mix) {
        return Fail("voice storage capacity changed during mix");
    }

    if (snapshot.voice_capacity != desc.voice_capacity) {
        return Fail("logical voice capacity was unexpected");
    }

    if (snapshot.voice_storage_capacity_before_mix < MAX_VOICES) {
        return Fail("voice storage capacity was unexpected");
    }

    if (snapshot.voice_storage_capacity_before_mix == snapshot.voice_capacity) {
        return Fail("voice storage capacity reported logical slot count");
    }

    return 0;
}

int AudioDisabledDiagnosticsDoesNotChangeResults() {
    TestAudioDevice enabled_like_device = CreateInitializedDevice();
    TestAudioDevice disabled_like_device = CreateInitializedDevice();
    AudioVoiceHandle enabled_voice{};
    AudioVoiceHandle disabled_voice{};
    StartBasicVoice(enabled_like_device, enabled_voice);
    StartBasicVoice(disabled_like_device, disabled_voice);

    std::array<std::int16_t, 4U> enabled_output{};
    std::array<std::int16_t, 4U> disabled_output{};
    const AudioMixResult enabled_result = enabled_like_device.Mix(std::span<std::int16_t>(enabled_output.data(), enabled_output.size()), 2U);
    const AudioMixResult disabled_result = disabled_like_device.Mix(std::span<std::int16_t>(disabled_output.data(), disabled_output.size()), 2U);
    if (enabled_result.status != disabled_result.status) {
        return Fail("disabled diagnostics changed mix status");
    }

    if (!SamplesEqual(std::span<const std::int16_t>(enabled_output.data(), enabled_output.size()), std::span<const std::int16_t>(disabled_output.data(), disabled_output.size()))) {
        return Fail("disabled diagnostics changed output samples");
    }

    if (enabled_like_device.Snapshot().mixed_frame_count != disabled_like_device.Snapshot().mixed_frame_count) {
        return Fail("disabled diagnostics changed mixed frame count");
    }

    return 0;
}

int AudioNoDeviceCodecResourceScriptUiGameAdapterDependency() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("minimal audio path failed");
    }

    std::array<std::int16_t, 4U> output{};
    if (device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U).status != AudioStatus::Success) {
        return Fail("minimal mix path failed");
    }

    if (device.Capabilities().backend_kind != AudioBackendKind::Test) {
        return Fail("audio fixture left test backend scope");
    }

    return 0;
}

int AudioPcmSamplePacketCreatesQueriesAndReleasesMetadata() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 42U, packet)) {
        return Fail("pcm packet create failed");
    }

    AudioPcmSamplePacketRecord record{};
    if (device.QueryPcmSamplePacket(packet, record) != AudioStatus::Success) {
        return Fail("pcm packet query failed");
    }

    if (!record.is_active) {
        return Fail("pcm packet record was not active");
    }

    if (record.packet_id != 42U) {
        return Fail("pcm packet id changed");
    }

    if (record.frame_count != 2U) {
        return Fail("pcm packet frame count changed");
    }

    if (record.interleaved_sample_count != 4U) {
        return Fail("pcm packet sample count changed");
    }

    if (record.byte_count != 4U * sizeof(std::int16_t)) {
        return Fail("pcm packet byte count changed");
    }

    if (device.ReleasePcmSamplePacket(packet) != AudioStatus::Success) {
        return Fail("pcm packet release failed");
    }

    const AudioPcmSamplePacketSnapshot snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.active_packet_count != 0U) {
        return Fail("pcm packet release left active packet");
    }

    if (snapshot.created_packet_count != 1U || snapshot.queried_packet_count != 1U || snapshot.released_packet_count != 1U) {
        return Fail("pcm packet lifecycle counters were unexpected");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsDuplicatePacketIdWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 7U, packet)) {
        return Fail("baseline pcm packet create failed");
    }

    const AudioPcmSamplePacketRequest duplicate_request = BasicPcmSamplePacketRequest(7U);
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, duplicate_request, AudioStatus::InvalidDescriptor)) {
        return Fail("duplicate pcm packet id was not rejected without active mutation");
    }

    const AudioPcmSamplePacketSnapshot snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.duplicate_packet_rejected_count != 1U) {
        return Fail("duplicate pcm packet rejection counter was unexpected");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsUnsupportedFormatWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(8U);
    request.format = AudioSampleFormat::Unsupported;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::UnsupportedFormat)) {
        return Fail("unsupported pcm packet format was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsUnsupportedSampleRateWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(9U);
    request.sample_rate = SAMPLE_RATE - 1U;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::UnsupportedFormat)) {
        return Fail("unsupported pcm packet sample rate was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsUnsupportedChannelCountWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(10U);
    request.channel_count = CHANNEL_COUNT + 1U;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::UnsupportedFormat)) {
        return Fail("unsupported pcm packet channel count was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsZeroFrameCountWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(11U);
    request.frame_count = 0U;
    request.interleaved_sample_count = 0U;
    request.byte_count = 0U;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("zero frame pcm packet was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsSampleCountMismatchWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(12U);
    --request.interleaved_sample_count;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("pcm packet sample count mismatch was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsByteCountMismatchWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(13U);
    --request.byte_count;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("pcm packet byte count mismatch was not rejected without active mutation");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsCapacityOverflowWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle first_packet{};
    for (std::size_t index = 0U; index < MAX_PCM_SAMPLE_PACKETS; ++index) {
        AudioPcmSamplePacketHandle packet{};
        const std::uint32_t packet_id = static_cast<std::uint32_t>(index + 1U);
        if (!CreateBasicPcmSamplePacket(device, packet_id, packet)) {
            return Fail("pcm packet capacity setup failed");
        }

        if (index == 0U) {
            first_packet = packet;
        }
    }

    const AudioPcmSamplePacketSnapshot before_snapshot = device.PcmSamplePacketSnapshot();
    const AudioPcmSamplePacketRequest overflow_request = BasicPcmSamplePacketRequest(static_cast<std::uint32_t>(MAX_PCM_SAMPLE_PACKETS + 1U));
    AudioPcmSamplePacketHandle rejected_packet{777U, 888U};
    const AudioPcmSamplePacketHandle expected_rejected_packet = rejected_packet;
    const AudioStatus overflow_status = device.CreatePcmSamplePacket(overflow_request, rejected_packet);
    if (overflow_status != AudioStatus::CapacityExceeded) {
        return Fail("pcm packet capacity overflow was not rejected without active mutation");
    }

    if (rejected_packet.slot != expected_rejected_packet.slot ||
        rejected_packet.generation != expected_rejected_packet.generation) {
        return Fail("pcm packet capacity rejection mutated caller output handle");
    }

    AudioPcmSamplePacketSnapshot snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.active_packet_count != MAX_PCM_SAMPLE_PACKETS) {
        return Fail("pcm packet capacity rejection changed active count");
    }

    if (snapshot.created_packet_count != before_snapshot.created_packet_count) {
        return Fail("pcm packet capacity rejection changed created count");
    }

    if (snapshot.rejected_packet_count != before_snapshot.rejected_packet_count + 1U) {
        return Fail("pcm packet capacity rejection did not increment rejected count");
    }

    if (snapshot.capacity_rejected_count != 1U) {
        return Fail("pcm packet capacity rejection counter was unexpected");
    }

    const std::size_t required_packet_count = before_snapshot.active_packet_count + 1U;
    if (snapshot.last_required_packet_count != required_packet_count) {
        return Fail("pcm packet capacity rejection missed required packet count");
    }

    if (snapshot.last_failed_packet_capacity != MAX_PCM_SAMPLE_PACKETS) {
        return Fail("pcm packet capacity rejection missed packet capacity");
    }

    if (snapshot.last_failed_active_packet_count != before_snapshot.active_packet_count) {
        return Fail("pcm packet capacity rejection missed active packet count");
    }

    if (snapshot.last_failed_packet_id != overflow_request.packet_id) {
        return Fail("pcm packet capacity rejection missed failed packet id");
    }

    if (snapshot.last_failed_frame_count != overflow_request.frame_count) {
        return Fail("pcm packet capacity rejection missed failed frame count");
    }

    if (snapshot.last_failed_sample_rate != overflow_request.sample_rate) {
        return Fail("pcm packet capacity rejection missed failed sample rate");
    }

    if (snapshot.last_failed_channel_count != overflow_request.channel_count) {
        return Fail("pcm packet capacity rejection missed failed channel count");
    }

    if (snapshot.last_failed_format != overflow_request.format) {
        return Fail("pcm packet capacity rejection missed failed format");
    }

    AudioPcmSamplePacketRequest invalid_request = BasicPcmSamplePacketRequest(0U);
    AudioPcmSamplePacketHandle invalid_packet{555U, 666U};
    if (device.CreatePcmSamplePacket(invalid_request, invalid_packet) != AudioStatus::InvalidDescriptor) {
        return Fail("pcm packet invalid descriptor did not reject after capacity failure");
    }

    snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.last_required_packet_count != 0U) {
        return Fail("pcm packet invalid descriptor kept required packet count");
    }

    if (snapshot.last_failed_packet_id != 0U ||
        snapshot.last_failed_packet_capacity != 0U ||
        snapshot.last_failed_active_packet_count != 0U ||
        snapshot.last_failed_frame_count != 0U ||
        snapshot.last_failed_sample_rate != 0U ||
        snapshot.last_failed_channel_count != 0U ||
        snapshot.last_failed_format != AudioSampleFormat::Signed16) {
        return Fail("pcm packet invalid descriptor kept capacity failure identity");
    }

    const AudioPcmSamplePacketRequest second_overflow_request = BasicPcmSamplePacketRequest(static_cast<std::uint32_t>(MAX_PCM_SAMPLE_PACKETS + 2U));
    AudioPcmSamplePacketHandle second_rejected_packet{};
    if (device.CreatePcmSamplePacket(second_overflow_request, second_rejected_packet) != AudioStatus::CapacityExceeded) {
        return Fail("pcm packet second capacity overflow was not rejected");
    }

    AudioPcmSamplePacketRecord record{};
    if (device.QueryPcmSamplePacket(first_packet, record) != AudioStatus::Success) {
        return Fail("pcm packet query did not clear capacity failure setup");
    }

    snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.last_required_packet_count != 0U) {
        return Fail("pcm packet query success kept required packet count");
    }

    if (snapshot.last_failed_packet_id != 0U ||
        snapshot.last_failed_packet_capacity != 0U ||
        snapshot.last_failed_active_packet_count != 0U ||
        snapshot.last_failed_frame_count != 0U ||
        snapshot.last_failed_sample_rate != 0U ||
        snapshot.last_failed_channel_count != 0U ||
        snapshot.last_failed_format != AudioSampleFormat::Signed16) {
        return Fail("pcm packet query success kept capacity failure identity");
    }

    return 0;
}

int AudioPcmSamplePacketRejectsStaleHandleWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 14U, packet)) {
        return Fail("stale pcm packet setup failed");
    }

    if (device.ReleasePcmSamplePacket(packet) != AudioStatus::Success) {
        return Fail("stale pcm packet release setup failed");
    }

    const AudioPcmSamplePacketSnapshot before_snapshot = device.PcmSamplePacketSnapshot();
    AudioPcmSamplePacketRecord record{};
    if (device.QueryPcmSamplePacket(packet, record) != AudioStatus::InvalidHandle) {
        return Fail("stale pcm packet query did not return invalid handle");
    }

    if (device.ReleasePcmSamplePacket(packet) != AudioStatus::InvalidHandle) {
        return Fail("stale pcm packet release did not return invalid handle");
    }

    const AudioPcmSamplePacketSnapshot after_snapshot = device.PcmSamplePacketSnapshot();
    if (after_snapshot.active_packet_count != before_snapshot.active_packet_count) {
        return Fail("stale pcm packet operation changed active count");
    }

    if (after_snapshot.stale_packet_rejected_count != before_snapshot.stale_packet_rejected_count + 2U) {
        return Fail("stale pcm packet rejection counter was unexpected");
    }

    return 0;
}

int AudioPcmSamplePacketSnapshotTracksCounters() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle first_packet{};
    AudioPcmSamplePacketHandle second_packet{};
    if (!CreateBasicPcmSamplePacket(device, 21U, first_packet)) {
        return Fail("first pcm packet create failed");
    }

    if (!CreateBasicPcmSamplePacket(device, 22U, second_packet)) {
        return Fail("second pcm packet create failed");
    }

    AudioPcmSamplePacketRecord record{};
    if (device.QueryPcmSamplePacket(first_packet, record) != AudioStatus::Success) {
        return Fail("pcm packet query for counter test failed");
    }

    if (device.ReleasePcmSamplePacket(second_packet) != AudioStatus::Success) {
        return Fail("pcm packet release for counter test failed");
    }

    const AudioPcmSamplePacketRequest duplicate_request = BasicPcmSamplePacketRequest(21U);
    AudioPcmSamplePacketHandle rejected_packet{};
    if (device.CreatePcmSamplePacket(duplicate_request, rejected_packet) != AudioStatus::InvalidDescriptor) {
        return Fail("pcm packet duplicate for counter test was not rejected");
    }

    const AudioPcmSamplePacketSnapshot snapshot = device.PcmSamplePacketSnapshot();
    if (snapshot.packet_capacity != MAX_PCM_SAMPLE_PACKETS) {
        return Fail("pcm packet snapshot capacity was unexpected");
    }

    if (snapshot.active_packet_count != 1U) {
        return Fail("pcm packet snapshot active count was unexpected");
    }

    if (snapshot.created_packet_count != 2U || snapshot.queried_packet_count != 1U || snapshot.released_packet_count != 1U) {
        return Fail("pcm packet snapshot lifecycle counters were unexpected");
    }

    if (snapshot.rejected_packet_count != 1U || snapshot.duplicate_packet_rejected_count != 1U) {
        return Fail("pcm packet snapshot rejection counters were unexpected");
    }

    if (snapshot.last_status != AudioStatus::InvalidDescriptor) {
        return Fail("pcm packet snapshot last status was unexpected");
    }

    if (snapshot.last_operation != AudioPcmSamplePacketOperation::Create) {
        return Fail("pcm packet snapshot last operation was unexpected");
    }

    return 0;
}

int AudioPcmSamplePacketRejectionsDoNotMutateSourceVoiceState() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("pcm packet boundary setup voice failed");
    }

    const AudioDeviceSnapshot before_device_snapshot = device.Snapshot();
    AudioPcmSamplePacketRequest request = BasicPcmSamplePacketRequest(31U);
    request.byte_count = 1U;
    if (!ExpectPcmCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("pcm packet boundary rejection failed");
    }

    const AudioDeviceSnapshot after_device_snapshot = device.Snapshot();
    if (after_device_snapshot.source_count != before_device_snapshot.source_count) {
        return Fail("pcm packet rejection changed source count");
    }

    if (after_device_snapshot.active_voice_count != before_device_snapshot.active_voice_count) {
        return Fail("pcm packet rejection changed voice count");
    }

    if (after_device_snapshot.registered_source_count != before_device_snapshot.registered_source_count) {
        return Fail("pcm packet rejection changed registered source count");
    }

    if (after_device_snapshot.started_voice_count != before_device_snapshot.started_voice_count) {
        return Fail("pcm packet rejection changed started voice count");
    }

    return 0;
}

int AudioPcmSamplePacketPublicContractsArePlainValues() {
    if (!std::is_standard_layout_v<AudioPcmSamplePacketHandle>) {
        return Fail("pcm packet handle was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmSamplePacketHandle>) {
        return Fail("pcm packet handle was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmSamplePacketRequest>) {
        return Fail("pcm packet request was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmSamplePacketRequest>) {
        return Fail("pcm packet request was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmSamplePacketRecord>) {
        return Fail("pcm packet record was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmSamplePacketRecord>) {
        return Fail("pcm packet record was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmSamplePacketSnapshot>) {
        return Fail("pcm packet snapshot was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmSamplePacketSnapshot>) {
        return Fail("pcm packet snapshot was not trivially copyable");
    }

    if (!std::is_enum_v<AudioPcmSamplePacketOperation>) {
        return Fail("pcm packet operation was not an enum");
    }

    return 0;
}

int AudioPcmStreamQueueQueuesDrainsAndReleasesMetadata() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 101U, packet)) {
        return Fail("stream queue packet setup failed");
    }

    const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(201U, packet, 101U);
    AudioPcmStreamQueueHandle queue{};
    if (device.CreatePcmStreamQueue(request, queue) != AudioStatus::Success) {
        return Fail("stream queue create failed");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue query failed");
    }

    if (!record.is_active) {
        return Fail("stream queue record was not active");
    }

    if (record.queue_id != 201U || record.packet_id != 101U) {
        return Fail("stream queue ids changed");
    }

    if (record.frame_count != 2U || record.remaining_frame_count != 2U) {
        return Fail("stream queue frame counts changed");
    }

    std::array<AudioPcmStreamQueueChunk, 2U> chunks{};
    std::size_t chunk_count = 0U;
    if (device.DrainPcmStreamQueue(queue, std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()), chunk_count) != AudioStatus::Success) {
        return Fail("stream queue drain failed");
    }

    if (chunk_count != 2U) {
        return Fail("stream queue chunk count changed");
    }

    if (chunks[0].first_frame != 0U || chunks[0].frame_count != 1U || chunks[0].first_interleaved_sample != 0U) {
        return Fail("stream queue first chunk shape changed");
    }

    if (chunks[1].first_frame != 1U || chunks[1].frame_count != 1U || chunks[1].first_interleaved_sample != CHANNEL_COUNT) {
        return Fail("stream queue second chunk shape changed");
    }

    if (chunks[0].is_final_chunk || !chunks[1].is_final_chunk) {
        return Fail("stream queue final chunk markers changed");
    }

    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue post-drain query failed");
    }

    if (record.drained_frame_count != 2U || record.remaining_frame_count != 0U) {
        return Fail("stream queue drain counters changed");
    }

    if (device.ReleasePcmStreamQueue(queue) != AudioStatus::Success) {
        return Fail("stream queue release failed");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.active_queue_count != 0U) {
        return Fail("stream queue release left active queue");
    }

    if (snapshot.created_queue_count != 1U || snapshot.drained_descriptor_count != 2U || snapshot.released_queue_count != 1U) {
        return Fail("stream queue lifecycle counters were unexpected");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsInvalidPacketHandleWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(202U, AudioPcmSamplePacketHandle{}, 102U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidHandle)) {
        return Fail("invalid packet handle was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.packet_rejected_count != 1U) {
        return Fail("invalid packet rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsReleasedPacketHandleWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 103U, packet)) {
        return Fail("released packet setup failed");
    }

    if (device.ReleasePcmSamplePacket(packet) != AudioStatus::Success) {
        return Fail("released packet setup release failed");
    }

    const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(203U, packet, 103U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidHandle)) {
        return Fail("released packet handle was not rejected without queue mutation");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsPacketIdMismatchWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 104U, packet)) {
        return Fail("packet id mismatch setup failed");
    }

    const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(204U, packet, 105U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("packet id mismatch was not rejected without queue mutation");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsDuplicateQueueIdWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreateBasicPcmStreamQueue(device, 106U, 206U, queue)) {
        return Fail("duplicate stream queue setup failed");
    }

    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 107U, packet)) {
        return Fail("duplicate stream queue second packet setup failed");
    }

    const AudioPcmStreamQueueRequest duplicate_request = BasicPcmStreamQueueRequest(206U, packet, 107U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, duplicate_request, AudioStatus::InvalidDescriptor)) {
        return Fail("duplicate stream queue id was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.duplicate_queue_rejected_count != 1U) {
        return Fail("duplicate stream queue rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsFrameRangeOverflowWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 108U, packet)) {
        return Fail("frame range setup failed");
    }

    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(208U, packet, 108U);
    request.first_frame = 1U;
    request.frame_count = 2U;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("frame range overflow was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.range_rejected_count != 1U) {
        return Fail("frame range rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsSampleCountMismatchWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 109U, packet)) {
        return Fail("sample count setup failed");
    }

    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(209U, packet, 109U);
    --request.interleaved_sample_count;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("stream queue sample mismatch was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.sample_count_rejected_count != 1U) {
        return Fail("stream queue sample rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsByteCountMismatchWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 110U, packet)) {
        return Fail("byte count setup failed");
    }

    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(210U, packet, 110U);
    --request.byte_count;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("stream queue byte mismatch was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.byte_count_rejected_count != 1U) {
        return Fail("stream queue byte rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsInvalidChunkFrameCountWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 111U, packet)) {
        return Fail("chunk count setup failed");
    }

    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(211U, packet, 111U);
    request.chunk_frame_count = 0U;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("zero chunk frame count was not rejected without queue mutation");
    }

    request = BasicPcmStreamQueueRequest(212U, packet, 111U);
    request.chunk_frame_count = MAX_PCM_STREAM_CHUNK_FRAMES + 1U;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::CapacityExceeded)) {
        return Fail("oversized chunk frame count was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.chunk_rejected_count != 2U) {
        return Fail("stream queue chunk rejection counter changed");
    }

    if (snapshot.last_required_queue_count != 0U) {
        return Fail("stream queue chunk rejection reported required queue count");
    }

    if (snapshot.last_required_output_chunk_count != 0U) {
        return Fail("stream queue chunk rejection reported required output count");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsCapacityOverflowWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 112U, packet)) {
        return Fail("stream queue capacity packet setup failed");
    }

    for (std::size_t index = 0U; index < MAX_PCM_STREAM_QUEUES; ++index) {
        const std::uint32_t queue_id = static_cast<std::uint32_t>(index + 300U);
        const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(queue_id, packet, 112U);
        AudioPcmStreamQueueHandle queue{};
        if (device.CreatePcmStreamQueue(request, queue) != AudioStatus::Success) {
            return Fail("stream queue capacity setup failed");
        }
    }

    const AudioPcmStreamQueueSnapshot before_snapshot = device.PcmStreamQueueSnapshot();
    const AudioPcmStreamQueueRequest overflow_request = BasicPcmStreamQueueRequest(399U, packet, 112U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, overflow_request, AudioStatus::CapacityExceeded)) {
        return Fail("stream queue capacity overflow was not rejected without queue mutation");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.active_queue_count != MAX_PCM_STREAM_QUEUES) {
        return Fail("stream queue capacity rejection changed active count");
    }

    if (snapshot.capacity_rejected_count != 1U) {
        return Fail("stream queue capacity rejection counter changed");
    }

    const std::size_t required_queue_count = before_snapshot.active_queue_count + 1U;
    if (snapshot.last_required_queue_count != required_queue_count) {
        return Fail("stream queue capacity rejection missed required queue count");
    }

    if (snapshot.last_required_output_chunk_count != 0U) {
        return Fail("stream queue capacity rejection reported output chunk count");
    }

    return 0;
}

int AudioPcmStreamQueueCapacityOverflowReportsRejectedEntry() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 132U, packet)) {
        return Fail("stream queue capacity entry packet setup failed");
    }

    AudioPcmStreamQueueHandle first_queue{};
    for (std::size_t index = 0U; index < MAX_PCM_STREAM_QUEUES; ++index) {
        const std::uint32_t queue_id = static_cast<std::uint32_t>(index + 420U);
        const AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(queue_id, packet, 132U);
        AudioPcmStreamQueueHandle queue{};
        if (device.CreatePcmStreamQueue(request, queue) != AudioStatus::Success) {
            return Fail("stream queue capacity entry setup failed");
        }

        if (index == 0U) {
            first_queue = queue;
        }
    }

    const AudioPcmStreamQueueSnapshot before_snapshot = device.PcmStreamQueueSnapshot();
    const AudioPcmStreamQueueRequest overflow_request = BasicPcmStreamQueueRequest(499U, packet, 132U);
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, overflow_request, AudioStatus::CapacityExceeded)) {
        return Fail("stream queue capacity entry overflow was not rejected without queue mutation");
    }

    const std::size_t required_queue_count = before_snapshot.active_queue_count + 1U;
    const AudioPcmStreamQueueSnapshot overflow_snapshot = device.PcmStreamQueueSnapshot();
    if (!PcmStreamQueueCapacityEntryMatches(overflow_snapshot, overflow_request, before_snapshot.active_queue_count, required_queue_count)) {
        return Fail("stream queue capacity entry snapshot did not match rejected request");
    }

    const AudioPcmStreamQueueRequest invalid_request = BasicPcmStreamQueueRequest(0U, packet, 132U);
    AudioPcmStreamQueueHandle rejected_queue{};
    if (device.CreatePcmStreamQueue(invalid_request, rejected_queue) != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue invalid request did not clear stale capacity entry");
    }

    const AudioPcmStreamQueueSnapshot invalid_snapshot = device.PcmStreamQueueSnapshot();
    if (!PcmStreamQueueCapacityEntryIsClear(invalid_snapshot)) {
        return Fail("stream queue invalid request kept stale capacity entry");
    }

    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, overflow_request, AudioStatus::CapacityExceeded)) {
        return Fail("stream queue second capacity overflow was not rejected");
    }

    if (device.ReleasePcmStreamQueue(first_queue) != AudioStatus::Success) {
        return Fail("stream queue release did not clear stale capacity entry");
    }

    const AudioPcmStreamQueueSnapshot release_snapshot = device.PcmStreamQueueSnapshot();
    if (!PcmStreamQueueCapacityEntryIsClear(release_snapshot)) {
        return Fail("stream queue release kept stale capacity entry");
    }

    AudioPcmStreamQueueHandle replacement_queue{};
    if (device.CreatePcmStreamQueue(overflow_request, replacement_queue) != AudioStatus::Success) {
        return Fail("stream queue success did not follow capacity entry clear");
    }

    const AudioPcmStreamQueueSnapshot success_snapshot = device.PcmStreamQueueSnapshot();
    if (!PcmStreamQueueCapacityEntryIsClear(success_snapshot)) {
        return Fail("stream queue success kept stale capacity entry");
    }

    return 0;
}

int AudioPcmStreamQueueDrainRejectsSmallOutputWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreateBasicPcmStreamQueue(device, 113U, 213U, queue)) {
        return Fail("stream queue small output setup failed");
    }

    AudioPcmStreamQueueRecord before_record{};
    if (device.QueryPcmStreamQueue(queue, before_record) != AudioStatus::Success) {
        return Fail("stream queue small output pre-query failed");
    }

    std::array<AudioPcmStreamQueueChunk, 1U> chunks{};
    std::size_t chunk_count = 7U;
    if (device.DrainPcmStreamQueue(queue, std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()), chunk_count) != AudioStatus::CapacityExceeded) {
        return Fail("stream queue small output was not rejected");
    }

    const AudioPcmStreamQueueSnapshot failure_snapshot = device.PcmStreamQueueSnapshot();
    AudioPcmStreamQueueRecord after_record{};
    if (device.QueryPcmStreamQueue(queue, after_record) != AudioStatus::Success) {
        return Fail("stream queue small output post-query failed");
    }

    if (chunk_count != 0U) {
        return Fail("stream queue small output wrote a descriptor count");
    }

    if (after_record.drained_frame_count != before_record.drained_frame_count) {
        return Fail("stream queue small output changed drained frames");
    }

    if (after_record.remaining_frame_count != before_record.remaining_frame_count) {
        return Fail("stream queue small output changed remaining frames");
    }

    const AudioPcmStreamQueueSnapshot after_query_snapshot = device.PcmStreamQueueSnapshot();
    if (after_query_snapshot.output_capacity_rejected_count != 1U) {
        return Fail("stream queue small output rejection counter changed");
    }

    const std::size_t expected_chunk_count =
        (before_record.remaining_frame_count + before_record.chunk_frame_count - 1U) / before_record.chunk_frame_count;
    if (failure_snapshot.last_required_output_chunk_count != expected_chunk_count) {
        return Fail("stream queue small output missed required chunk count");
    }

    if (failure_snapshot.last_required_queue_count != 0U) {
        return Fail("stream queue small output reported required queue count");
    }

    return 0;
}

int AudioPcmStreamQueueRejectsStaleHandleWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreateBasicPcmStreamQueue(device, 114U, 214U, queue)) {
        return Fail("stream queue stale setup failed");
    }

    if (device.ReleasePcmStreamQueue(queue) != AudioStatus::Success) {
        return Fail("stream queue stale release setup failed");
    }

    const AudioPcmStreamQueueSnapshot before_snapshot = device.PcmStreamQueueSnapshot();
    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::InvalidHandle) {
        return Fail("stale stream queue query did not return invalid handle");
    }

    std::array<AudioPcmStreamQueueChunk, 1U> chunks{};
    std::size_t chunk_count = 0U;
    if (device.DrainPcmStreamQueue(queue, std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()), chunk_count) != AudioStatus::InvalidHandle) {
        return Fail("stale stream queue drain did not return invalid handle");
    }

    if (device.ReleasePcmStreamQueue(queue) != AudioStatus::InvalidHandle) {
        return Fail("stale stream queue release did not return invalid handle");
    }

    const AudioPcmStreamQueueSnapshot after_snapshot = device.PcmStreamQueueSnapshot();
    if (after_snapshot.active_queue_count != before_snapshot.active_queue_count) {
        return Fail("stale stream queue operation changed active count");
    }

    if (after_snapshot.stale_queue_rejected_count != before_snapshot.stale_queue_rejected_count + 3U) {
        return Fail("stale stream queue rejection counter changed");
    }

    return 0;
}

int AudioPcmStreamQueueRejectionsDoNotMutatePacketSourceVoiceCallbackState() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("stream queue no-mutation voice setup failed");
    }

    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 115U, packet)) {
        return Fail("stream queue no-mutation packet setup failed");
    }

    const AudioDeviceSnapshot before_device_snapshot = device.Snapshot();
    const AudioPcmSamplePacketSnapshot before_packet_snapshot = device.PcmSamplePacketSnapshot();
    AudioPcmStreamQueueRequest request = BasicPcmStreamQueueRequest(215U, packet, 115U);
    --request.byte_count;
    if (!ExpectPcmStreamQueueCreateRejectedWithoutActiveMutation(device, request, AudioStatus::InvalidDescriptor)) {
        return Fail("stream queue no-mutation rejection failed");
    }

    const AudioDeviceSnapshot after_device_snapshot = device.Snapshot();
    if (after_device_snapshot.source_count != before_device_snapshot.source_count) {
        return Fail("stream queue rejection changed source count");
    }

    if (after_device_snapshot.active_voice_count != before_device_snapshot.active_voice_count) {
        return Fail("stream queue rejection changed voice count");
    }

    if (after_device_snapshot.registered_source_count != before_device_snapshot.registered_source_count) {
        return Fail("stream queue rejection changed registered source count");
    }

    if (after_device_snapshot.started_voice_count != before_device_snapshot.started_voice_count) {
        return Fail("stream queue rejection changed started voice count");
    }

    const AudioPcmSamplePacketSnapshot after_packet_snapshot = device.PcmSamplePacketSnapshot();
    if (after_packet_snapshot.active_packet_count != before_packet_snapshot.active_packet_count) {
        return Fail("stream queue rejection changed active packet count");
    }

    if (after_packet_snapshot.created_packet_count != before_packet_snapshot.created_packet_count) {
        return Fail("stream queue rejection changed packet create count");
    }

    AudioCallbackDevice callback_device;
    if (callback_device.Snapshot().submitted_buffer_count != 0U) {
        return Fail("stream queue rejection touched callback state");
    }

    return 0;
}

int AudioPcmStreamQueueSnapshotTracksCounters() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle first_queue{};
    AudioPcmStreamQueueHandle second_queue{};
    if (!CreateBasicPcmStreamQueue(device, 116U, 216U, first_queue)) {
        return Fail("first stream queue create failed");
    }

    if (!CreateBasicPcmStreamQueue(device, 117U, 217U, second_queue)) {
        return Fail("second stream queue create failed");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(first_queue, record) != AudioStatus::Success) {
        return Fail("stream queue counter query failed");
    }

    std::array<AudioPcmStreamQueueChunk, 2U> chunks{};
    std::size_t chunk_count = 0U;
    if (device.DrainPcmStreamQueue(first_queue, std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()), chunk_count) != AudioStatus::Success) {
        return Fail("stream queue counter drain failed");
    }

    if (device.ReleasePcmStreamQueue(second_queue) != AudioStatus::Success) {
        return Fail("stream queue counter release failed");
    }

    AudioPcmSamplePacketHandle packet{};
    if (!CreateBasicPcmSamplePacket(device, 118U, packet)) {
        return Fail("stream queue counter duplicate setup failed");
    }

    const AudioPcmStreamQueueRequest duplicate_request = BasicPcmStreamQueueRequest(216U, packet, 118U);
    AudioPcmStreamQueueHandle rejected_queue{};
    if (device.CreatePcmStreamQueue(duplicate_request, rejected_queue) != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue duplicate for counter test was not rejected");
    }

    const AudioPcmStreamQueueSnapshot snapshot = device.PcmStreamQueueSnapshot();
    if (snapshot.queue_capacity != MAX_PCM_STREAM_QUEUES) {
        return Fail("stream queue snapshot capacity changed");
    }

    if (snapshot.active_queue_count != 1U) {
        return Fail("stream queue snapshot active count changed");
    }

    if (snapshot.created_queue_count != 2U || snapshot.queried_queue_count != 1U || snapshot.released_queue_count != 1U) {
        return Fail("stream queue snapshot lifecycle counters changed");
    }

    if (snapshot.drained_descriptor_count != 2U || snapshot.drained_frame_count != 2U) {
        return Fail("stream queue snapshot drain counters changed");
    }

    if (snapshot.rejected_queue_count != 1U || snapshot.duplicate_queue_rejected_count != 1U) {
        return Fail("stream queue snapshot rejection counters changed");
    }

    if (snapshot.last_status != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue snapshot last status changed");
    }

    if (snapshot.last_operation != AudioPcmStreamQueueOperation::Create) {
        return Fail("stream queue snapshot last operation changed");
    }

    return 0;
}

int AudioPcmStreamQueuePublicContractsArePlainValues() {
    if (!std::is_standard_layout_v<AudioPcmStreamQueueHandle>) {
        return Fail("stream queue handle was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueHandle>) {
        return Fail("stream queue handle was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmStreamQueueRequest>) {
        return Fail("stream queue request was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueRequest>) {
        return Fail("stream queue request was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmStreamQueueRecord>) {
        return Fail("stream queue record was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueRecord>) {
        return Fail("stream queue record was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmStreamQueueChunk>) {
        return Fail("stream queue chunk was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueChunk>) {
        return Fail("stream queue chunk was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmStreamQueueSnapshot>) {
        return Fail("stream queue snapshot was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueSnapshot>) {
        return Fail("stream queue snapshot was not trivially copyable");
    }

    if (!std::is_enum_v<AudioPcmStreamQueueOperation>) {
        return Fail("stream queue operation was not an enum");
    }

    if (!std::is_enum_v<AudioPcmStreamQueueStatus>) {
        return Fail("stream queue status was not an enum");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackSubmitsDrainedChunks() {
    constexpr std::size_t FRAMES_PER_BUFFER = 16U;
    constexpr std::size_t FRAME_COUNT = FRAMES_PER_BUFFER * 2U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreatePcmStreamQueueForFrames(device, 119U, 219U, FRAME_COUNT, FRAMES_PER_BUFFER, queue)) {
        return Fail("stream queue callback setup failed");
    }

    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    for (std::size_t index = 0U; index < samples.size(); ++index) {
        samples[index] = static_cast<std::int16_t>(index + 1U);
    }

    FakeCallbackSubmitContext context{};
    context.expected_frames_per_buffer = FRAMES_PER_BUFFER;
    const AudioPcmStreamQueueCallbackSubmitter submitter = FakeCallbackSubmitter(&context, FRAMES_PER_BUFFER);
    std::array<AudioPcmStreamQueueChunk, 2U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    const AudioStatus status = SubmitPcmStreamQueueToCallback(&device,
                                                              queue,
                                                              submitter,
                                                              std::span<const std::int16_t>(samples.data(), samples.size()),
                                                              std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                                              &result);
    if (status != AudioStatus::Success) {
        return Fail("stream queue callback submit failed");
    }

    if (result.status != AudioStatus::Success) {
        return Fail("stream queue callback result status failed");
    }

    if (result.drained_chunk_count != 2U || result.submitted_chunk_count != 2U) {
        return Fail("stream queue callback chunk counts changed");
    }

    if (result.submitted_frame_count != FRAME_COUNT || result.submitted_sample_count != SAMPLE_COUNT) {
        return Fail("stream queue callback submitted sample counts changed");
    }

    if (!result.reached_final_chunk || result.last_submitted_first_frame != FRAMES_PER_BUFFER) {
        return Fail("stream queue callback final chunk tracking changed");
    }

    if (context.attempted_call_count != 2U || context.submitted_call_count != 2U) {
        return Fail("stream queue callback submit calls changed");
    }

    if (context.submitted_frame_count != FRAME_COUNT || context.submitted_sample_count != SAMPLE_COUNT) {
        return Fail("stream queue callback fake submit counts changed");
    }

    if (context.first_submitted_sample != 1 || context.last_submitted_sample != static_cast<std::int16_t>(SAMPLE_COUNT)) {
        return Fail("stream queue callback sample range changed");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue callback post-query failed");
    }

    if (record.remaining_frame_count != 0U || record.drained_frame_count != FRAME_COUNT) {
        return Fail("stream queue callback did not drain queue");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackRejectsPartialFinalChunkWithoutDrain() {
    constexpr std::size_t FRAMES_PER_BUFFER = 16U;
    constexpr std::size_t FRAME_COUNT = 24U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreatePcmStreamQueueForFrames(device, 120U, 220U, FRAME_COUNT, FRAMES_PER_BUFFER, queue)) {
        return Fail("stream queue callback partial setup failed");
    }

    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    FakeCallbackSubmitContext context{};
    context.expected_frames_per_buffer = FRAMES_PER_BUFFER;
    const AudioPcmStreamQueueCallbackSubmitter submitter = FakeCallbackSubmitter(&context, FRAMES_PER_BUFFER);
    std::array<AudioPcmStreamQueueChunk, 2U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    const AudioStatus status = SubmitPcmStreamQueueToCallback(&device,
                                                              queue,
                                                              submitter,
                                                              std::span<const std::int16_t>(samples.data(), samples.size()),
                                                              std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                                              &result);
    if (status != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue callback partial chunk was not rejected");
    }

    if (context.attempted_call_count != 0U) {
        return Fail("stream queue callback partial rejection submitted data");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue callback partial post-query failed");
    }

    if (record.remaining_frame_count != FRAME_COUNT || record.drained_frame_count != 0U) {
        return Fail("stream queue callback partial rejection drained queue");
    }

    if (result.status != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue callback partial result status changed");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackRejectsSmallSampleSpanWithoutDrain() {
    constexpr std::size_t FRAMES_PER_BUFFER = 16U;
    constexpr std::size_t FRAME_COUNT = 16U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreatePcmStreamQueueForFrames(device, 121U, 221U, FRAME_COUNT, FRAMES_PER_BUFFER, queue)) {
        return Fail("stream queue callback small span setup failed");
    }

    std::array<std::int16_t, SAMPLE_COUNT - 1U> samples{};
    FakeCallbackSubmitContext context{};
    context.expected_frames_per_buffer = FRAMES_PER_BUFFER;
    const AudioPcmStreamQueueCallbackSubmitter submitter = FakeCallbackSubmitter(&context, FRAMES_PER_BUFFER);
    std::array<AudioPcmStreamQueueChunk, 1U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    const AudioStatus status = SubmitPcmStreamQueueToCallback(&device,
                                                              queue,
                                                              submitter,
                                                              std::span<const std::int16_t>(samples.data(), samples.size()),
                                                              std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                                              &result);
    if (status != AudioStatus::InvalidDescriptor) {
        return Fail("stream queue callback small span was not rejected");
    }

    if (context.attempted_call_count != 0U) {
        return Fail("stream queue callback small span submitted data");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue callback small span post-query failed");
    }

    if (record.remaining_frame_count != FRAME_COUNT || record.drained_frame_count != 0U) {
        return Fail("stream queue callback small span drained queue");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackStopsOnSubmitFailure() {
    constexpr std::size_t FRAMES_PER_BUFFER = 16U;
    constexpr std::size_t FRAME_COUNT = FRAMES_PER_BUFFER * 2U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreatePcmStreamQueueForFrames(device, 122U, 222U, FRAME_COUNT, FRAMES_PER_BUFFER, queue)) {
        return Fail("stream queue callback failure setup failed");
    }

    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    FakeCallbackSubmitContext context{};
    context.expected_frames_per_buffer = FRAMES_PER_BUFFER;
    context.fail_on_call = 2U;
    context.failure_status = AudioStatus::CapacityExceeded;
    const AudioPcmStreamQueueCallbackSubmitter submitter = FakeCallbackSubmitter(&context, FRAMES_PER_BUFFER);
    std::array<AudioPcmStreamQueueChunk, 2U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    const AudioStatus status = SubmitPcmStreamQueueToCallback(&device,
                                                              queue,
                                                              submitter,
                                                              std::span<const std::int16_t>(samples.data(), samples.size()),
                                                              std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                                              &result);
    if (status != AudioStatus::CapacityExceeded) {
        return Fail("stream queue callback submit failure status changed");
    }

    if (context.attempted_call_count != 2U || context.submitted_call_count != 1U) {
        return Fail("stream queue callback submit failure call counts changed");
    }

    if (result.drained_chunk_count != 2U || result.submitted_chunk_count != 1U) {
        return Fail("stream queue callback submit failure result counts changed");
    }

    if (result.status != AudioStatus::CapacityExceeded) {
        return Fail("stream queue callback submit failure result status changed");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue callback failure post-query failed");
    }

    if (record.remaining_frame_count != 0U) {
        return Fail("stream queue callback failure did not drain descriptors");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackRejectsUnstartedCallbackWithoutDrain() {
    constexpr std::size_t FRAMES_PER_BUFFER = 16U;
    constexpr std::size_t FRAME_COUNT = 16U;
    constexpr std::size_t SAMPLE_COUNT = FRAME_COUNT * CHANNEL_COUNT;
    TestAudioDevice device = CreateInitializedDevice();
    AudioPcmStreamQueueHandle queue{};
    if (!CreatePcmStreamQueueForFrames(device, 123U, 223U, FRAME_COUNT, FRAMES_PER_BUFFER, queue)) {
        return Fail("stream queue callback unstarted setup failed");
    }

    AudioCallbackDevice callback_device;
    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    std::array<AudioPcmStreamQueueChunk, 1U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    const AudioStatus status = SubmitPcmStreamQueueToCallback(&device,
                                                              queue,
                                                              &callback_device,
                                                              std::span<const std::int16_t>(samples.data(), samples.size()),
                                                              std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                                              &result);
    if (status != AudioStatus::NotStarted) {
        return Fail("stream queue callback unstarted status changed");
    }

    AudioPcmStreamQueueRecord record{};
    if (device.QueryPcmStreamQueue(queue, record) != AudioStatus::Success) {
        return Fail("stream queue callback unstarted post-query failed");
    }

    if (record.remaining_frame_count != FRAME_COUNT || record.drained_frame_count != 0U) {
        return Fail("stream queue callback unstarted drained queue");
    }

    return 0;
}

int AudioPcmStreamQueueCallbackPublicContractsArePlainValues() {
    if (!std::is_standard_layout_v<AudioPcmStreamQueueCallbackResult>) {
        return Fail("stream queue callback result was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueCallbackResult>) {
        return Fail("stream queue callback result was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioPcmStreamQueueCallbackSubmitter>) {
        return Fail("stream queue callback submitter was not standard layout");
    }

    if (!std::is_trivially_copyable_v<AudioPcmStreamQueueCallbackSubmitter>) {
        return Fail("stream queue callback submitter was not trivially copyable");
    }

    return 0;
}

int AudioCallbackDescDefaultValuesAreBounded() {
    const AudioCallbackDeviceDesc desc{};
    if (desc.backend_kind != AudioBackendKind::Callback) {
        return Fail("callback descriptor did not select explicit backend");
    }

    if (desc.format != AudioSampleFormat::Signed16) {
        return Fail("callback descriptor did not select S16");
    }

    if (desc.sample_rate != SAMPLE_RATE) {
        return Fail("callback descriptor did not select fixed sample rate");
    }

    if (desc.channel_count != CHANNEL_COUNT) {
        return Fail("callback descriptor did not select stereo");
    }

    if (desc.buffer_count < AudioCallbackDeviceDesc::MIN_BUFFER_COUNT) {
        return Fail("callback descriptor buffer count was below minimum");
    }

    if (desc.buffer_count > AudioCallbackDeviceDesc::MAX_BUFFER_COUNT) {
        return Fail("callback descriptor buffer count exceeded maximum");
    }

    if (desc.frames_per_buffer < AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER) {
        return Fail("callback descriptor frame count was below minimum");
    }

    if (desc.frames_per_buffer > AudioCallbackDeviceDesc::MAX_FRAMES_PER_BUFFER) {
        return Fail("callback descriptor frame count exceeded maximum");
    }

    return 0;
}

int AudioCallbackSnapshotDefaultValuesAreExplicit() {
    const AudioCallbackSnapshot snapshot{};
    if (snapshot.last_status != AudioStatus::NotInitialized) {
        return Fail("callback snapshot default status was not explicit");
    }

    if (snapshot.initialized) {
        return Fail("callback snapshot default initialized flag was true");
    }

    if (snapshot.started) {
        return Fail("callback snapshot default started flag was true");
    }

    if (snapshot.shutdown) {
        return Fail("callback snapshot default shutdown flag was true");
    }

    if (!CallbackSnapshotsEqual(snapshot, AudioCallbackSnapshot{})) {
        return Fail("callback snapshot default counters were unexpected");
    }

    return 0;
}

int AudioCallbackCompletionDefaultValuesAreExplicit() {
    const AudioCallbackCompletion completion{};
    if (completion.status != AudioStatus::Success) {
        return Fail("callback completion default status was unexpected");
    }

    if (completion.sequence != 0U) {
        return Fail("callback completion default sequence was unexpected");
    }

    if (completion.buffer_slot != 0U) {
        return Fail("callback completion default slot was unexpected");
    }

    if (completion.frame_count != 0U) {
        return Fail("callback completion default frame count was unexpected");
    }

    return 0;
}

int AudioCallbackPublicContractUsesExplicitOwnerAndValues() {
    if (!std::is_standard_layout_v<AudioCallbackDeviceDesc>) {
        return Fail("callback descriptor was not a plain value contract");
    }

    if (!std::is_trivially_copyable_v<AudioCallbackDeviceDesc>) {
        return Fail("callback descriptor was not trivially copyable");
    }

    if (!std::is_standard_layout_v<AudioCallbackSnapshot>) {
        return Fail("callback snapshot was not a plain value contract");
    }

    if (!std::is_trivially_copyable_v<AudioCallbackSnapshot>) {
        return Fail("callback snapshot was not trivially copyable");
    }

    if (std::is_copy_constructible_v<AudioCallbackDevice>) {
        return Fail("callback device owner was copy constructible");
    }

    if (std::is_copy_assignable_v<AudioCallbackDevice>) {
        return Fail("callback device owner was copy assignable");
    }

    if (std::is_move_constructible_v<AudioCallbackDevice>) {
        return Fail("callback device owner was move constructible");
    }

    return 0;
}

int AudioCallbackDeviceRejectsUnsupportedBackendBeforeHardware() {
    AudioCallbackDeviceDesc desc{};
    desc.backend_kind = AudioBackendKind::Test;
    if (!ExpectCallbackInitializeStatusWithoutMutation(desc, AudioStatus::UnsupportedBackend)) {
        return Fail("callback device did not reject unsupported backend before hardware");
    }

    desc.backend_kind = AudioBackendKind::Unsupported;
    if (!ExpectCallbackInitializeStatusWithoutMutation(desc, AudioStatus::UnsupportedBackend)) {
        return Fail("callback device did not reject unsupported backend value before hardware");
    }

    return 0;
}

int AudioCallbackDeviceRejectsUnsupportedFormatBeforeHardware() {
    AudioCallbackDeviceDesc format_desc{};
    format_desc.format = AudioSampleFormat::Unsupported;
    if (!ExpectCallbackInitializeStatusWithoutMutation(format_desc, AudioStatus::UnsupportedFormat)) {
        return Fail("callback device did not reject unsupported format before hardware");
    }

    AudioCallbackDeviceDesc sample_rate_desc{};
    sample_rate_desc.sample_rate = 44100U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(sample_rate_desc, AudioStatus::UnsupportedFormat)) {
        return Fail("callback device did not reject unsupported sample rate before hardware");
    }

    AudioCallbackDeviceDesc channel_desc{};
    channel_desc.channel_count = 1U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(channel_desc, AudioStatus::UnsupportedFormat)) {
        return Fail("callback device did not reject unsupported channel count before hardware");
    }

    return 0;
}

int AudioCallbackDeviceRejectsInvalidBufferShapeBeforeHardware() {
    AudioCallbackDeviceDesc small_buffer_desc{};
    small_buffer_desc.buffer_count = AudioCallbackDeviceDesc::MIN_BUFFER_COUNT - 1U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(small_buffer_desc, AudioStatus::InvalidDescriptor)) {
        return Fail("callback device did not reject small buffer count before hardware");
    }

    AudioCallbackDeviceDesc large_buffer_desc{};
    large_buffer_desc.buffer_count = AudioCallbackDeviceDesc::MAX_BUFFER_COUNT + 1U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(large_buffer_desc, AudioStatus::CapacityExceeded)) {
        return Fail("callback device did not reject large buffer count before hardware");
    }

    AudioCallbackDeviceDesc small_frame_desc{};
    small_frame_desc.frames_per_buffer = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER - 1U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(small_frame_desc, AudioStatus::InvalidDescriptor)) {
        return Fail("callback device did not reject small frame count before hardware");
    }

    AudioCallbackDeviceDesc large_frame_desc{};
    large_frame_desc.frames_per_buffer = AudioCallbackDeviceDesc::MAX_FRAMES_PER_BUFFER + 1U;
    if (!ExpectCallbackInitializeStatusWithoutMutation(large_frame_desc, AudioStatus::CapacityExceeded)) {
        return Fail("callback device did not reject large frame count before hardware");
    }

    return 0;
}

int AudioCallbackDeviceUninitializedOperationsReturnExplicitStatus() {
    AudioCallbackDevice device;
    const AudioCallbackSnapshot before_snapshot = device.Snapshot();
    std::array<std::int16_t, 2U> samples{};
    std::array<AudioCallbackCompletion, 1U> completions{};
    std::size_t completion_count = 99U;

    if (device.Start() != AudioStatus::NotInitialized) {
        return Fail("callback start did not return uninitialized status");
    }

    if (device.SubmitS16Buffer(std::span<const std::int16_t>(samples.data(), samples.size()), 1U) != AudioStatus::NotInitialized) {
        return Fail("callback submit did not return uninitialized status");
    }

    if (device.WaitForCompletedCallbacks(1U, 1U) != AudioStatus::NotInitialized) {
        return Fail("callback wait did not return uninitialized status");
    }

    if (device.DrainCompletions(completions.data(), completions.size(), completion_count) != AudioStatus::NotInitialized) {
        return Fail("callback drain did not return uninitialized status");
    }

    if (completion_count != 0U) {
        return Fail("callback drain did not reset output count");
    }

    if (device.Stop() != AudioStatus::NotInitialized) {
        return Fail("callback stop did not return uninitialized status");
    }

    if (device.Shutdown() != AudioStatus::NotInitialized) {
        return Fail("callback shutdown did not return uninitialized status");
    }

    if (!CallbackSnapshotsEqual(device.Snapshot(), before_snapshot)) {
        return Fail("callback uninitialized operations mutated snapshot");
    }

    return 0;
}

int AudioCallbackDeviceControlledBackendSubmitsAndDrains() {
    yuengine::audio::AudioCallbackDeviceBackendTestConfig config{};
    config.enabled = true;
    config.complete_submitted_buffer = true;
    ScopedAudioCallbackBackendTestConfig scoped_config(config);

    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
    AudioCallbackDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("controlled callback initialize failed");
    }

    if (device.Start() != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback start failed");
    }

    constexpr std::size_t SAMPLE_COUNT = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER * CHANNEL_COUNT;
    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    samples[0U] = 11;
    samples[1U] = -11;
    if (device.SubmitS16Buffer(std::span<const std::int16_t>(samples.data(), samples.size()), desc.frames_per_buffer) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback submit failed");
    }

    if (device.WaitForCompletedCallbacks(1U, 1U) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback wait failed");
    }

    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    if (device.DrainCompletions(completions.data(), completions.size(), completion_count) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback drain failed");
    }

    if (completion_count != 1U) {
        return FailAfterShutdown(device, "controlled callback completion count changed");
    }

    if (completions[0U].status != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback completion status changed");
    }

    if (completions[0U].frame_count != desc.frames_per_buffer) {
        return FailAfterShutdown(device, "controlled callback completion frame count changed");
    }

    const AudioCallbackSnapshot before_shutdown_snapshot = device.Snapshot();
    if (before_shutdown_snapshot.submitted_buffer_count != 1U) {
        return FailAfterShutdown(device, "controlled callback submitted count changed");
    }

    if (before_shutdown_snapshot.completed_callback_count != 1U) {
        return FailAfterShutdown(device, "controlled callback completed count changed");
    }

    if (before_shutdown_snapshot.drained_completion_count != 1U) {
        return FailAfterShutdown(device, "controlled callback drained count changed");
    }

    if (before_shutdown_snapshot.queued_buffer_count != 0U) {
        return FailAfterShutdown(device, "controlled callback queued count changed");
    }

    if (device.Stop() != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback stop failed");
    }

    if (device.Shutdown() != AudioStatus::ShutdownComplete) {
        return Fail("controlled callback shutdown failed");
    }

    return 0;
}

int AudioCallbackDeviceControlledBackendSubmitCapacityRecordsRejectedEntry() {
    yuengine::audio::AudioCallbackDeviceBackendTestConfig config{};
    config.enabled = true;
    config.complete_submitted_buffer = false;
    ScopedAudioCallbackBackendTestConfig scoped_config(config);

    AudioCallbackDeviceDesc desc{};
    desc.buffer_count = AudioCallbackDeviceDesc::MIN_BUFFER_COUNT;
    desc.frames_per_buffer = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
    AudioCallbackDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("controlled callback capacity initialize failed");
    }

    if (device.Start() != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback capacity start failed");
    }

    constexpr std::size_t SAMPLE_COUNT = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER * CHANNEL_COUNT;
    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    for (std::size_t index = 0U; index < desc.buffer_count; ++index) {
        samples[0U] = static_cast<std::int16_t>(index + 1U);
        const std::span<const std::int16_t> submit_samples(samples.data(), samples.size());
        if (device.SubmitS16Buffer(submit_samples, desc.frames_per_buffer) != AudioStatus::Success) {
            return FailAfterShutdown(device, "controlled callback capacity setup submit failed");
        }
    }

    const std::span<const std::int16_t> rejected_samples(samples.data(), samples.size());
    if (device.SubmitS16Buffer(rejected_samples, desc.frames_per_buffer) != AudioStatus::CapacityExceeded) {
        return FailAfterShutdown(device, "controlled callback capacity status changed");
    }

    const AudioCallbackSnapshot capacity_snapshot = device.Snapshot();
    const std::uint64_t expected_sequence = static_cast<std::uint64_t>(desc.buffer_count) + 1U;
    const std::size_t required_queued_buffer_count = desc.buffer_count + 1U;
    if (!CallbackSubmissionCapacityFieldsMatch(capacity_snapshot,
                                               expected_sequence,
                                               desc.buffer_count,
                                               desc.buffer_count,
                                               desc.frames_per_buffer,
                                               samples.size(),
                                               required_queued_buffer_count)) {
        return FailAfterShutdown(device, "controlled callback capacity identity changed");
    }

    if (capacity_snapshot.failed_submission_count != 1U) {
        return FailAfterShutdown(device, "controlled callback capacity failure count changed");
    }

    if (capacity_snapshot.queued_buffer_count != desc.buffer_count) {
        return FailAfterShutdown(device, "controlled callback capacity queued count changed");
    }

    const std::size_t invalid_frame_count = desc.frames_per_buffer - 1U;
    if (device.SubmitS16Buffer(rejected_samples, invalid_frame_count) != AudioStatus::InvalidDescriptor) {
        return FailAfterShutdown(device, "controlled callback invalid submit status changed");
    }

    const AudioCallbackSnapshot invalid_snapshot = device.Snapshot();
    if (!CallbackSubmissionCapacityFieldsAreClear(invalid_snapshot)) {
        return FailAfterShutdown(device, "controlled callback invalid submit kept capacity identity");
    }

    if (device.SubmitS16Buffer(rejected_samples, desc.frames_per_buffer) != AudioStatus::CapacityExceeded) {
        return FailAfterShutdown(device, "controlled callback second capacity status changed");
    }

    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 99U;
    if (device.DrainCompletions(completions.data(), completions.size(), completion_count) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback empty drain status changed");
    }

    if (completion_count != 0U) {
        return FailAfterShutdown(device, "controlled callback empty drain count changed");
    }

    const AudioCallbackSnapshot drain_snapshot = device.Snapshot();
    if (!CallbackSubmissionCapacityFieldsAreClear(drain_snapshot)) {
        return FailAfterShutdown(device, "controlled callback empty drain kept capacity identity");
    }

    if (device.SubmitS16Buffer(rejected_samples, desc.frames_per_buffer) != AudioStatus::CapacityExceeded) {
        return FailAfterShutdown(device, "controlled callback shutdown capacity status changed");
    }

    if (device.Shutdown() != AudioStatus::ShutdownComplete) {
        return Fail("controlled callback capacity shutdown failed");
    }

    const AudioCallbackSnapshot shutdown_snapshot = device.Snapshot();
    if (!CallbackSubmissionCapacityFieldsAreClear(shutdown_snapshot)) {
        return Fail("controlled callback shutdown kept capacity identity");
    }

    return 0;
}

int AudioCallbackDeviceControlledBackendSplitsUnavailableAndBackendError() {
    AudioCallbackDeviceDesc desc{};

    yuengine::audio::AudioCallbackDeviceBackendTestConfig unavailable_config{};
    unavailable_config.enabled = true;
    unavailable_config.initialize_status = AudioStatus::DeviceUnavailable;
    {
        ScopedAudioCallbackBackendTestConfig scoped_config(unavailable_config);
        AudioCallbackDevice device;
        if (device.Initialize(desc) != AudioStatus::DeviceUnavailable) {
            return Fail("controlled callback unavailable status changed");
        }

        const AudioCallbackSnapshot snapshot = device.Snapshot();
        if (snapshot.initialized) {
            return Fail("controlled callback unavailable initialized flag changed");
        }

        if (snapshot.last_status != AudioStatus::DeviceUnavailable) {
            return Fail("controlled callback unavailable last status changed");
        }
    }

    yuengine::audio::AudioCallbackDeviceBackendTestConfig backend_error_config{};
    backend_error_config.enabled = true;
    backend_error_config.initialize_status = AudioStatus::BackendError;
    ScopedAudioCallbackBackendTestConfig scoped_config(backend_error_config);
    AudioCallbackDevice device;
    if (device.Initialize(desc) != AudioStatus::BackendError) {
        return Fail("controlled callback backend error status changed");
    }

    const AudioCallbackSnapshot snapshot = device.Snapshot();
    if (snapshot.initialized) {
        return Fail("controlled callback backend error initialized flag changed");
    }

    if (snapshot.last_status != AudioStatus::BackendError) {
        return Fail("controlled callback backend error last status changed");
    }

    return 0;
}

int AudioCallbackDeviceControlledBackendSubmitErrorRollsBack() {
    yuengine::audio::AudioCallbackDeviceBackendTestConfig config{};
    config.enabled = true;
    config.submit_status = AudioStatus::BackendError;
    ScopedAudioCallbackBackendTestConfig scoped_config(config);

    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
    AudioCallbackDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("controlled callback submit error initialize failed");
    }

    if (device.Start() != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback submit error start failed");
    }

    constexpr std::size_t SAMPLE_COUNT = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER * CHANNEL_COUNT;
    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    if (device.SubmitS16Buffer(std::span<const std::int16_t>(samples.data(), samples.size()), desc.frames_per_buffer) != AudioStatus::BackendError) {
        return FailAfterShutdown(device, "controlled callback submit error status changed");
    }

    const AudioCallbackSnapshot snapshot = device.Snapshot();
    if (snapshot.submitted_buffer_count != 0U) {
        return FailAfterShutdown(device, "controlled callback failed submit kept submitted count");
    }

    if (snapshot.queued_buffer_count != 0U) {
        return FailAfterShutdown(device, "controlled callback failed submit kept queued count");
    }

    if (snapshot.failed_submission_count != 1U) {
        return FailAfterShutdown(device, "controlled callback failed submit counter changed");
    }

    if (snapshot.last_status != AudioStatus::BackendError) {
        return FailAfterShutdown(device, "controlled callback failed submit last status changed");
    }

    if (!CallbackSubmissionCapacityFieldsAreClear(snapshot)) {
        return FailAfterShutdown(device, "controlled callback failed submit kept capacity identity");
    }

    if (device.Shutdown() != AudioStatus::ShutdownComplete) {
        return Fail("controlled callback submit error shutdown failed");
    }

    return 0;
}

int AudioCallbackDeviceControlledBackendCallbackErrorReportsFailure() {
    yuengine::audio::AudioCallbackDeviceBackendTestConfig config{};
    config.enabled = true;
    config.report_callback_error_on_submit = true;
    ScopedAudioCallbackBackendTestConfig scoped_config(config);

    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
    AudioCallbackDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("controlled callback error initialize failed");
    }

    if (device.Start() != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback error start failed");
    }

    constexpr std::size_t SAMPLE_COUNT = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER * CHANNEL_COUNT;
    std::array<std::int16_t, SAMPLE_COUNT> samples{};
    if (device.SubmitS16Buffer(std::span<const std::int16_t>(samples.data(), samples.size()), desc.frames_per_buffer) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback error submit failed");
    }

    if (device.WaitForCompletedCallbacks(1U, 1U) != AudioStatus::CallbackFailed) {
        return FailAfterShutdown(device, "controlled callback error wait status changed");
    }

    const AudioCallbackSnapshot snapshot = device.Snapshot();
    if (snapshot.failed_callback_count != 1U) {
        return FailAfterShutdown(device, "controlled callback error counter changed");
    }

    if (snapshot.last_status != AudioStatus::CallbackFailed) {
        return FailAfterShutdown(device, "controlled callback error last status changed");
    }

    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    if (device.DrainCompletions(completions.data(), completions.size(), completion_count) != AudioStatus::Success) {
        return FailAfterShutdown(device, "controlled callback error drain failed");
    }

    if (completion_count != 0U) {
        return FailAfterShutdown(device, "controlled callback error completion count changed");
    }

    const AudioCallbackSnapshot drain_snapshot = device.Snapshot();
    if (drain_snapshot.last_status != AudioStatus::CallbackFailed) {
        return FailAfterShutdown(device, "controlled callback error drain last status changed");
    }

    if (device.Shutdown() != AudioStatus::ShutdownComplete) {
        return Fail("controlled callback error shutdown failed");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_CREATE_DEVICE, AudioCreateTestDeviceReturnsCapabilities},
        {TEST_UNSUPPORTED_BACKEND, AudioCreateDeviceRejectsUnsupportedBackend},
        {TEST_UNSUPPORTED_FORMAT, AudioCreateDeviceRejectsUnsupportedFormat},
        {TEST_REGISTER_SOURCE, AudioRegisterSyntheticSourceReturnsStableId},
        {TEST_SOURCE_CAPACITY, AudioSourceCapacityOverflowDoesNotMutate},
        {TEST_START_VOICE, AudioStartVoiceReturnsGenerationHandle},
        {TEST_MISSING_SOURCE, AudioStartVoiceRejectsMissingSource},
        {TEST_INVALID_GAIN, AudioStartVoiceRejectsInvalidGainWithoutMutation},
        {TEST_VOICE_CAPACITY, AudioVoiceCapacityOverflowDoesNotMutate},
        {TEST_STOP_STALE, AudioStopVoiceInvalidatesStaleHandle},
        {TEST_REINITIALIZE_STALE_HANDLES, AudioReinitializeInvalidatesPriorSourceAndVoiceHandles},
        {TEST_SINGLE_VOICE, AudioMixSingleVoiceWritesDeterministicS16StereoSamples},
        {TEST_UNITY_GAIN, AudioMixUnityGainPreservesS16EdgeSamples},
        {TEST_FRACTIONAL_GAIN, AudioMixFractionalGainRoundsTowardZeroDeterministically},
        {TEST_MULTIPLE_VOICES, AudioMixMultipleVoicesUsesStableOrderAndSaturates},
        {TEST_MAX_VOICE_FULL_SCALE, AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow},
        {TEST_STOPS_AT_END, AudioMixStopsVoiceAtEndOfSource},
        {TEST_SILENT_TAIL, AudioMixEndedVoiceWritesSilentTail},
        {TEST_OVERWRITE, AudioMixOverwritesPrefilledDestination},
        {TEST_UNDERSIZED_OUTPUT, AudioMixRejectsUndersizedBufferWithoutWritingSamples},
        {TEST_MIX_OUTPUT_CAPACITY_ENTRY, AudioMixRejectsUndersizedBufferReportsCapacityEntry},
        {TEST_UNINITIALIZED_LIFECYCLE, AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation},
        {TEST_NO_GROW, AudioMixDoesNotGrowVoiceStorage},
        {TEST_DISABLED_DIAGNOSTICS, AudioDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FORBIDDEN_DEPENDENCY, AudioNoDeviceCodecResourceScriptUiGameAdapterDependency},
        {TEST_CALLBACK_DESC, AudioCallbackDescDefaultValuesAreBounded},
        {TEST_CALLBACK_SNAPSHOT, AudioCallbackSnapshotDefaultValuesAreExplicit},
        {TEST_CALLBACK_COMPLETION, AudioCallbackCompletionDefaultValuesAreExplicit},
        {TEST_CALLBACK_PUBLIC_CONTRACT, AudioCallbackPublicContractUsesExplicitOwnerAndValues},
        {TEST_CALLBACK_UNSUPPORTED_BACKEND, AudioCallbackDeviceRejectsUnsupportedBackendBeforeHardware},
        {TEST_CALLBACK_UNSUPPORTED_FORMAT, AudioCallbackDeviceRejectsUnsupportedFormatBeforeHardware},
        {TEST_CALLBACK_INVALID_BUFFER_SHAPE, AudioCallbackDeviceRejectsInvalidBufferShapeBeforeHardware},
        {TEST_CALLBACK_UNINITIALIZED_OPERATIONS, AudioCallbackDeviceUninitializedOperationsReturnExplicitStatus},
        {TEST_CALLBACK_CONTROLLED_SUBMIT_DRAIN, AudioCallbackDeviceControlledBackendSubmitsAndDrains},
        {TEST_CALLBACK_CONTROLLED_SUBMIT_CAPACITY, AudioCallbackDeviceControlledBackendSubmitCapacityRecordsRejectedEntry},
        {TEST_CALLBACK_CONTROLLED_STATUS_SPLIT, AudioCallbackDeviceControlledBackendSplitsUnavailableAndBackendError},
        {TEST_CALLBACK_CONTROLLED_SUBMIT_ERROR, AudioCallbackDeviceControlledBackendSubmitErrorRollsBack},
        {TEST_CALLBACK_CONTROLLED_CALLBACK_ERROR, AudioCallbackDeviceControlledBackendCallbackErrorReportsFailure},
        {TEST_PCM_CREATE_QUERY_RELEASE, AudioPcmSamplePacketCreatesQueriesAndReleasesMetadata},
        {TEST_PCM_DUPLICATE, AudioPcmSamplePacketRejectsDuplicatePacketIdWithoutMutation},
        {TEST_PCM_UNSUPPORTED_FORMAT, AudioPcmSamplePacketRejectsUnsupportedFormatWithoutMutation},
        {TEST_PCM_UNSUPPORTED_SAMPLE_RATE, AudioPcmSamplePacketRejectsUnsupportedSampleRateWithoutMutation},
        {TEST_PCM_UNSUPPORTED_CHANNEL_COUNT, AudioPcmSamplePacketRejectsUnsupportedChannelCountWithoutMutation},
        {TEST_PCM_ZERO_FRAME_COUNT, AudioPcmSamplePacketRejectsZeroFrameCountWithoutMutation},
        {TEST_PCM_SAMPLE_COUNT_MISMATCH, AudioPcmSamplePacketRejectsSampleCountMismatchWithoutMutation},
        {TEST_PCM_BYTE_COUNT_MISMATCH, AudioPcmSamplePacketRejectsByteCountMismatchWithoutMutation},
        {TEST_PCM_CAPACITY, AudioPcmSamplePacketRejectsCapacityOverflowWithoutMutation},
        {TEST_PCM_STALE_HANDLE, AudioPcmSamplePacketRejectsStaleHandleWithoutMutation},
        {TEST_PCM_SNAPSHOT_COUNTERS, AudioPcmSamplePacketSnapshotTracksCounters},
        {TEST_PCM_SOURCE_VOICE_BOUNDARY, AudioPcmSamplePacketRejectionsDoNotMutateSourceVoiceState},
        {TEST_PCM_PUBLIC_CONTRACT, AudioPcmSamplePacketPublicContractsArePlainValues},
        {TEST_STREAM_QUEUE_LIFECYCLE, AudioPcmStreamQueueQueuesDrainsAndReleasesMetadata},
        {TEST_STREAM_QUEUE_INVALID_PACKET, AudioPcmStreamQueueRejectsInvalidPacketHandleWithoutMutation},
        {TEST_STREAM_QUEUE_RELEASED_PACKET, AudioPcmStreamQueueRejectsReleasedPacketHandleWithoutMutation},
        {TEST_STREAM_QUEUE_PACKET_ID_MISMATCH, AudioPcmStreamQueueRejectsPacketIdMismatchWithoutMutation},
        {TEST_STREAM_QUEUE_DUPLICATE, AudioPcmStreamQueueRejectsDuplicateQueueIdWithoutMutation},
        {TEST_STREAM_QUEUE_RANGE, AudioPcmStreamQueueRejectsFrameRangeOverflowWithoutMutation},
        {TEST_STREAM_QUEUE_SAMPLE_COUNT, AudioPcmStreamQueueRejectsSampleCountMismatchWithoutMutation},
        {TEST_STREAM_QUEUE_BYTE_COUNT, AudioPcmStreamQueueRejectsByteCountMismatchWithoutMutation},
        {TEST_STREAM_QUEUE_CHUNK, AudioPcmStreamQueueRejectsInvalidChunkFrameCountWithoutMutation},
        {TEST_STREAM_QUEUE_CAPACITY, AudioPcmStreamQueueRejectsCapacityOverflowWithoutMutation},
        {TEST_STREAM_QUEUE_CAPACITY_ENTRY, AudioPcmStreamQueueCapacityOverflowReportsRejectedEntry},
        {TEST_STREAM_QUEUE_SMALL_OUTPUT, AudioPcmStreamQueueDrainRejectsSmallOutputWithoutMutation},
        {TEST_STREAM_QUEUE_STALE, AudioPcmStreamQueueRejectsStaleHandleWithoutMutation},
        {TEST_STREAM_QUEUE_NO_MUTATION, AudioPcmStreamQueueRejectionsDoNotMutatePacketSourceVoiceCallbackState},
        {TEST_STREAM_QUEUE_SNAPSHOT, AudioPcmStreamQueueSnapshotTracksCounters},
        {TEST_STREAM_QUEUE_PUBLIC_CONTRACT, AudioPcmStreamQueuePublicContractsArePlainValues},
        {TEST_STREAM_QUEUE_CALLBACK_SUBMIT, AudioPcmStreamQueueCallbackSubmitsDrainedChunks},
        {TEST_STREAM_QUEUE_CALLBACK_PARTIAL, AudioPcmStreamQueueCallbackRejectsPartialFinalChunkWithoutDrain},
        {TEST_STREAM_QUEUE_CALLBACK_SMALL_SPAN, AudioPcmStreamQueueCallbackRejectsSmallSampleSpanWithoutDrain},
        {TEST_STREAM_QUEUE_CALLBACK_SUBMIT_FAILURE, AudioPcmStreamQueueCallbackStopsOnSubmitFailure},
        {TEST_STREAM_QUEUE_CALLBACK_UNSTARTED, AudioPcmStreamQueueCallbackRejectsUnstartedCallbackWithoutDrain},
        {TEST_STREAM_QUEUE_CALLBACK_PUBLIC_CONTRACT, AudioPcmStreamQueueCallbackPublicContractsArePlainValues}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
