// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Include/YuEngine/AudioScene/AudioSceneStatus.h

#pragma once

namespace yuengine::audioscene {
enum class AudioSceneStatus {
    Success,
    NullPointer,
    InvalidFrameId,
    BackendUnavailable,
    MissingSource,
    MissingSoundAsset,
    MissingAudioReadyRecord,
    MissingAudioPacket,
    InvalidSourceRecord,
    InvalidGain,
    OutputCapacityExceeded
};
}
