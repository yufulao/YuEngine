// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/KernelHostRuntime.h

#pragma once

#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Platform/IHostRuntime.h"

namespace yuengine::kernel {
class KernelHostRuntime final : public platform::IHostRuntime {
public:
    /**
     * @comment 构造 KernelHostRuntime 实例。
     * @param kernel 函数写入的 Kernel。
     */
    explicit KernelHostRuntime(EngineKernel& kernel);

    /**
     * @comment 启动组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return Start 值。
     */
    platform::HostError Start(std::vector<std::string>& lifecycle_trace) override;
    /**
     * @comment 推进运行时一帧。
     * @param frame_index 输入 帧索引。
     * @param tick_time_nanoseconds 输入 tick 纳秒时间。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return Tick 值。
     */
    platform::HostError Tick(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override;
    /**
     * @comment 关闭组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 关闭 值。
     */
    platform::HostError Shutdown(std::vector<std::string>& lifecycle_trace) override;

private:
    EngineKernel& kernel_;
};
}
