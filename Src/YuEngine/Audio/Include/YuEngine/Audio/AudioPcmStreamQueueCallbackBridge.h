// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioPcmStreamQueueCallbackBridge.h

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
 * @comment 为已初始化 callback device 创建 submitter adapter。
 * @param callback_device 输入 callback device owner。
 * @return 包含 callback device state public values 的 submitter value。
 */
AudioPcmStreamQueueCallbackSubmitter CreateAudioCallbackDeviceSubmitter(AudioCallbackDevice *callback_device);

/**
 * @comment 将 PCM stream queue 排空到 caller storage，并把每个 chunk 提交给 S16 callback submitter。
 * @param stream_device 输入 stream queue owner。
 * @param queue 输入 stream queue handle。
 * @param submitter 输入 callback submitter value。
 * @param interleaved_samples 输入调用方持有的 packet S16 sample storage。
 * @param chunk_storage 调用方持有的 chunk descriptor storage。
 * @param out_result 输出 bridge result。
 * @return 显式操作状态。
 */
AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           const AudioPcmStreamQueueCallbackSubmitter &submitter,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result);

/**
 * @comment 排空 PCM stream queue，并把每个 chunk 提交给 AudioCallbackDevice。
 * @param stream_device 输入 stream queue owner。
 * @param queue 输入 stream queue handle。
 * @param callback_device 输入 callback device owner。
 * @param interleaved_samples 输入调用方持有的 packet S16 sample storage。
 * @param chunk_storage 调用方持有的 chunk descriptor storage。
 * @param out_result 输出 bridge result。
 * @return 显式操作状态。
 */
AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           AudioCallbackDevice *callback_device,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result);
}
