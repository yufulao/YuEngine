#include "YuEngine/Kernel/EngineKernel.h"

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
    services_.CloseRegistrationWindow();
}

bool EngineKernel::RegisterModule(IModule& module) {
    if (running_) {
        return false;
    }

    const std::string moduleName(module.Name());
    if (!module_by_name_.contains(moduleName)) {
        module_by_name_.emplace(moduleName, &module);
    }

    modules_.push_back(&module);
    return true;
}

KernelResult EngineKernel::Start(std::vector<std::string>& lifecycleTrace) {
    if (running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_START_TRACE);

    std::unordered_map<std::string_view, IModule*> moduleByName;
    moduleByName.reserve(modules_.size());
    started_modules_.reserve(modules_.size());

    for (IModule* module : modules_) {
        const std::string_view moduleName = module->Name();
        if (moduleByName.contains(moduleName)) {
            return KernelResult::Failure(KernelStatus::DuplicateModule, DUPLICATE_MODULE_MESSAGE);
        }

        moduleByName.emplace(moduleName, module);
    }

    services_.OpenRegistrationWindow();

    while (started_modules_.size() < modules_.size()) {
        bool progressed = false;

        for (IModule* module : modules_) {
            if (IsStarted(module->Name())) {
                continue;
            }

            if (!DependenciesStarted(*module)) {
                continue;
            }

            if (!RequiredServicesAvailable(*module)) {
                const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
                if (!teardownResult.succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::MissingService, MISSING_REQUIRED_SERVICE_MESSAGE));
            }

            const KernelResult startResult = module->Start(services_, lifecycleTrace);
            if (!startResult.succeeded) {
                const KernelResult failedModuleCleanupResult = module->Shutdown(lifecycleTrace);
                services_.UnregisterOwner(module->Name());

                const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
                if (!failedModuleCleanupResult.succeeded) {
                    return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE));
                }

                if (!teardownResult.succeeded) {
                    return CompleteStartupAttempt(teardownResult);
                }

                return CompleteStartupAttempt(KernelResult::Failure(startResult.status, STARTUP_TEARDOWN_MESSAGE));
            }

            started_modules_.push_back(module);
            progressed = true;
        }

        if (!progressed) {
            const KernelResult teardownResult = ShutdownStarted(lifecycleTrace);
            if (!teardownResult.succeeded) {
                return CompleteStartupAttempt(teardownResult);
            }

            return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::DependencyFailure, UNRESOLVED_DEPENDENCY_MESSAGE));
        }
    }

    running_ = true;
    return CompleteStartupAttempt(KernelResult::Success());
}

KernelResult EngineKernel::Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) {
    if (!running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_UPDATE_TRACE);

    for (std::size_t moduleIndex = 0U; moduleIndex < started_modules_.size(); ++moduleIndex) {
        IModule* module = started_modules_[moduleIndex];
        const KernelResult updateResult = module->Update(frameIndex, tickTimeNanoseconds, lifecycleTrace);
        if (!updateResult.succeeded) {
            const KernelResult teardownResult = ShutdownFailedAndDependents(module->Name(), lifecycleTrace);
            if (!teardownResult.succeeded) {
                return teardownResult;
            }

            return KernelResult::Failure(KernelStatus::UpdateFailure, UPDATE_TEARDOWN_MESSAGE);
        }
    }

    return KernelResult::Success();
}

KernelResult EngineKernel::Shutdown(std::vector<std::string>& lifecycleTrace) {
    if (!running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycleTrace.push_back(KERNEL_SHUTDOWN_TRACE);
    const KernelResult shutdownResult = ShutdownStarted(lifecycleTrace);
    running_ = false;
    return shutdownResult;
}

ServiceRegistry& EngineKernel::Services() {
    return services_;
}

const ServiceRegistry& EngineKernel::Services() const {
    return services_;
}

bool EngineKernel::IsStarted(std::string_view moduleName) const {
    for (const IModule* module : started_modules_) {
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

        if (!services_.Contains(requiredService)) {
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
    const auto moduleIterator = module_by_name_.find(std::string(moduleName));
    if (moduleIterator == module_by_name_.end()) {
        return nullptr;
    }

    return moduleIterator->second;
}

KernelResult EngineKernel::CompleteStartupAttempt(KernelResult result) {
    services_.CloseRegistrationWindow();
    return result;
}

KernelResult EngineKernel::ShutdownStarted(std::vector<std::string>& lifecycleTrace) {
    return ShutdownStartedFrom(0U, lifecycleTrace);
}

KernelResult EngineKernel::ShutdownStartedFrom(std::size_t startIndex, std::vector<std::string>& lifecycleTrace) {
    KernelResult finalResult = KernelResult::Success();

    while (started_modules_.size() > startIndex) {
        IModule* module = started_modules_.back();
        const KernelResult shutdownResult = module->Shutdown(lifecycleTrace);
        services_.UnregisterOwner(module->Name());
        started_modules_.pop_back();

        if (!shutdownResult.succeeded && finalResult.succeeded) {
            finalResult = KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    return finalResult;
}

KernelResult EngineKernel::ShutdownFailedAndDependents(std::string_view failedModuleName, std::vector<std::string>& lifecycleTrace) {
    KernelResult finalResult = KernelResult::Success();
    std::vector<IModule*> remainingModules;
    remainingModules.reserve(started_modules_.size());

    for (IModule* module : started_modules_) {
        if (module->Name() == failedModuleName) {
            continue;
        }

        if (DependencyChainContains(*module, failedModuleName)) {
            continue;
        }

        remainingModules.push_back(module);
    }

    for (std::size_t reverseIndex = started_modules_.size(); reverseIndex > 0U; --reverseIndex) {
        IModule* module = started_modules_[reverseIndex - 1U];
        if (module->Name() != failedModuleName && !DependencyChainContains(*module, failedModuleName)) {
            continue;
        }

        const KernelResult shutdownResult = module->Shutdown(lifecycleTrace);
        services_.UnregisterOwner(module->Name());

        if (!shutdownResult.succeeded && finalResult.succeeded) {
            finalResult = KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    started_modules_ = remainingModules;
    return finalResult;
}
}
