// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/TestAudioDevice.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "YuEngine/Audio/AudioCapabilities.h"
#include "YuEngine/Audio/AudioDeviceDesc.h"
#include "YuEngine/Audio/AudioDeviceSnapshot.h"
#include "YuEngine/Audio/AudioMixResult.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketOperation.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRecord.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioPcmSamplePacketSlot.h"
#include "YuEngine/Audio/AudioPcmSamplePacketSnapshot.h"
#include "YuEngine/Audio/AudioPcmStreamQueueChunk.h"
#include "YuEngine/Audio/AudioPcmStreamQueueHandle.h"
#include "YuEngine/Audio/AudioPcmStreamQueueOperation.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRecord.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/Audio/AudioPcmStreamQueueSlot.h"
#include "YuEngine/Audio/AudioPcmStreamQueueSnapshot.h"
#include "YuEngine/Audio/AudioSourceId.h"
#include "YuEngine/Audio/AudioSourceSlot.h"
#include "YuEngine/Audio/AudioStatus.h"
#include "YuEngine/Audio/AudioVoiceHandle.h"
#include "YuEngine/Audio/AudioVoiceSlot.h"

namespace yuengine::audio {
class TestAudioDevice final {
public:
    /**
     * @comment 构造 TestAudioDevice 实例。
     */
    TestAudioDevice();

