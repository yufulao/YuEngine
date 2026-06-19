// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeFrameInputSnapshotRef.h

#pragma once

#include <cstdint>

namespace yuengine::kernel {
struct RuntimeFrameInputSnapshotRef {
    const void* snapshot = nullptr;
    std::uint32_t snapshot_version = 0U;
};
}
