// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Src/EngineKernel.cpp

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

    const std::string module_name(module.Name());
    if (!module_by_name_.contains(module_name)) {
        module_by_name_.emplace(module_name, &module);
    }

    modules_.push_back(&module);
    return true;
}

KernelResult EngineKernel::Start(std::vector<std::string>& lifecycle_trace) {
    if (running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycle_trace.push_back(KERNEL_START_TRACE);

    std::unordered_map<std::string_view, IModule*> module_by_name;
    module_by_name.reserve(modules_.size());
    started_modules_.reserve(modules_.size());

    for (IModule* module : modules_) {
        const std::string_view module_name = module->Name();
        if (module_by_name.contains(module_name)) {
            return KernelResult::Failure(KernelStatus::DuplicateModule, DUPLICATE_MODULE_MESSAGE);
        }

        module_by_name.emplace(module_name, module);
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
                const KernelResult teardown_result = ShutdownStarted(lifecycle_trace);
                if (!teardown_result.succeeded) {
                    return CompleteStartupAttempt(teardown_result);
                }

                return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::MissingService, MISSING_REQUIRED_SERVICE_MESSAGE));
            }

            const KernelResult start_result = module->Start(services_, lifecycle_trace);
            if (!start_result.succeeded) {
                const KernelResult failed_module_cleanup_result = module->Shutdown(lifecycle_trace);
                services_.UnregisterOwner(module->Name());

                const KernelResult teardown_result = ShutdownStarted(lifecycle_trace);
                if (!failed_module_cleanup_result.succeeded) {
                    return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE));
                }

                if (!teardown_result.succeeded) {
                    return CompleteStartupAttempt(teardown_result);
                }

                return CompleteStartupAttempt(KernelResult::Failure(start_result.status, STARTUP_TEARDOWN_MESSAGE));
            }

            started_modules_.push_back(module);
            progressed = true;
        }

        if (!progressed) {
            const KernelResult teardown_result = ShutdownStarted(lifecycle_trace);
            if (!teardown_result.succeeded) {
                return CompleteStartupAttempt(teardown_result);
            }

            return CompleteStartupAttempt(KernelResult::Failure(KernelStatus::DependencyFailure, UNRESOLVED_DEPENDENCY_MESSAGE));
        }
    }

    running_ = true;
    return CompleteStartupAttempt(KernelResult::Success());
}

