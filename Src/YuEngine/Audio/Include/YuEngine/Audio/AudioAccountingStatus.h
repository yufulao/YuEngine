// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioAccountingStatus.h

#pragma once

namespace yuengine::audio {
enum class AudioAccountingStatus {
    DeferredUntilYuMemoryIntegration,
    UsesYuMemoryVocabulary
};
}
