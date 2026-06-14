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
        : name_(std::move(name)),
          dependencies_(std::move(dependencies)),
          required_services_(),
          published_services_(),
          fail_on_start_(failOnStart),
          fail_on_update_(false),
          fail_on_shutdown_(failOnShutdown),
          verify_required_services_on_shutdown_(verifyRequiredServicesOnShutdown),
          service_registry_(nullptr),
          published_service_value_(0) {
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
        : name_(std::move(name)),
          dependencies_(std::move(dependencies)),
          required_services_(std::move(requiredServices)),
          published_services_(std::move(publishedServices)),
          fail_on_start_(failOnStart),
          fail_on_update_(failOnUpdate),
          fail_on_shutdown_(failOnShutdown),
          verify_required_services_on_shutdown_(verifyRequiredServicesOnShutdown),
          service_registry_(nullptr),
          published_service_value_(7) {
    }

    std::string_view Name() const override {
        return name_;
    }

    std::vector<std::string_view> Dependencies() const override {
        return dependencies_;
    }

    std::vector<std::string_view> RequiredServices() const override {
        return required_services_;
    }

    std::vector<std::string_view> PublishedServices() const override {
        return published_services_;
    }

    KernelResult Start(ServiceRegistry& serviceRegistry, std::vector<std::string>& lifecycleTrace) override {
        service_registry_ = &serviceRegistry;
        lifecycleTrace.push_back(std::string("module.start.") + name_);

        for (const std::string_view publishedService : published_services_) {
            const bool registered = serviceRegistry.Register<int>(name_, publishedService, published_service_value_);
            if (!registered) {
                return KernelResult::Failure(KernelStatus::DuplicateService, DUPLICATE_SERVICE_MESSAGE);
            }
        }

        if (fail_on_start_) {
            return KernelResult::Failure(KernelStatus::StartupFailure, STARTUP_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back(std::string("module.update.") + name_);

        if (fail_on_update_) {
            return KernelResult::Failure(KernelStatus::UpdateFailure, UPDATE_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Shutdown(std::vector<std::string>& lifecycleTrace) override {
        lifecycleTrace.push_back(std::string("module.shutdown.") + name_);
        if (fail_on_shutdown_) {
            return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }

        if (verify_required_services_on_shutdown_) {
            const KernelResult serviceVerificationResult = VerifyRequiredServicesOnShutdown();
            if (!serviceVerificationResult.succeeded) {
                return serviceVerificationResult;
            }
        }

        return KernelResult::Success();
    }

private:
    // Verifies teardown can still read dependency-published services.
    KernelResult VerifyRequiredServicesOnShutdown() const {
        if (service_registry_ == nullptr) {
            return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }

        for (const std::string_view requiredService : required_services_) {
            if (service_registry_->Resolve<int>(requiredService) == nullptr) {
                return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
            }
        }

        return KernelResult::Success();
    }

    std::string name_;
    std::vector<std::string_view> dependencies_;
    std::vector<std::string_view> required_services_;
    std::vector<std::string_view> published_services_;
    bool fail_on_start_;
    bool fail_on_update_;
    bool fail_on_shutdown_;
    bool verify_required_services_on_shutdown_;
    ServiceRegistry* service_registry_;
    int published_service_value_;
    static constexpr const char* DUPLICATE_SERVICE_MESSAGE = "duplicate service";
    static constexpr const char* STARTUP_FAILURE_MESSAGE = "module startup failed";
    static constexpr const char* UPDATE_FAILURE_MESSAGE = "module update failed";
    static constexpr const char* SHUTDOWN_FAILURE_MESSAGE = "module shutdown failed";
};
