#include "yuengine/kernel/engine_kernel.h"

#include <unordered_map>

namespace yuengine::kernel {
namespace {
constexpr const char* KERNEL_START_TRACE = "kernel.start";
constexpr const char* KERNEL_UPDATE_TRACE = "kernel.update";
constexpr const char* KERNEL_SHUTDOWN_TRACE = "kernel.shutdown";
constexpr const char* DUPLICATE_MODULE_MESSAGE = "duplicate module";
constexpr const char* UNRESOLVED_DEPENDENCY_MESSAGE = "module dependency order could not be resolved";
constexpr const char* MISSING_REQUIRED_SERVICE_MESSAGE = "required service was not available before module startup";
constexpr const char* STARTUP_TEARDOWN_MESSAGE = "module startup failed after deterministic teardown";
constexpr const char* UPDATE_TEARDOWN_MESSAGE = "module update failed after dependent teardown";
constexpr const char* SHUTDOWN_FAILURE_MESSAGE = "module shutdown failed";
constexpr const char* INVALID_LIFECYCLE_MESSAGE = "kernel lifecycle call was out of order";
}

EngineKernel::EngineKernel() {
    _services.CloseRegistrationWindow();
}

bool EngineKernel::RegisterModule(IModule& module) {
    if (_running) {
        return false;
    }

    const std::string moduleName(module.Name());
    if (!_moduleByName.contains(moduleName)) {
        _moduleByName.emplace(moduleName, &module);
    }

    _modules.push_back(&module);
    return true;
}

KernelResult EngineKernel::Start(std::vector<std::string>& lifecycleTrace) {
    if (_running) {
        return KernelResult::Failure(KERNEL_STATUS::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_START_TRACE);

    std::unordered_map<std::string_view, IModule*> moduleByName;
    moduleByName.reserve(_modules.size());
    _startedModules.reserve(_modules.size());

    for (IModule* module : _modules) {
        const std::string_view moduleName = module->Name();
        if (moduleByName.contains(moduleName)) {
            return KernelResult::Failure(KERNEL_STATUS::DuplicateModule, DUPLICATE_MODULE_MESSAGE);
        }

        moduleByName.emplace(moduleName, module);
    }

    _services.OpenRegistrationWindow();

    while (_startedModules.size() < _modules.size()) {
        bool progressed = false;

        for (IModule* module : _modules) {
            if (IsStarted(module->Name())) {
                continue;
            }

            if (!DependenciesStarted(*module)) {
                continue;
            }

            if (!RequiredServicesAvailable(*module)) {
                const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
                if (!teardownResult.Succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(KernelResult::Failure(KERNEL_STATUS::MissingService, MISSING_REQUIRED_SERVICE_MESSAGE));
            }

            const KernelResult startResult = module->Start(_services, lifecycleTrace);
            if (!startResult.Succeeded) {
                const KernelResult failedModuleCleanupResult = module->Shutdown(lifecycleTrace);
                _services.UnregisterOwner(module->Name());

                const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
                if (!failedModuleCleanupResult.Succeeded) {
                    return CompleteStartupAttempt(KernelResult::Failure(KERNEL_STATUS::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE));
                }

                if (!teardownResult.Succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(KernelResult::Failure(startResult.Status, STARTUP_TEARDOWN_MESSAGE));
            }

            _startedModules.push_back(module);
            progressed = true;
        }

        if (!progressed) {
            const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
            if (!teardownResult.Succeeded) {
                return CompleteStartupAttempt(teardownResult);
            }

            return CompleteStartupAttempt(KernelResult::Failure(KERNEL_STATUS::DependencyFailure, UNRESOLVED_DEPENDENCY_MESSAGE));
        }
    }

    _running = true;
    return CompleteStartupAttempt(KernelResult::Success());
}

KernelResult EngineKernel::Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) {
    if (!_running) {
        return KernelResult::Failure(KERNEL_STATUS::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_UPDATE_TRACE);

    for (std::size_t moduleIndex = 0U; moduleIndex < _startedModules.size(); ++moduleIndex) {
        IModule* module = _startedModules[moduleIndex];
        const KernelResult updateResult = module->Update(frameIndex, tickTimeNanoseconds, lifecycleTrace);
        if (!updateResult.Succeeded) {
            const KernelResult teardownResult = ShutdownFailedAndDependents(module->Name(), lifecycleTrace);
            if (!teardownResult.Succeeded) {
                return teardownResult;
            }

            return KernelResult::Failure(KERNEL_STATUS::UpdateFailure, UPDATE_TEARDOWN_MESSAGE);
        }
    }

    return KernelResult::Success();
}

KernelResult EngineKernel::Shutdown(std::vector<std::string>& lifecycleTrace) {
    if (!_running) {
        return KernelResult::Failure(KERNEL_STATUS::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_SHUTDOWN_TRACE);
    const KernelResult shutdownResult = ShutdownStarted(lifecycleTrace);
    _running = false;
    return shutdownResult;
}

ServiceRegistry& EngineKernel::Services() {
    return _services;
}

const ServiceRegistry& EngineKernel::Services() const {
    return _services;
}

bool EngineKernel::IsStarted(std::string_view moduleName) const {
    for (const IModule* module : _startedModules) {
        if (module->Name() == moduleName) {
            return true;
        }
    }

    return false;
}

bool EngineKernel::DependenciesStarted(const IModule& module) const {
    const std::vector<std::string_view> dependencies = module.Dependencies();
    for (const std::string_view dependency : dependencies) {
        if (!IsStarted(dependency)) {
            return false;
        }
    }

    return true;
}

bool EngineKernel::RequiredServicesAvailable(const IModule& module) const {
    const std::vector<std::string_view> requiredServices = module.RequiredServices();
    for (const std::string_view requiredService : requiredServices) {
        if (!RequiredDependencyChainPublishesService(module, requiredService)) {
            return false;
        }

        if (!_services.Contains(requiredService)) {
            return false;
        }
    }

    return true;
}

bool EngineKernel::ModulePublishesService(const IModule& module, std::string_view serviceId) const {
    const std::vector<std::string_view> publishedServices = module.PublishedServices();
    for (const std::string_view publishedService : publishedServices) {
        if (publishedService == serviceId) {
            return true;
        }
    }

    return false;
}

bool EngineKernel::RequiredDependencyChainPublishesService(const IModule& module, std::string_view serviceId) const {
    const std::vector<std::string_view> dependencies = module.Dependencies();
    for (const std::string_view dependency : dependencies) {
        const IModule* dependencyModule = FindModule(dependency);
        if (dependencyModule == nullptr) {
            continue;
        }

        if (ModulePublishesService(*dependencyModule, serviceId)) {
            return true;
        }

        if (RequiredDependencyChainPublishesService(*dependencyModule, serviceId)) {
            return true;
        }
    }

    return false;
}

bool EngineKernel::DependencyChainContains(const IModule& module, std::string_view dependencyName) const {
    const std::vector<std::string_view> dependencies = module.Dependencies();
    for (const std::string_view dependency : dependencies) {
        if (dependency == dependencyName) {
            return true;
        }

        const IModule* dependencyModule = FindModule(dependency);
        if (dependencyModule == nullptr) {
            continue;
        }

        if (DependencyChainContains(*dependencyModule, dependencyName)) {
            return true;
        }
    }

    return false;
}

const IModule* EngineKernel::FindModule(std::string_view moduleName) const {
    const auto moduleIterator = _moduleByName.find(std::string(moduleName));
    if (moduleIterator == _moduleByName.end()) {
        return nullptr;
    }

    return moduleIterator->second;
}

KernelResult EngineKernel::CompleteStartupAttempt(KernelResult result) {
    _services.CloseRegistrationWindow();
    return result;
}

KernelResult EngineKernel::ShutdownStarted(std::vector<std::string>& lifecycleTrace) {
    return ShutdownStartedFrom(0U, lifecycleTrace);
}

KernelResult EngineKernel::ShutdownStartedFrom(std::size_t startIndex, std::vector<std::string>& lifecycleTrace) {
    KernelResult finalResult = KernelResult::Success();

    while (_startedModules.size() > startIndex) {
        IModule* module = _startedModules.back();
        const KernelResult shutdownResult = module->Shutdown(lifecycleTrace);
        _services.UnregisterOwner(module->Name());
        _startedModules.pop_back();

        if (!shutdownResult.Succeeded && finalResult.Succeeded) {
            finalResult = KernelResult::Failure(KERNEL_STATUS::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    return finalResult;
}

KernelResult EngineKernel::ShutdownFailedAndDependents(std::string_view failedModuleName, std::vector<std::string>& lifecycleTrace) {
    KernelResult finalResult = KernelResult::Success();
    std::vector<IModule*> remainingModules;
    remainingModules.reserve(_startedModules.size());

    for (IModule* module : _startedModules) {
        if (module->Name() == failedModuleName) {
            continue;
        }

        if (DependencyChainContains(*module, failedModuleName)) {
            continue;
        }

        remainingModules.push_back(module);
    }

    for (std::size_t reverseIndex = _startedModules.size(); reverseIndex > 0U; --reverseIndex) {
        IModule* module = _startedModules[reverseIndex - 1U];
        if (module->Name() != failedModuleName && !DependencyChainContains(*module, failedModuleName)) {
            continue;
        }

        const KernelResult shutdownResult = module->Shutdown(lifecycleTrace);
        _services.UnregisterOwner(module->Name());

        if (!shutdownResult.Succeeded && finalResult.Succeeded) {
            finalResult = KernelResult::Failure(KERNEL_STATUS::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    _startedModules = remainingModules;
    return finalResult;
}
}