    /**
     * @comment 初始化实例。
     * @param desc 输入 descriptor。
     * @return 显式操作状态。
     */
    AudioStatus Initialize(const AudioDeviceDesc& desc);
    /**
     * @comment 注册 synthetic source。
     * @param interleaved_samples 输入 interleaved samples。
     * @param frame_count 输入 frame 数量。
     * @param out_source 成功时写入输出 source。
     * @return 显式操作状态。
     */
    AudioStatus RegisterSyntheticSource(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count, AudioSourceId& out_source);
    /**
     * @comment 创建有界 PCM sample packet 元数据记录。
     * @param request 输入 packet request。
     * @param out_packet 成功时写入输出 packet handle。
     * @return 显式操作状态。
     */
    AudioStatus CreatePcmSamplePacket(const AudioPcmSamplePacketRequest& request, AudioPcmSamplePacketHandle& out_packet);
    /**
     * @comment 查询 PCM sample packet 元数据记录。
     * @param packet 输入 packet handle。
     * @param out_record 成功时写入输出 packet record。
     * @return 显式操作状态。
     */
    AudioStatus QueryPcmSamplePacket(AudioPcmSamplePacketHandle packet, AudioPcmSamplePacketRecord& out_record);
    /**
     * @comment 释放 PCM sample packet 元数据记录。
     * @param packet 输入 packet handle。
     * @return 显式操作状态。
     */
    AudioStatus ReleasePcmSamplePacket(AudioPcmSamplePacketHandle packet);
    /**
     * @comment 创建有界 PCM stream queue 元数据记录。
     * @param request 输入 queue request。
     * @param out_queue 成功时写入输出 queue handle。
     * @return 显式操作状态。
     */
    AudioStatus CreatePcmStreamQueue(const AudioPcmStreamQueueRequest& request, AudioPcmStreamQueueHandle& out_queue);
    /**
     * @comment 查询 PCM stream queue 元数据记录。
     * @param queue 输入 queue handle。
     * @param out_record 成功时写入输出 queue record。
     * @return 显式操作状态。
     */
    AudioStatus QueryPcmStreamQueue(AudioPcmStreamQueueHandle queue, AudioPcmStreamQueueRecord& out_record);
    /**
     * @comment 将 PCM stream queue chunk descriptors 排空到调用方持有存储。
     * @param queue 输入 queue handle。
     * @param out_chunks 输出 chunk descriptor 存储。
     * @param out_chunk_count 函数写入的输出 chunk 数量。
     * @return 显式操作状态。
     */
    AudioStatus DrainPcmStreamQueue(AudioPcmStreamQueueHandle queue, std::span<AudioPcmStreamQueueChunk> out_chunks, std::size_t& out_chunk_count);
    /**
     * @comment 释放 PCM stream queue 元数据记录。
     * @param queue 输入 queue handle。
     * @return 显式操作状态。
     */
    AudioStatus ReleasePcmStreamQueue(AudioPcmStreamQueueHandle queue);
    /**
     * @comment 启动 voice。
     * @param source 输入 source。
     * @param gain_q15 输入 q15 gain。
     * @param out_voice 成功时写入输出 voice。
     * @return 显式操作状态。
     */
    AudioStatus StartVoice(AudioSourceId source, std::uint32_t gain_q15, AudioVoiceHandle& out_voice);
    /**
     * @comment 停止 voice。
     * @param handle 输入 handle。
     * @return 显式操作状态。
     */
    AudioStatus StopVoice(AudioVoiceHandle handle);
    /**
     * @comment 将请求的 samples 混入输出 buffer。
     * @param output_samples 函数更新的输出 sample buffer。
     * @param requested_frames 输入请求 frame 数量。
     * @return 显式操作结果。
     */
    AudioMixResult Mix(std::span<std::int16_t> output_samples, std::size_t requested_frames);
    /**
     * @comment 返回支持的 capabilities。
     * @return 能力数据。
     */
    AudioCapabilities Capabilities() const;
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    AudioDeviceSnapshot Snapshot() const;
    /**
     * @comment 返回 PCM sample packet 状态快照。
     * @return 快照值。
     */
    AudioPcmSamplePacketSnapshot PcmSamplePacketSnapshot() const;
    /**
     * @comment 返回 PCM stream queue 状态快照。
     * @return 快照值。
     */
    AudioPcmStreamQueueSnapshot PcmStreamQueueSnapshot() const;

private:
    AudioStatus RecordFailure(AudioStatus status);
    void RecordSuccess();
    AudioStatus RecordPcmSamplePacketFailure(AudioStatus status, AudioPcmSamplePacketOperation operation);
    AudioStatus RecordPcmStreamQueueFailure(AudioStatus status, AudioPcmStreamQueueOperation operation);
    AudioStatus ValidatePcmSamplePacketRequest(const AudioPcmSamplePacketRequest& request) const;
    bool IsDeviceFormatSupported(const AudioDeviceDesc& desc) const;
    bool IsSourceValid(AudioSourceId source) const;
    bool IsVoiceHandleValid(AudioVoiceHandle handle) const;
    bool IsPcmSamplePacketHandleValid(AudioPcmSamplePacketHandle packet) const;
    bool IsPcmStreamQueueHandleValid(AudioPcmStreamQueueHandle queue) const;
    bool HasActivePcmSamplePacketId(std::uint32_t packet_id) const;
    bool HasActivePcmStreamQueueId(std::uint32_t queue_id) const;
    std::int16_t ReadSourceSample(const AudioVoiceSlot& voice, std::size_t channel) const;
    std::int32_t ScaleSample(std::int16_t sample, std::uint32_t gain_q15) const;
    std::int16_t SaturateToS16(std::int64_t sample) const;
    void SetPcmSamplePacketLastStatus(AudioStatus status, AudioPcmSamplePacketOperation operation);
    void SetPcmStreamQueueLastStatus(AudioStatus status, AudioPcmStreamQueueOperation operation);
    void StopVoiceSlot(AudioVoiceSlot& voice);

    std::vector<AudioSourceSlot> sources_;
    std::vector<AudioVoiceSlot> voices_;
    std::vector<AudioPcmSamplePacketSlot> pcm_sample_packets_;
    std::vector<AudioPcmStreamQueueSlot> pcm_stream_queues_;
    AudioCapabilities capabilities_;
    AudioDeviceSnapshot snapshot_;
    AudioPcmSamplePacketSnapshot pcm_sample_packet_snapshot_;
    AudioPcmStreamQueueSnapshot pcm_stream_queue_snapshot_;
    std::uint32_t generation_seed_;
    bool is_initialized_;
};
}
