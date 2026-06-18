// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldKernelModule.h

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Kernel/IModule.h"
#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/ServiceRegistry.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldKernelModuleDesc.h"

namespace yuengine::world {
class WorldKernelModule final : public kernel::IModule {
public:
    /**
     * @comment 构造 WorldKernelModule adapter for one WorldInstance。
     * @param world module lifecycle 更新的 World instance。
     * @param desc 输入 adapter descriptor。
     */
    explicit WorldKernelModule(WorldInstance &world, WorldKernelModuleDesc desc=WorldKernelModuleDesc{});

    /**
     * @comment 返回 stable Kernel module name。
     * @return Module name 值。
     */
    std::string_view Name() const override;
    /**
     * @comment 返回声明的 Kernel module dependencies。
     * @return dependency identifiers 列表。
     */
    std::vector<std::string_view> Dependencies() const override;
    /**
     * @comment 返回 start 前要求的 service identifiers。
     * @return required service identifiers 列表。
     */
    std::vector<std::string_view> RequiredServices() const override;
    /**
     * @comment 返回 start 期间 published 的 service identifiers。
     * @return published service identifiers 列表。
     */
    std::vector<std::string_view> PublishedServices() const override;
    /**
     * @comment 通过 Kernel module lifecycle 启动 world。
     * @param service_registry 函数更新的 service registry。
     * @param lifecycle_trace 函数更新的 lifecycle trace。
     * @return 显式 Kernel 操作结果。
     */
    kernel::KernelResult Start(kernel::ServiceRegistry &service_registry, std::vector<std::string> &lifecycle_trace) override;
    /**
     * @comment 通过 Kernel module lifecycle 更新 world。
     * @param frame_index 输入 frame index。
     * @param tick_time_nanoseconds 输入 tick time nanoseconds。
     * @param lifecycle_trace 函数更新的 lifecycle trace。
     * @return 显式 Kernel 操作结果。
     */
    kernel::KernelResult Update(std::uint32_t frame_index,
        std::uint64_t tick_time_nanoseconds,
        std::vector<std::string> &lifecycle_trace) override;
    /**
     * @comment 通过 Kernel module lifecycle 停止 world。
     * @param lifecycle_trace 函数更新的 lifecycle trace。
     * @return 显式 Kernel 操作结果。
     */
    kernel::KernelResult Shutdown(std::vector<std::string> &lifecycle_trace) override;

private:
    const char *NormalizeText(const char *text, const char *fallback) const;
    kernel::KernelResult FailStart(const char *message) const;
    kernel::KernelResult FailServicePublication() const;
    kernel::KernelResult FailUpdate() const;
    kernel::KernelResult FailShutdown() const;

    WorldInstance &world_;
    const char *module_name_;
    const char *world_service_id_;
    std::uint64_t fixed_step_duration_;
    bool publish_world_service_;
    bool world_started_ = false;
};
}
