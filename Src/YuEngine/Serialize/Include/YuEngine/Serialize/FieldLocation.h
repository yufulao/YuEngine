// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/FieldLocation.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/SerializeTypeTag.h"

namespace yuengine::serialize {
struct FieldLocation final {
    SerializeTypeTag type = SerializeTypeTag::UInt32;
    std::uint32_t payload_offset = 0U;
    std::uint32_t payload_byte_count = 0U;
    bool found = false;
};
}
