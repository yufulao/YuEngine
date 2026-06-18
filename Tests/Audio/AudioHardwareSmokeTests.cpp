// Module: Tests Audio
// File: Tests/Audio/AudioHardwareSmokeTests.cpp

#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Audio/AudioCallbackDevice.h"
#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioDeviceDesc.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackBridge.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackResult.h"
#include "YuEngine/Audio/AudioPcmStreamQueueChunk.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Audio/TestAudioDevice.h"

using yuengine::audio::AudioCallbackCompletion;
using yuengine::audio::AudioCallbackDevice;
using yuengine::audio::AudioCallbackDeviceDesc;
using yuengine::audio::AudioCallbackSnapshot;
using yuengine::audio::AudioDeviceDesc;
using yuengine::audio::AudioPcmSamplePacketHandle;
using yuengine::audio::AudioPcmSamplePacketRequest;
using yuengine::audio::AudioPcmStreamQueueCallbackResult;
using yuengine::audio::AudioPcmStreamQueueChunk;
using yuengine::audio::AudioPcmStreamQueueHandle;
using yuengine::audio::AudioPcmStreamQueueRequest;
using yuengine::audio::AudioSampleFormat;
using yuengine::audio::AudioStatus;
using yuengine::audio::CHANNEL_COUNT;
using yuengine::audio::SAMPLE_RATE;
using yuengine::audio::SubmitPcmStreamQueueToCallback;
using TestAudioDevice = yuengine::audio::TestAudioDevice;

namespace {
constexpr const char *TEST_HARDWARE_CALLBACK = "Audio_HardwareCallback_CompletesSubmittedBuffer";
constexpr const char *TEST_HARDWARE_STREAM_QUEUE_CALLBACK = "Audio_HardwareCallback_SubmitsPcmStreamQueue";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr int SKIP_RETURN_CODE = 77;
constexpr std::size_t SMOKE_FRAMES = AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
constexpr std::size_t SMOKE_SAMPLE_COUNT = SMOKE_FRAMES * CHANNEL_COUNT;
constexpr std::uint32_t CALLBACK_WAIT_TIMEOUT_MS = 2000U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Skip(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stdout);
    std::fputc('\n', stdout);
    return SKIP_RETURN_CODE;
}

bool IsStrictHardwareRequested() {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
    const char *value = std::getenv("YUENGINE_AUDIO_STRICT_HARDWARE");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (value == nullptr) {
        return false;
    }

    const std::string_view request(value);
    if (request == "1") {
        return true;
    }

    if (request == "true") {
        return true;
    }

    return request == "TRUE";
}

int FailAfterShutdown(AudioCallbackDevice &device, std::string_view message) {
    device.Shutdown();
    return Fail(message);
}

bool CreateHardwareStreamQueue(TestAudioDevice &stream_device, AudioPcmStreamQueueHandle &out_queue) {
    const AudioPcmSamplePacketRequest packet_request{
        301U,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        SMOKE_FRAMES,
        SMOKE_SAMPLE_COUNT,
        SMOKE_SAMPLE_COUNT * sizeof(std::int16_t)};
    AudioPcmSamplePacketHandle packet{};
    if (stream_device.CreatePcmSamplePacket(packet_request, packet) != AudioStatus::Success) {
        return false;
    }

    const AudioPcmStreamQueueRequest queue_request{
        401U,
        packet,
        301U,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        0U,
        SMOKE_FRAMES,
        SMOKE_SAMPLE_COUNT,
        SMOKE_SAMPLE_COUNT * sizeof(std::int16_t),
        SMOKE_FRAMES};
    return stream_device.CreatePcmStreamQueue(queue_request, out_queue) == AudioStatus::Success;
}

int AudioHardwareCallbackCompletesSubmittedBuffer() {
    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = SMOKE_FRAMES;

    AudioCallbackDevice device;
    AudioStatus status = device.Initialize(desc);
    if (status == AudioStatus::UnsupportedBackend) {
        if (IsStrictHardwareRequested()) {
            return Fail("audio callback hardware strict smoke failed because the backend is not compiled");
        }

        return Skip("audio callback hardware smoke skipped because the backend is not compiled");
    }

    if (status == AudioStatus::DeviceUnavailable) {
        if (IsStrictHardwareRequested()) {
            return Fail("audio callback hardware strict smoke failed because a callback device is unavailable");
        }

        return Skip("audio callback hardware smoke skipped because a callback device is unavailable");
    }

    if (status != AudioStatus::Success) {
        return Fail("audio callback hardware initialize failed");
    }

    AudioCallbackSnapshot snapshot = device.Snapshot();
    if (!snapshot.initialized) {
        return FailAfterShutdown(device, "audio callback hardware snapshot was not initialized");
    }

    if (snapshot.buffer_capacity != desc.buffer_count) {
        return FailAfterShutdown(device, "audio callback hardware capacity was unexpected");
    }

    status = device.Start();
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware start failed");
    }

    std::array<std::int16_t, SMOKE_SAMPLE_COUNT> samples{};
    status = device.SubmitS16Buffer(std::span<const std::int16_t>(samples.data(), samples.size()), SMOKE_FRAMES);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware submit failed");
    }

    status = device.WaitForCompletedCallbacks(1U, CALLBACK_WAIT_TIMEOUT_MS);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware completion wait failed");
    }

    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    status = device.DrainCompletions(completions.data(), completions.size(), completion_count);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware drain failed");
    }

    if (completion_count != 1U) {
        return FailAfterShutdown(device, "audio callback hardware completion count was unexpected");
    }

    if (completions[0U].status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware completion status failed");
    }

    if (completions[0U].frame_count != SMOKE_FRAMES) {
        return FailAfterShutdown(device, "audio callback hardware completion frame count was unexpected");
    }

    status = device.Stop();
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(device, "audio callback hardware stop failed");
    }

    status = device.Shutdown();
    if (status != AudioStatus::ShutdownComplete) {
        return Fail("audio callback hardware shutdown failed");
    }

    snapshot = device.Snapshot();
    if (snapshot.submitted_buffer_count != 1U) {
        return Fail("audio callback hardware submitted count was unexpected");
    }

    if (snapshot.completed_callback_count != 1U) {
        return Fail("audio callback hardware callback count was unexpected");
    }

    if (snapshot.failed_submission_count != 0U) {
        return Fail("audio callback hardware submission failure count was unexpected");
    }

    if (snapshot.failed_callback_count != 0U) {
        return Fail("audio callback hardware callback failure count was unexpected");
    }

    if (snapshot.queued_buffer_count != 0U) {
        return Fail("audio callback hardware queued count was unexpected");
    }

    if (!snapshot.shutdown) {
        return Fail("audio callback hardware shutdown flag was not set");
    }

    return 0;
}

