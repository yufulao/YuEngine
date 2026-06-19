// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputCommandSnapshot.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputCommandRecord.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputCommandSnapshot final {
    std::uint64_t frame_index = 0U;
    InputStatus status = InputStatus::Success;
    std::array<InputCommandRecord, MAX_INPUT_COMMAND_RECORDS> commands{};
    std::size_t command_count = 0U;
};
}
