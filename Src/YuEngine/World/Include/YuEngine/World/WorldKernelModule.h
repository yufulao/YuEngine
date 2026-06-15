// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldKernelModule.h

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
     * @comment Constructs a WorldKernelModule adapter for one WorldInstance.
     * @param world World instance updated by the module lifecycle.
     * @param desc Input adapter descriptor.
     */
    explicit WorldKernelModule(WorldInstance &world, WorldKernelModuleDesc desc=WorldKernelModuleDesc{});

    /**
     * @comment Returns the stable Kernel module name.
     * @return Module name value.
     */
    std::string_view Name() const override;
    /**
     * @comment Returns declared Kernel module dependencies.
     * @return Dependency identifiers.
     */
    std::vector<std::string_view> Dependencies() const override;
    /**
     * @comment Returns service identifiers required before start.
     * @return Required service identifiers.
     */
    std::vector<std::string_view> RequiredServices() const override;
    /**
     * @comment Returns service identifiers published during start.
     * @return Published service identifiers.
     */
    std::vector<std::string_view> PublishedServices() const override;
    /**
     * @comment Starts the world through the Kernel module lifecycle.
     * @param service_registry Service registry updated by the function.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit Kernel operation result.
     */
    kernel::KernelResult Start(kernel::ServiceRegistry &service_registry, std::vector<std::string> &lifecycle_trace) override;
    /**
     * @comment Updates the world through the Kernel module lifecycle.
     * @param frame_index Input frame index.
     * @param tick_time_nanoseconds Input tick time nanoseconds.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit Kernel operation result.
     */
    kernel::KernelResult Update(std::uint32_t frame_index,
        std::uint64_t tick_time_nanoseconds,
        std::vector<std::string> &lifecycle_trace) override;
    /**
     * @comment Stops the world through the Kernel module lifecycle.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit Kernel operation result.
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