int AudioHardwareCallbackSubmitsPcmStreamQueue() {
    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = SMOKE_FRAMES;

    AudioCallbackDevice callback_device;
    AudioStatus status = callback_device.Initialize(desc);
    if (status == AudioStatus::UnsupportedBackend) {
        if (IsStrictHardwareRequested()) {
            return Fail("audio stream queue callback strict smoke failed because the backend is not compiled");
        }

        return Skip("audio stream queue callback hardware smoke skipped because the backend is not compiled");
    }

    if (status == AudioStatus::DeviceUnavailable) {
        if (IsStrictHardwareRequested()) {
            return Fail("audio stream queue callback strict smoke failed because a callback device is unavailable");
        }

        return Skip("audio stream queue callback hardware smoke skipped because a callback device is unavailable");
    }

    if (status != AudioStatus::Success) {
        return Fail("audio stream queue callback hardware initialize failed");
    }

    status = callback_device.Start();
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback hardware start failed");
    }

    TestAudioDevice stream_device;
    if (stream_device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback stream device initialize failed");
    }

    AudioPcmStreamQueueHandle queue{};
    if (!CreateHardwareStreamQueue(stream_device, queue)) {
        return FailAfterShutdown(callback_device, "audio stream queue callback setup failed");
    }

    std::array<std::int16_t, SMOKE_SAMPLE_COUNT> samples{};
    std::array<AudioPcmStreamQueueChunk, 1U> chunks{};
    AudioPcmStreamQueueCallbackResult result{};
    status = SubmitPcmStreamQueueToCallback(&stream_device,
                                            queue,
                                            &callback_device,
                                            std::span<const std::int16_t>(samples.data(), samples.size()),
                                            std::span<AudioPcmStreamQueueChunk>(chunks.data(), chunks.size()),
                                            &result);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback submit failed");
    }

    if (result.submitted_chunk_count != 1U || result.submitted_frame_count != SMOKE_FRAMES) {
        return FailAfterShutdown(callback_device, "audio stream queue callback result counts changed");
    }

    if (!result.reached_final_chunk) {
        return FailAfterShutdown(callback_device, "audio stream queue callback final chunk was not reached");
    }

    status = callback_device.WaitForCompletedCallbacks(1U, CALLBACK_WAIT_TIMEOUT_MS);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback completion wait failed");
    }

    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    status = callback_device.DrainCompletions(completions.data(), completions.size(), completion_count);
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback drain failed");
    }

    if (completion_count != 1U) {
        return FailAfterShutdown(callback_device, "audio stream queue callback completion count changed");
    }

    if (completions[0U].frame_count != SMOKE_FRAMES) {
        return FailAfterShutdown(callback_device, "audio stream queue callback completion frame count changed");
    }

    status = callback_device.Stop();
    if (status != AudioStatus::Success) {
        return FailAfterShutdown(callback_device, "audio stream queue callback stop failed");
    }

    status = callback_device.Shutdown();
    if (status != AudioStatus::ShutdownComplete) {
        return Fail("audio stream queue callback shutdown failed");
    }

    const AudioCallbackSnapshot snapshot = callback_device.Snapshot();
    if (snapshot.submitted_buffer_count != 1U || snapshot.completed_callback_count != 1U) {
        return Fail("audio stream queue callback snapshot counts changed");
    }

    if (snapshot.failed_submission_count != 0U || snapshot.failed_callback_count != 0U) {
        return Fail("audio stream queue callback failure counters changed");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    if (test_name != TEST_HARDWARE_CALLBACK) {
        if (test_name == TEST_HARDWARE_STREAM_QUEUE_CALLBACK) {
            return AudioHardwareCallbackSubmitsPcmStreamQueue();
        }

        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return AudioHardwareCallbackCompletesSubmittedBuffer();
}
