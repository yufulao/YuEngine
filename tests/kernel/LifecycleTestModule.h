#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "yuengine/kernel/IModule.h"

class LifecycleTestModule final : public yuengine::kernel::IModule
{
public:
    LifecycleTestModule(std::string name, std::vector<std::string_view> dependencies, bool failOnStart)
        : _name(std::move(name)),
          _dependencies(std::move(dependencies)),
          _requiredServices(),
          _publishedServices(),
          _failOnStart(failOnStart),
          _failOnUpdate(false),
          _publishedServiceValue(0)
    {
    }

    LifecycleTestModule(
        std::string name,
        std::vector<std::string_view> dependencies,
        std::vector<std::string_view> requiredServices,
        std::vector<std::string_view> publishedServices,
        bool failOnStart,
        bool failOnUpdate)
        : _name(std::move(name)),
          _dependencies(std::move(dependencies)),
          _requiredServices(std::move(requiredServices)),
          _publishedServices(std::move(publishedServices)),
          _failOnStart(failOnStart),
          _failOnUpdate(failOnUpdate),
          _publishedServiceValue(7)
    {
    }

    std::string_view Name() const override
    {
        return _name;
    }

    std::vector<std::string_view> Dependencies() const override
    {
        return _dependencies;
    }

    std::vector<std::string_view> RequiredServices() const override
    {
        return _requiredServices;
    }

    std::vector<std::string_view> PublishedServices() const override
    {
        return _publishedServices;
    }

    yuengine::kernel::KernelResult Start(yuengine::kernel::ServiceRegistry& serviceRegistry, std::vector<std::string>& lifecycleTrace) override
    {
        lifecycleTrace.push_back(std::string("module.start.") + _name);

        for (const std::string_view publishedService : _publishedServices)
        {
            const bool registered = serviceRegistry.Register<int>(_name, publishedService, _publishedServiceValue);
            if (!registered)
            {
                return yuengine::kernel::KernelResult::Failure(yuengine::kernel::KernelStatus::DuplicateService, "duplicate service");
            }
        }

        if (_failOnStart)
        {
            return yuengine::kernel::KernelResult::Failure(yuengine::kernel::KernelStatus::StartupFailure, "module startup failed");
        }

        return yuengine::kernel::KernelResult::Success();
    }

    yuengine::kernel::KernelResult Update(std::uint32_t frameIndex, std::uint64_t tickTimeNanoseconds, std::vector<std::string>& lifecycleTrace) override
    {
        static_cast<void>(frameIndex);
        static_cast<void>(tickTimeNanoseconds);
        lifecycleTrace.push_back(std::string("module.update.") + _name);

        if (_failOnUpdate)
        {
            return yuengine::kernel::KernelResult::Failure(yuengine::kernel::KernelStatus::UpdateFailure, "module update failed");
        }

        return yuengine::kernel::KernelResult::Success();
    }

    yuengine::kernel::KernelResult Shutdown(std::vector<std::string>& lifecycleTrace) override
    {
        lifecycleTrace.push_back(std::string("module.shutdown.") + _name);
        return yuengine::kernel::KernelResult::Success();
    }

private:
    std::string _name;
    std::vector<std::string_view> _dependencies;
    std::vector<std::string_view> _requiredServices;
    std::vector<std::string_view> _publishedServices;
    bool _failOnStart;
    bool _failOnUpdate;
    int _publishedServiceValue;
};
