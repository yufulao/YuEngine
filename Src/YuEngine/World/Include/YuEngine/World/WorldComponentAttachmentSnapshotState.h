// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldComponentAttachmentSnapshotState final {
    std::uint32_t attachment_record_count = 0U;
    std::uint32_t chunk_record_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
