// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneRecordValueStreamState.h

#pragma once

#include <cstdint>

namespace yuengine::world {
struct WorldSceneRecordValueStreamState final {
    std::uint32_t identity_record_count = 0U;
    std::uint32_t transform_record_count = 0U;
    std::uint32_t attachment_record_count = 0U;
    std::uint32_t binding_record_count = 0U;
    std::uint32_t committed_byte_count = 0U;
};
}
