// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/IHostRuntime.h

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "YuEngine/Platform/HostError.h"

namespace yuengine::platform {
class IHostRuntime {
public:
    virtual ~IHostRuntime() = default;

    /**
     * @comment 启动组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return Start 值。
     */
    virtual HostError Start(std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment 推进运行时一帧。
     * @param frame_index 输入 帧索引。
     * @param tick_time_nanoseconds 输入 tick 纳秒时间。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return Tick 值。
     */
    virtual HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment 关闭组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 关闭 值。
     */
    virtual HostError Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
