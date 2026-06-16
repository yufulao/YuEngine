// Module: Tests Audio
// File: Tests/Audio/AudioHardwareSmokeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Audio/AudioCallbackDevice.h"
#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioConstants.h"

using yuengine::audio::AudioCallbackCompletion;
using yuengine::audio::AudioCallbackDevice;
using yuengine::audio::AudioCallbackDeviceDesc;
using yuengine::audio::AudioCallbackSnapshot;
using yuengine::audio::AudioStatus;
using yuengine::audio::CHANNEL_COUNT;

namespace {
constexpr const char *TEST_HARDWARE_CALLBACK = "Audio_HardwareCallback_CompletesSubmittedBuffer";
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

int FailAfterShutdown(AudioCallbackDevice &device, std::string_view message) {
    device.Shutdown();
    return Fail(message);
}

int AudioHardwareCallbackCompletesSubmittedBuffer() {
    AudioCallbackDeviceDesc desc{};
    desc.frames_per_buffer = SMOKE_FRAMES;

    AudioCallbackDevice device;
    AudioStatus status = device.Initialize(desc);
    if (status == AudioStatus::UnsupportedBackend) {
        return Skip("audio callback hardware smoke skipped because the backend is not compiled");
    }

    if (status == AudioStatus::DeviceUnavailable) {
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
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    if (test_name != TEST_HARDWARE_CALLBACK) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return AudioHardwareCallbackCompletesSubmittedBuffer();
}
