// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneContractQueue.h

#pragma once

#include <cstddef>
#include <span>

#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/AudioScene/AudioSceneSnapshot.h"
#include "YuEngine/AudioScene/AudioSceneSourceRecord.h"
#include "YuEngine/AudioScene/AudioSceneStatus.h"
#include "YuEngine/AudioScene/AudioSceneSubmitRequest.h"
#include "YuEngine/AudioScene/AudioSceneSubmitResult.h"

namespace yuengine::audioscene {
class AudioSceneContractQueue final {
public:
    /**
     * @comment 将 L1 audio source 值记录转换为 PCM stream queue 请求。
     * @param request 输入 audio scene submit 请求。
     * @param out_requests 调用方持有的输出 queue request storage。
     * @param out_result 输出 submit 结果。
     * @return 显式操作状态。
     */
    AudioSceneStatus SubmitSourceUpdates(
        const AudioSceneSubmitRequest &request,
        std::span<yuengine::audio::AudioPcmStreamQueueRequest> out_requests,
        AudioSceneSubmitResult *out_result);

    /**
     * @comment 返回当前 contract queue 快照。
     * @return 快照值。
     */
    AudioSceneSnapshot Snapshot() const;

private:
    AudioSceneStatus ValidateRequest(
        const AudioSceneSubmitRequest &request,
        std::span<yuengine::audio::AudioPcmStreamQueueRequest> out_requests,
        std::size_t *out_active_source_count,
        std::size_t *out_playing_source_count) const;
    AudioSceneStatus ValidateSource(const AudioSceneSourceRecord &source) const;
    bool IsKnownSourceState(AudioSceneSourceState state) const;
    bool IsBusIdValid(std::uint32_t bus_id) const;
    bool IsPlayingSource(const AudioSceneSourceRecord &source) const;
    bool IsPacketHandleValid(yuengine::audio::AudioPcmSamplePacketHandle packet) const;
    bool IsSourceIdValid(yuengine::audio::AudioSourceId source) const;
    std::uint32_t BuildBusQueueId(const AudioSceneSourceRecord &source) const;
    void FillQueueRequest(
        const AudioSceneSubmitRequest &request,
        const AudioSceneSourceRecord &source,
        yuengine::audio::AudioPcmStreamQueueRequest *out_request) const;
    AudioSceneStatus RecordSuccess(const AudioSceneSubmitResult &result);
    AudioSceneStatus RecordFailure(AudioSceneStatus status, const AudioSceneSubmitResult &result);

    AudioSceneSnapshot snapshot_{};
};
}
