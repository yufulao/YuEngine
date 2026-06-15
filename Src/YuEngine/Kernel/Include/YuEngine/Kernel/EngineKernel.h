// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/EngineKernel.h

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
     * @comment Constructs a EngineKernel instance.
     */
    EngineKernel();

    /**
     * @comment Registers module.
     * @param module Module updated by the function.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool RegisterModule(IModule& module);

    /**
     * @comment Starts the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    KernelResult Start(std::vector<std::string>& lifecycle_trace);
    /**
     * @comment Updates the component for one frame.
     * @param frame_index Input frame index.
     * @param tick_time_nanoseconds Input tick time nanoseconds.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace);
    /**
     * @comment Shuts down the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    KernelResult Shutdown(std::vector<std::string>& lifecycle_trace);

    /**
     * @comment Returns the service registry.
     * @return Reference to the requested object.
     */
    ServiceRegistry& Services();
    /**
     * @comment Returns the service registry.
     * @return Reference to the requested object.
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
