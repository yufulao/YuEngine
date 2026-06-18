// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/IModule.h

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * @comment 返回 模块名。
     * @return Name 值。
     */
    virtual std::string_view Name() const = 0;
    /**
     * @comment 返回 声明的模块依赖。
     * @return Dependencies 值。
     */
    virtual std::vector<std::string_view> Dependencies() const = 0;
    /**
     * @comment 返回 所需 服务 identifiers。
     * @return 所需 services 值。
     */
    virtual std::vector<std::string_view> RequiredServices() const = 0;
    /**
     * @comment 返回已发布的服务标识符。
     * @return 已发布的服务列表。
     */
    virtual std::vector<std::string_view> PublishedServices() const = 0;
    /**
     * @comment 启动组件。
     * @param service_registry 函数写入的 Service registry。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    virtual KernelResult Start(ServiceRegistry& service_registry, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment 更新组件一帧。
     * @param frame_index 输入 帧索引。
     * @param tick_time_nanoseconds 输入 tick 纳秒时间。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    virtual KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment 关闭组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    virtual KernelResult Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
