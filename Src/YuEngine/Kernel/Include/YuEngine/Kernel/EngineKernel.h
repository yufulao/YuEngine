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
    EngineKernel();

    bool RegisterModule(IModule& module);

    KernelResult Start(std::vector<std::string>& lifecycle_trace);
    KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace);
    KernelResult Shutdown(std::vector<std::string>& lifecycle_trace);

    ServiceRegistry& Services();
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
