// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryStatus.h

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
