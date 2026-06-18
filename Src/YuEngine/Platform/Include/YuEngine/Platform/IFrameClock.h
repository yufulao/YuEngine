// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/IFrameClock.h

#pragma once

#include <cstdint>

namespace yuengine::platform {
class IFrameClock {
public:
    virtual ~IFrameClock() = default;

    /**
     * @comment 返回下一次 tick 的纳秒时间。
     * @return 下一次 tick 的纳秒值。
     */
    virtual std::uint64_t NextTickNanoseconds() = 0;
};
}