KernelResult EngineKernel::Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) {
    if (!running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycle_trace.push_back(KERNEL_UPDATE_TRACE);

    for (std::size_t module_index = 0U; module_index < started_modules_.size(); ++module_index) {
        IModule* module = started_modules_[module_index];
        const KernelResult update_result = module->Update(frame_index, tick_time_nanoseconds, lifecycle_trace);
        if (!update_result.succeeded) {
            const KernelResult teardown_result = ShutdownFailedAndDependents(module->Name(), lifecycle_trace);
            if (!teardown_result.succeeded) {
                return teardown_result;
            }

            return KernelResult::Failure(KernelStatus::UpdateFailure, UPDATE_TEARDOWN_MESSAGE);
        }
    }

    return KernelResult::Success();
}

KernelResult EngineKernel::Shutdown(std::vector<std::string>& lifecycle_trace) {
    if (!running_) {
        return KernelResult::Failure(KernelStatus::InvalidLifecycle, INVALID_LIFECYCLE_MESSAGE);
    }

    lifecycle_trace.push_back(KERNEL_SHUTDOWN_TRACE);
    const KernelResult shutdown_result = ShutdownStarted(lifecycle_trace);
    running_ = false;
    return shutdown_result;
}

ServiceRegistry& EngineKernel::Services() {
    return services_;
}

const ServiceRegistry& EngineKernel::Services() const {
    return services_;
}

bool EngineKernel::IsStarted(std::string_view module_name) const {
    for (const IModule* module : started_modules_) {
        if (module->Name() == module_name) {
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
    const std::vector<std::string_view> required_services = module.RequiredServices();
    for (const std::string_view required_service : required_services) {
        if (!RequiredDependencyChainPublishesService(module, required_service)) {
            return false;
        }

        if (!services_.Contains(required_service)) {
            return false;
        }
    }

    return true;
}

bool EngineKernel::ModulePublishesService(const IModule& module, std::string_view service_id) const {
    const std::vector<std::string_view> published_services = module.PublishedServices();
    for (const std::string_view published_service : published_services) {
        if (published_service == service_id) {
            return true;
        }
    }

    return false;
}

bool EngineKernel::RequiredDependencyChainPublishesService(const IModule& module, std::string_view service_id) const {
    const std::vector<std::string_view> dependencies = module.Dependencies();
    for (const std::string_view dependency : dependencies) {
        const IModule* dependency_module = FindModule(dependency);
        if (dependency_module == nullptr) {
            continue;
        }

        if (ModulePublishesService(*dependency_module, service_id)) {
            return true;
        }

        if (RequiredDependencyChainPublishesService(*dependency_module, service_id)) {
            return true;
        }
    }

    return false;
}

bool EngineKernel::DependencyChainContains(const IModule& module, std::string_view dependency_name) const {
    const std::vector<std::string_view> dependencies = module.Dependencies();
    for (const std::string_view dependency : dependencies) {
        if (dependency == dependency_name) {
            return true;
        }

        const IModule* dependency_module = FindModule(dependency);
        if (dependency_module == nullptr) {
            continue;
        }

        if (DependencyChainContains(*dependency_module, dependency_name)) {
            return true;
        }
    }

    return false;
}

const IModule* EngineKernel::FindModule(std::string_view module_name) const {
    const auto module_iterator = module_by_name_.find(std::string(module_name));
    if (module_iterator == module_by_name_.end()) {
        return nullptr;
    }

    return module_iterator->second;
}

KernelResult EngineKernel::CompleteStartupAttempt(KernelResult result) {
    services_.CloseRegistrationWindow();
    return result;
}

KernelResult EngineKernel::ShutdownStarted(std::vector<std::string>& lifecycle_trace) {
    return ShutdownStartedFrom(0U, lifecycle_trace);
}

KernelResult EngineKernel::ShutdownStartedFrom(std::size_t start_index, std::vector<std::string>& lifecycle_trace) {
    KernelResult final_result = KernelResult::Success();

    while (started_modules_.size() > start_index) {
        IModule* module = started_modules_.back();
        const KernelResult shutdown_result = module->Shutdown(lifecycle_trace);
        services_.UnregisterOwner(module->Name());
        started_modules_.pop_back();

        if (!shutdown_result.succeeded && final_result.succeeded) {
            final_result = KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    return final_result;
}

KernelResult EngineKernel::ShutdownFailedAndDependents(std::string_view failed_module_name, std::vector<std::string>& lifecycle_trace) {
    KernelResult final_result = KernelResult::Success();
    std::vector<IModule*> remaining_modules;
    remaining_modules.reserve(started_modules_.size());

    for (IModule* module : started_modules_) {
        if (module->Name() == failed_module_name) {
            continue;
        }

        if (DependencyChainContains(*module, failed_module_name)) {
            continue;
        }

        remaining_modules.push_back(module);
    }

    for (std::size_t reverse_index = started_modules_.size(); reverse_index > 0U; --reverse_index) {
        IModule* module = started_modules_[reverse_index - 1U];
        if (module->Name() != failed_module_name && !DependencyChainContains(*module, failed_module_name)) {
            continue;
        }

        const KernelResult shutdown_result = module->Shutdown(lifecycle_trace);
        services_.UnregisterOwner(module->Name());

        if (!shutdown_result.succeeded && final_result.succeeded) {
            final_result = KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }
    }

    started_modules_ = remaining_modules;
    return final_result;
}
}
