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

kernel_result_t EngineKernel::Start(std::vector<std::string>& lifecycleTrace) {
    if (_running) {
        return kernel_result_t::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_START_TRACE);

    std::unordered_map<std::string_view, IModule*> moduleByName;
    moduleByName.reserve(_modules.size());
    _startedModules.reserve(_modules.size());

    for (IModule* module : _modules) {
        const std::string_view moduleName = module->Name();
        if (moduleByName.contains(moduleName)) {
            return kernel_result_t::Failure(KernelStatus::DuplicateModule, DUPLICATE_MODULE_MESSAGE);
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
                const kernel_result_t teardownResult = ShutdownStarted(lifecycleTrace);
                if (!teardownResult.Succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(kernel_result_t::Failure(KernelStatus::MissingService, MISSING_REQUIRED_SERVICE_MESSAGE));
            }

            const kernel_result_t startResult = module->Start(_services, lifecycleTrace);
            if (!startResult.Succeeded) {
                const kernel_result_t failedModuleCleanupResult = module->Shutdown(lifecycleTrace);
                _services.UnregisterOwner(module->Name());

                const kernel_result_t teardownResult = ShutdownStarted(lifecycleTrace);
                if (!failedModuleCleanupResult.Succeeded) {
                    return CompleteStartupAttempt(kernel_result_t::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE));
                }

                if (!teardownResult.Succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(kernel_result_t::Failure(startResult.Status, STARTUP_TEARDOWN_MESSAGE));
            }

            _startedModules.push_back(module);
            progressed = true;
        }

        if (!progressed) {
            const kernel_result_t teardownResult = ShutdownStarted(lifecycleTrace);
            if (!teardownResult.Succeeded) {
                return CompleteStartupAttempt(teardownResult);
            }

            return CompleteStartupAttempt(kernel_result_t::Failure(KernelStatus::DependencyFailure, UNRESOLVED_DEPENDENCY_MESSAGE));
        }
    }

    _running = true;
    return CompleteStartupAttempt(kernel_result_t::Success());
}

kernel_result_t EngineKernel::Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) {
    if (!_running) {
        return kernel_result_t::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_UPDATE_TRACE);

    for (std::size_t moduleIndex = 0U; moduleIndex < _startedModules.size(); ++moduleIndex) {
        IModule* module = _startedModules[moduleIndex];
        const kernel_result_t updateResult = module->Update(frameIndex, tickTimeNanoseconds, lifecycleTrace);
        if (!updateResult.Succeeded) {
            const kernel_result_t teardownResult = ShutdownFailedAndDependents(module->Name(), lifecycleTrace);
            if (!teardownResult.Succeeded) {
                return teardownResult;
            }

            return kernel_result_t::Failure(KernelStatus::UpdateFailure, UPDATE_TEARDOWN_MESSAGE);
        }
    }

    return kernel_result_t::Success();
}

kernel_result_t EngineKernel::Shutdown(std::vector<std::string>& lifecycleTrace) {
    if (!_running) {
        return kernel_result_t::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_SHUTDOWN_TRACE);
    const kernel_result_t shutdownResult = ShutdownStarted(lifecycleTrace);
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

kernel_result_t EngineKernel::CompleteStartupAttempt(kernel_result_t result) {
    _services.CloseRegistrationWindow();
    return result;
}

kernel_result_t EngineKernel::ShutdownStarted(std::vector<std::string>& lifecycleTrace) {
    return ShutdownStartedFrom(0U, lifecycleTrace);
}

kernel_result_t EngineKernel::ShutdownStartedFrom(std::size_t startIndex, std::vector<std::string>& lifecycleTrace) {
    kernel_result_t finalResult = kernel_result_t::Success();

    while (_startedModules.size() > startIndex) {
        IModule* module = _startedModules.back();
        const kernel_result_t shutdownResult = module->Shutdown(lifecycleTrace);
        _services.UnregisterOwner(module->Name());
        _startedModules.pop_back();

        if (!shutdownResult.Succeeded && finalResult.Succeeded) {
            finalResult = kernel_result_t::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    return finalResult;
}

kernel_result_t EngineKernel::ShutdownFailedAndDependents(std::string_view failedModuleName, std::vector<std::string>& lifecycleTrace) {
    kernel_result_t finalResult = kernel_result_t::Success();
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

        const kernel_result_t shutdownResult = module->Shutdown(lifecycleTrace);
        _services.UnregisterOwner(module->Name());

        if (!shutdownResult.Succeeded && finalResult.Succeeded) {
            finalResult = kernel_result_t::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    _startedModules = remainingModules;
    return finalResult;
}
}
