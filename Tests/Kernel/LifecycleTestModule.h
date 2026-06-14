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
        bool fail_on_start,
        bool fail_on_shutdown = false,
        bool verify_required_services_on_shutdown = false)
        : name_(std::move(name)),
          dependencies_(std::move(dependencies)),
          required_services_(),
          published_services_(),
          fail_on_start_(fail_on_start),
          fail_on_update_(false),
          fail_on_shutdown_(fail_on_shutdown),
          verify_required_services_on_shutdown_(verify_required_services_on_shutdown),
          service_registry_(nullptr),
          published_service_value_(0) {
    }

    LifecycleTestModule(
        std::string name,
        std::vector<std::string_view> dependencies,
        std::vector<std::string_view> required_services,
        std::vector<std::string_view> published_services,
        bool fail_on_start,
        bool fail_on_update,
        bool fail_on_shutdown = false,
        bool verify_required_services_on_shutdown = false)
        : name_(std::move(name)),
          dependencies_(std::move(dependencies)),
          required_services_(std::move(required_services)),
          published_services_(std::move(published_services)),
          fail_on_start_(fail_on_start),
          fail_on_update_(fail_on_update),
          fail_on_shutdown_(fail_on_shutdown),
          verify_required_services_on_shutdown_(verify_required_services_on_shutdown),
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

    KernelResult Start(ServiceRegistry& service_registry, std::vector<std::string>& lifecycle_trace) override {
        service_registry_ = &service_registry;
        lifecycle_trace.push_back(std::string("module.start.") + name_);

        for (const std::string_view published_service : published_services_) {
            const bool registered = service_registry.Register<int>(name_, published_service, published_service_value_);
            if (!registered) {
                return KernelResult::Failure(KernelStatus::DuplicateService, DUPLICATE_SERVICE_MESSAGE);
            }
        }

        if (fail_on_start_) {
            return KernelResult::Failure(KernelStatus::StartupFailure, STARTUP_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) override {
        static_cast<void>(frame_index);
        static_cast<void>(tick_time_nanoseconds);
        lifecycle_trace.push_back(std::string("module.update.") + name_);

        if (fail_on_update_) {
            return KernelResult::Failure(KernelStatus::UpdateFailure, UPDATE_FAILURE_MESSAGE);
        }

        return KernelResult::Success();
    }

    KernelResult Shutdown(std::vector<std::string>& lifecycle_trace) override {
        lifecycle_trace.push_back(std::string("module.shutdown.") + name_);
        if (fail_on_shutdown_) {
            return KernelResult::Failure(KernelStatus::ShutdownFailure, SHUTDOWN_FAILURE_MESSAGE);
        }

        if (verify_required_services_on_shutdown_) {
            const KernelResult service_verification_result = VerifyRequiredServicesOnShutdown();
            if (!service_verification_result.succeeded) {
                return service_verification_result;
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

        for (const std::string_view required_service : required_services_) {
            if (service_registry_->Resolve<int>(required_service) == nullptr) {
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
