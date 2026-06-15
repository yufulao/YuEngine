// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiAccountingStatus.h

#pragma once

namespace yuengine::rhi {
enum class RhiAccountingStatus {
    DeferredUntilYuMemoryIntegration,
    UsesYuMemoryVocabulary
};
}
