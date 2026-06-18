// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentQueryStatus {
    Success,
    InvalidSourceBridge,
    InvalidOutputBuffer,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    OutputCapacityExceeded
};
}
