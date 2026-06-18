// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/EngineKernel.h

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Kernel/IModule.h"
#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
class EngineKernel final {
public:
    /**
     * @comment 构造 EngineKernel 实例。
     */
    EngineKernel();

    /**
     * @comment 注册模块。
     * @param module 函数写入的 Module。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool RegisterModule(IModule& module);

    /**
     * @comment 启动组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    KernelResult Start(std::vector<std::string>& lifecycle_trace);
    /**
     * @comment 更新组件一帧。
     * @param frame_index 输入 帧索引。
     * @param tick_time_nanoseconds 输入 tick 纳秒时间。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace);
    /**
     * @comment 关闭组件。
     * @param lifecycle_trace 函数写入的生命周期轨迹。
     * @return 显式操作结果。
     */
    KernelResult Shutdown(std::vector<std::string>& lifecycle_trace);

    /**
     * @comment 返回服务注册表。
     * @return 请求对象的引用。
     */
    ServiceRegistry& Services();
    /**
     * @comment 返回服务注册表。
     * @return 请求对象的引用。
     */
    const ServiceRegistry& Services() const;

private:
    bool IsStarted(std::string_view module_name) const;
    bool DependenciesStarted(const IModule& module) const;
    bool RequiredServicesAvailable(const IModule& module) const;
    bool ModulePublishesService(const IModule& module, std::string_view service_id) const;
    bool RequiredDependencyChainPublishesService(const IModule& module, std::string_view service_id) const;
    bool DependencyChainContains(const IModule& module, std::string_view dependency_name) const;
    const IModule* FindModule(std::string_view module_name) const;
    KernelResult CompleteStartupAttempt(KernelResult result);
    KernelResult ShutdownStarted(std::vector<std::string>& lifecycle_trace);
    KernelResult ShutdownStartedFrom(std::size_t start_index, std::vector<std::string>& lifecycle_trace);
    KernelResult ShutdownFailedAndDependents(std::string_view failed_module_name, std::vector<std::string>& lifecycle_trace);

    std::vector<IModule*> modules_;
    std::vector<IModule*> started_modules_;
    ServiceRegistry services_;
    bool running_ = false;
    std::unordered_map<std::string, IModule*> module_by_name_;
};
}
