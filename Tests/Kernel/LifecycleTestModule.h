#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "YuEngine/Kernel/IModule.h"

using yuengine::kernel::IModule;
using yuengine::kernel::KernelResult;
using yuengine::kernel::KernelStatus;
using yuengine::kernel::ServiceRegistry;

class LifecycleTestModule final : public IModule {
public:
    LifecycleTestModule(
        std::string name,
        std::vector<std::string_view> dependencies,
        bool failOnStart,
        bool failOnShutdown = false,
        bool verifyRequiredServicesOnShutdown = false)
        : _name(std::move(name)),
          _dependencies(std::move(dependencies)),
          _requiredServices(),
          _publishedServices(),
          _failOnStart(failOnStart),
          _failOnUpdate(false),
          _failOnShutdown(failOnShutdown),
          _verifyRequiredServicesOnShutdown(verifyRequiredServicesOnShutdown),
          _serviceRegistry(nullptr),
          _publishedServiceValue(0) {
    }

    LifecycleTestModule(
        std::string name,
        std::vector<std::string_view> dependencies,
        std::vector<std::string_view> requiredServices,
        std::vector<std::string_view> publishedServices,
        bool failOnStart,
        bool failOnUpdate,
        bool failOnShutdown = false,
        bool verifyRequiredServicesOnShutdown = false)
        : _name(std::move(name)),
          _dependencies(std::move(dependencies)),
          _requiredServices(std::move(requiredServices)),
          _publishedServices(std::move(publishedServices)),
          _failOnStart(failOnStart),
          _failOnUpdate(failOnUpdate),
          _failOnShutdown(failOnShutdown),
          _verifyRequiredServicesOnShutdown(verifyRequiredServicesOnShutdown),
          _serviceRegistry(nullptr),
          _publishedServiceValue(7) {
    }

    std::string_view Name() const override {
        return _name;
    }

    std::vector<std::string_view> Dependencies() const override {
        return _dependencies;
    }

    std::vector<std::string_view> RequiredServices() const override {
        return _requiredServices;
    }

    std::vector<std::string_view> PublishedServices() const override {
        return _publishedServices;
    }

    KernelResult Start(ServiceRegistry& serviceRegistry, std::vector<std::string>& lifecycleTrace) override {
        _serviceRegistry = &serviceRegistry;
        lifecycleTrace.push_back(std::string("module.start.") + _name);

        for (const std::string_view publishedService : _publishedServices) {
            const bool registered = serviceRegistry.Register<int>(_name, publishedService, _publishedServiceValue);
            if (!registered) {
                return KernelResult::Failure(KernelStatus::DuplicateService, DUPLICATE_SERVICE_MESSAGE);
            }
        }

        if (_failOnStart) {
            return KernelResult::Failure(KernelStatus::StartupFailure, STARTUP_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back(std::string("module.update.") + _name);

        if (_failOnUpdate) {
            return KernelResult::Failure(KernelStatus::UpdateFailure, UPDATE_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Shutdown(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back(std::string("module.shutdown.") + _name);
        if (_failOnShutdown) {
            return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }

        if (_verifyRequiredServicesOnShutdown) {
            const KernelResult serviceVerificationResult = VerifyRequiredServicesOnShutdown();
            if (!serviceVerificationResult.Succeeded) {
                return serviceVerificationResult;
            }
        }

        return KernelResult::Success();
    }

private:
    // Verifies teardown can still read dependency-published services.
    KernelResult VerifyRequiredServicesOnShutdown() const {
        if (_serviceRegistry == nullptr) {
            return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }

        for (const std::string_view requiredService : _requiredServices) {
            if (_serviceRegistry->Resolve<int>(requiredService) == nullptr) {
                return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
            }
        }

        return KernelResult::Success();
    }

    std::string _name;
    std::vector<std::string_view> _dependencies;
    std::vector<std::string_view> _requiredServices;
    std::vector<std::string_view> _publishedServices;
    bool _failOnStart;
    bool _failOnUpdate;
    bool _failOnShutdown;
    bool _verifyRequiredServicesOnShutdown;
    ServiceRegistry* _serviceRegistry;
    int _publishedServiceValue;
    static constexpr const char* DUPLICATE_SERVICE_MESSAGE = "duplicate service";
    static constexpr const char* STARTUP_FAILURE_MESSAGE = "module startup failed";
    static constexpr const char* UPDATE_FAILURE_MESSAGE = "module update failed";
    static constexpr const char* SHUTDOWN_FAILURE_MESSAGE = "module shutdown failed";
};
