// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackDevice.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioCallbackDeviceState;

class AudioCallbackDevice final {
public:
    /**
     * @comment Constructs an empty audio callback device owner.
     */
    AudioCallbackDevice();
    /**
     * @comment Releases any private backend objects owned by the device.
     */
    ~AudioCallbackDevice();

    AudioCallbackDevice(const AudioCallbackDevice &) = delete;
    AudioCallbackDevice &operator=(const AudioCallbackDevice &) = delete;

    /**
     * @comment Initializes a private callback backend from a value descriptor.
     * @param desc Input callback backend descriptor.
     * @return Explicit operation status.
     */
    AudioStatus Initialize(const AudioCallbackDeviceDesc &desc);
    /**
     * @comment Starts callback processing for the initialized backend.
     * @return Explicit operation status.
     */
    AudioStatus Start();
    /**
     * @comment Submits fixed S16 interleaved samples to a preallocated callback buffer.
     * @param interleaved_samples Input caller-owned S16 interleaved samples.
     * @param frame_count Input frame count.
     * @return Explicit operation status.
     */
    AudioStatus SubmitS16Buffer(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count);
    /**
     * @comment Waits until at least target_completed_count callbacks are completed.
     * @param target_completed_count Target completed callback count.
     * @param timeout_milliseconds Bounded wait duration.
     * @return Explicit operation status.
     */
    AudioStatus WaitForCompletedCallbacks(std::uint64_t target_completed_count, std::uint32_t timeout_milliseconds);
    /**
     * @comment Drains callback completion records into caller-owned storage.
     * @param completions Caller-owned completion output buffer.
     * @param completion_capacity Number of completion records available.
     * @param out_completion_count Output written completion count.
     * @return Explicit operation status.
     */
    AudioStatus DrainCompletions(AudioCallbackCompletion *completions, std::size_t completion_capacity, std::size_t &out_completion_count);
    /**
     * @comment Stops callback processing without destroying private backend objects.
     * @return Explicit operation status.
     */
    AudioStatus Stop();
    /**
     * @comment Shuts down the private backend and releases all platform objects.
     * @return Explicit operation status.
     */
    AudioStatus Shutdown();
    /**
     * @comment Returns callback counters and lifecycle state.
     * @return Snapshot value.
     */
    AudioCallbackSnapshot Snapshot() const;

private:
    AudioCallbackDeviceState *state_;
};
}
