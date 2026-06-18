// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/TestAudioDevice.h

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
     * @comment Constructs a TestAudioDevice instance.
     */
    TestAudioDevice();

    /**
     * @comment Initializes the instance.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    AudioStatus Initialize(const AudioDeviceDesc& desc);
    /**
     * @comment Registers synthetic source.
     * @param interleaved_samples Input interleaved samples.
     * @param frame_count Input frame count.
     * @param out_source Output source written on success.
     * @return Explicit operation status.
     */
    AudioStatus RegisterSyntheticSource(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count, AudioSourceId& out_source);
    /**
     * @comment Creates a bounded PCM sample packet metadata record.
     * @param request Input packet request.
     * @param out_packet Output packet handle written on success.
     * @return Explicit operation status.
     */
    AudioStatus CreatePcmSamplePacket(const AudioPcmSamplePacketRequest& request, AudioPcmSamplePacketHandle& out_packet);
    /**
     * @comment Queries a PCM sample packet metadata record.
     * @param packet Input packet handle.
     * @param out_record Output packet record written on success.
     * @return Explicit operation status.
     */
    AudioStatus QueryPcmSamplePacket(AudioPcmSamplePacketHandle packet, AudioPcmSamplePacketRecord& out_record);
    /**
     * @comment Releases a PCM sample packet metadata record.
     * @param packet Input packet handle.
     * @return Explicit operation status.
     */
    AudioStatus ReleasePcmSamplePacket(AudioPcmSamplePacketHandle packet);
    /**
     * @comment Creates a bounded PCM stream queue metadata record.
     * @param request Input queue request.
     * @param out_queue Output queue handle written on success.
     * @return Explicit operation status.
     */
    AudioStatus CreatePcmStreamQueue(const AudioPcmStreamQueueRequest& request, AudioPcmStreamQueueHandle& out_queue);
    /**
     * @comment Queries a PCM stream queue metadata record.
     * @param queue Input queue handle.
     * @param out_record Output queue record written on success.
     * @return Explicit operation status.
     */
    AudioStatus QueryPcmStreamQueue(AudioPcmStreamQueueHandle queue, AudioPcmStreamQueueRecord& out_record);
    /**
     * @comment Drains PCM stream queue chunk descriptors into caller-owned storage.
     * @param queue Input queue handle.
     * @param out_chunks Output chunk descriptor storage.
     * @param out_chunk_count Output chunk count written by the function.
     * @return Explicit operation status.
     */
    AudioStatus DrainPcmStreamQueue(AudioPcmStreamQueueHandle queue, std::span<AudioPcmStreamQueueChunk> out_chunks, std::size_t& out_chunk_count);
    /**
     * @comment Releases a PCM stream queue metadata record.
     * @param queue Input queue handle.
     * @return Explicit operation status.
     */
    AudioStatus ReleasePcmStreamQueue(AudioPcmStreamQueueHandle queue);
    /**
     * @comment Starts voice.
     * @param source Input source.
     * @param gain_q15 Input gain q15.
     * @param out_voice Output voice written on success.
     * @return Explicit operation status.
     */
    AudioStatus StartVoice(AudioSourceId source, std::uint32_t gain_q15, AudioVoiceHandle& out_voice);
    /**
     * @comment Stops voice.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    AudioStatus StopVoice(AudioVoiceHandle handle);
    /**
     * @comment Mixes requested samples into the output buffer.
     * @param output_samples Output sample buffer updated by the function.
     * @param requested_frames Input requested frames.
     * @return Explicit operation result.
     */
    AudioMixResult Mix(std::span<std::int16_t> output_samples, std::size_t requested_frames);
    /**
     * @comment Returns the supported capabilities.
     * @return Capability data.
     */
    AudioCapabilities Capabilities() const;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    AudioDeviceSnapshot Snapshot() const;
    /**
     * @comment Returns a snapshot of PCM sample packet state.
     * @return Snapshot value.
     */
    AudioPcmSamplePacketSnapshot PcmSamplePacketSnapshot() const;
    /**
     * @comment Returns a snapshot of PCM stream queue state.
     * @return Snapshot value.
     */
    AudioPcmStreamQueueSnapshot PcmStreamQueueSnapshot() const;

private:
    AudioStatus RecordFailure(AudioStatus status);
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
