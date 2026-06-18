// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueCallbackBridge.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioPcmStreamQueueCallbackResult.h"
#include "YuEngine/Audio/AudioPcmStreamQueueCallbackSubmitter.h"
#include "YuEngine/Audio/AudioPcmStreamQueueChunk.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"

namespace yuengine::audio {
class AudioCallbackDevice;
class TestAudioDevice;

/**
 * @comment Creates a submitter adapter for an initialized callback device.
 * @param callback_device Input callback device owner.
 * @return Submitter value with callback device state captured as public values.
 */
AudioPcmStreamQueueCallbackSubmitter CreateAudioCallbackDeviceSubmitter(AudioCallbackDevice *callback_device);

/**
 * @comment Drains a PCM stream queue into caller storage and submits each chunk to an S16 callback submitter.
 * @param stream_device Input stream queue owner.
 * @param queue Input stream queue handle.
 * @param submitter Input callback submitter value.
 * @param interleaved_samples Input caller-owned S16 sample storage for the packet.
 * @param chunk_storage Caller-owned chunk descriptor storage.
 * @param out_result Output bridge result.
 * @return Explicit operation status.
 */
AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           const AudioPcmStreamQueueCallbackSubmitter &submitter,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result);

/**
 * @comment Drains a PCM stream queue and submits each chunk to an AudioCallbackDevice.
 * @param stream_device Input stream queue owner.
 * @param queue Input stream queue handle.
 * @param callback_device Input callback device owner.
 * @param interleaved_samples Input caller-owned S16 sample storage for the packet.
 * @param chunk_storage Caller-owned chunk descriptor storage.
 * @param out_result Output bridge result.
 * @return Explicit operation status.
 */
AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           AudioCallbackDevice *callback_device,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result);
}
