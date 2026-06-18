// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/FixedFrameClock.h

#pragma once

#include <cstdint>

#include "YuEngine/Platform/IFrameClock.h"

namespace yuengine::platform {
class FixedFrameClock final : public IFrameClock {
public:
    /**
     * @comment 构造 FixedFrameClock 实例。
     * @param first_tick_nanoseconds 输入首个 tick 的纳秒值。
     * @param step_nanoseconds 输入 step nanoseconds。
     */
    FixedFrameClock(std::uint64_t first_tick_nanoseconds, std::uint64_t step_nanoseconds);

    /**
     * @comment 返回下一次 tick 的纳秒时间。
     * @return 下一次 tick 的纳秒值。
     */
    std::uint64_t NextTickNanoseconds() override;

private:
    std::uint64_t next_tick_nanoseconds_;
    std::uint64_t step_nanoseconds_;
};
}
