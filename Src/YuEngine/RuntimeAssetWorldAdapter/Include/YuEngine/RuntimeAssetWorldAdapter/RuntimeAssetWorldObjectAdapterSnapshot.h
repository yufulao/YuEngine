// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterSnapshot final {
    std::uint64_t build_attempt_count = 0U;
    std::uint64_t built_identity_count = 0U;
    std::uint64_t built_transform_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    std::uint64_t rejected_record_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    RuntimeAssetWorldObjectAdapterStatus last_status = RuntimeAssetWorldObjectAdapterStatus::Success;
};
}
