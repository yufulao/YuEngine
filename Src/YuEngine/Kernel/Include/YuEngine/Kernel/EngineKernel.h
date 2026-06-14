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

    KernelResult Start(std::vector<std::string>& lifecycleTrace);
    KernelResult Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace);
    KernelResult Shutdown(std::vector<std::string>& lifecycleTrace);

    ServiceRegistry& Services();
    const ServiceRegistry& Services() const;

private:
    bool IsStarted(std::string_view moduleName) const;
    bool DependenciesStarted(const IModule& module) const;
    bool RequiredServicesAvailable(const IModule& module) const;
    bool ModulePublishesService(const IModule& module, std::string_view serviceId) const;
    bool RequiredDependencyChainPublishesService(const IModule& module, std::string_view serviceId) const;
    bool DependencyChainContains(const IModule& module, std::string_view dependencyName) const;
    const IModule* FindModule(std::string_view moduleName) const;
    KernelResult CompleteStartupAttempt(KernelResult result);
    KernelResult ShutdownStarted(std::vector<std::string>& lifecycleTrace);
    KernelResult ShutdownStartedFrom(std::size_t startIndex, std::vector<std::string>& lifecycleTrace);
    KernelResult ShutdownFailedAndDependents(std::string_view failedModuleName, std::vector<std::string>& lifecycleTrace);

    std::vector<IModule*> modules_;
    std::vector<IModule*> started_modules_;
    ServiceRegistry services_;
    bool running_ = false;
    std::unordered_map<std::string, IModule*> module_by_name_;
};
}
