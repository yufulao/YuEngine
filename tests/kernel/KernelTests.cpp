#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "LifecycleTestModule.h"
#include "yuengine/kernel/EngineKernel.h"
#include "yuengine/kernel/KernelStatus.h"
#include "yuengine/kernel/ServiceRegistry.h"

using EngineKernel = yuengine::kernel::EngineKernel;
using KernelStatus = yuengine::kernel::KernelStatus;
using ServiceRegistry = yuengine::kernel::ServiceRegistry;

namespace
{
constexpr const char* TEST_LIFECYCLE = "Kernel_ModuleLifecycle_DependencyOrder";
constexpr const char* TEST_STARTUP_FAILURE = "Kernel_ModuleStartupFailure_TearsDownStartedModules";
constexpr const char* TEST_SERVICE_REGISTRY = "Kernel_ServiceRegistry_ResolveAndMissingService";
constexpr const char* MODULE_A = "A";
constexpr const char* MODULE_B = "B";
constexpr const char* MODULE_C = "C";
constexpr const char* MODULE_D = "D";
constexpr const char* MODULE_FAIL = "Fail";
constexpr const char* SERVICE_A = "ServiceA";
constexpr const char* SERVICE_B = "ServiceB";
constexpr const char* MISSING_SERVICE = "MissingService";
constexpr std::uint32_t FRAME_INDEX = 0U;
constexpr std::uint64_t TICK_TIME_NANOSECONDS = 1000U;

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

int KernelModuleLifecycleDependencyOrder()
{
    EngineKernel kernel;
    LifecycleTestModule moduleA(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule moduleB(MODULE_B, std::vector<std::string_view>{MODULE_A}, false);
    LifecycleTestModule moduleC(MODULE_C, std::vector<std::string_view>{MODULE_B}, false);
    std::vector<std::string> lifecycleTrace;

    kernel.RegisterModule(moduleC);
    kernel.RegisterModule(moduleB);
    kernel.RegisterModule(moduleA);

    const auto startResult = kernel.Start(lifecycleTrace);
    if (!startResult.Succeeded)
    {
        return Fail("kernel startup failed");
    }

    const auto updateResult = kernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, lifecycleTrace);
    if (!updateResult.Succeeded)
    {
        return Fail("kernel update failed");
    }

    const auto shutdownResult = kernel.Shutdown(lifecycleTrace);
    if (!shutdownResult.Succeeded)
    {
        return Fail("kernel shutdown failed");
    }

    const std::vector<std::string> expectedTrace{
        "kernel.start",
        "module.start.A",
        "module.start.B",
        "module.start.C",
        "kernel.update",
        "module.update.A",
        "module.update.B",
        "module.update.C",
        "kernel.shutdown",
        "module.shutdown.C",
        "module.shutdown.B",
        "module.shutdown.A"};

    if (lifecycleTrace != expectedTrace)
    {
        return Fail("kernel lifecycle order did not match dependency order");
    }

    EngineKernel updateFailureKernel;
    LifecycleTestModule updateModuleA(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule updateModuleB(
        MODULE_B,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_B},
        false,
        true);
    LifecycleTestModule updateModuleC(MODULE_C, std::vector<std::string_view>{MODULE_B}, false);
    std::vector<std::string> updateFailureTrace;

    updateFailureKernel.RegisterModule(updateModuleA);
    updateFailureKernel.RegisterModule(updateModuleB);
    updateFailureKernel.RegisterModule(updateModuleC);

    const auto updateFailureStart = updateFailureKernel.Start(updateFailureTrace);
    if (!updateFailureStart.Succeeded)
    {
        return Fail("update failure setup did not start");
    }

    const auto updateFailureResult = updateFailureKernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, updateFailureTrace);
    if (updateFailureResult.Succeeded)
    {
        return Fail("kernel update failure was not surfaced");
    }

    if (updateFailureResult.Status != KernelStatus::UpdateFailure)
    {
        return Fail("kernel update failure had wrong status");
    }

    if (updateFailureKernel.Services().Resolve<int>(SERVICE_B) != nullptr)
    {
        return Fail("failed module service was not deregistered after update failure");
    }

    const std::vector<std::string> expectedUpdateFailureTrace{
        "kernel.start",
        "module.start.A",
        "module.start.B",
        "module.start.C",
        "kernel.update",
        "module.update.A",
        "module.update.B",
        "module.shutdown.C",
        "module.shutdown.B"};

    if (updateFailureTrace != expectedUpdateFailureTrace)
    {
        return Fail("update failure did not stop failed module and dependents in reverse order");
    }

    EngineKernel independentUpdateFailureKernel;
    LifecycleTestModule independentModuleA(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule independentModuleB(
        MODULE_B,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_B},
        false,
        true);
    LifecycleTestModule independentModuleC(MODULE_C, std::vector<std::string_view>(), false);
    std::vector<std::string> independentUpdateFailureTrace;

    independentUpdateFailureKernel.RegisterModule(independentModuleA);
    independentUpdateFailureKernel.RegisterModule(independentModuleB);
    independentUpdateFailureKernel.RegisterModule(independentModuleC);

    const auto independentUpdateFailureStart = independentUpdateFailureKernel.Start(independentUpdateFailureTrace);
    if (!independentUpdateFailureStart.Succeeded)
    {
        return Fail("independent update failure setup did not start");
    }

    const auto independentUpdateFailureResult = independentUpdateFailureKernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, independentUpdateFailureTrace);
    if (independentUpdateFailureResult.Succeeded)
    {
        return Fail("independent update failure was not surfaced");
    }

    const auto independentShutdownResult = independentUpdateFailureKernel.Shutdown(independentUpdateFailureTrace);
    if (!independentShutdownResult.Succeeded)
    {
        return Fail("independent update failure cleanup shutdown failed");
    }

    const std::vector<std::string> expectedIndependentUpdateFailureTrace{
        "kernel.start",
        "module.start.A",
        "module.start.B",
        "module.start.C",
        "kernel.update",
        "module.update.A",
        "module.update.B",
        "module.shutdown.B",
        "kernel.shutdown",
        "module.shutdown.C",
        "module.shutdown.A"};

    if (independentUpdateFailureTrace != expectedIndependentUpdateFailureTrace)
    {
        return Fail("update failure stopped an independent later-started module");
    }

    return 0;
}

int KernelModuleStartupFailureTearsDownStartedModules()
{
    EngineKernel kernel;
    LifecycleTestModule moduleA(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule moduleFail(
        MODULE_FAIL,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        true,
        false);
    std::vector<std::string> lifecycleTrace;

    kernel.RegisterModule(moduleA);
    kernel.RegisterModule(moduleFail);

    const auto startResult = kernel.Start(lifecycleTrace);
    if (startResult.Succeeded)
    {
        return Fail("kernel startup failure was not surfaced");
    }

    if (startResult.Status != KernelStatus::StartupFailure)
    {
        return Fail("kernel startup failure had wrong status");
    }

    const std::vector<std::string> expectedTrace{
        "kernel.start",
        "module.start.A",
        "module.start.Fail",
        "module.shutdown.Fail",
        "module.shutdown.A"};

    if (lifecycleTrace != expectedTrace)
    {
        return Fail("startup failure did not tear down started modules deterministically");
    }

    if (kernel.Services().Resolve<int>(SERVICE_A) != nullptr)
    {
        return Fail("partial startup failure did not deregister published services");
    }

    EngineKernel missingServiceKernel;
    LifecycleTestModule moduleRequiresMissing(
        MODULE_B,
        std::vector<std::string_view>(),
        std::vector<std::string_view>{MISSING_SERVICE},
        std::vector<std::string_view>(),
        false,
        false);
    std::vector<std::string> missingServiceTrace;

    missingServiceKernel.RegisterModule(moduleRequiresMissing);

    const auto missingServiceResult = missingServiceKernel.Start(missingServiceTrace);
    if (missingServiceResult.Succeeded)
    {
        return Fail("missing required service did not block startup");
    }

    if (missingServiceResult.Status != KernelStatus::MissingService)
    {
        return Fail("missing required service had wrong status");
    }

    const std::vector<std::string> expectedMissingServiceTrace{"kernel.start"};
    if (missingServiceTrace != expectedMissingServiceTrace)
    {
        return Fail("missing required service ran module startup");
    }

    EngineKernel unguaranteedServiceKernel;
    LifecycleTestModule providerModule(
        MODULE_A,
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        false,
        false);
    LifecycleTestModule serviceConsumerWithoutDependency(
        MODULE_D,
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        std::vector<std::string_view>(),
        false,
        false);
    std::vector<std::string> unguaranteedServiceTrace;

    unguaranteedServiceKernel.RegisterModule(providerModule);
    unguaranteedServiceKernel.RegisterModule(serviceConsumerWithoutDependency);

    const auto unguaranteedServiceResult = unguaranteedServiceKernel.Start(unguaranteedServiceTrace);
    if (unguaranteedServiceResult.Succeeded)
    {
        return Fail("available service without dependency edge did not block startup");
    }

    if (unguaranteedServiceResult.Status != KernelStatus::MissingService)
    {
        return Fail("unguaranteed service provider had wrong status");
    }

    const std::vector<std::string> expectedUnguaranteedServiceTrace{
        "kernel.start",
        "module.start.A",
        "module.shutdown.A"};

    if (unguaranteedServiceTrace != expectedUnguaranteedServiceTrace)
    {
        return Fail("unguaranteed service provider ran consumer startup");
    }

    return 0;
}

int KernelServiceRegistryResolveAndMissingService()
{
    ServiceRegistry serviceRegistry;
    int service = 7;
    const bool registered = serviceRegistry.Register<int>(MODULE_A, SERVICE_A, service);
    if (!registered)
    {
        return Fail("registered service was rejected");
    }

    int* resolvedService = serviceRegistry.Resolve<int>(SERVICE_A);
    if (resolvedService == nullptr)
    {
        return Fail("registered service did not resolve");
    }

    if (*resolvedService != service)
    {
        return Fail("registered service resolved to wrong instance");
    }

    double* missingService = serviceRegistry.Resolve<double>(MISSING_SERVICE);
    if (missingService != nullptr)
    {
        return Fail("missing service did not report missing");
    }

    int duplicateService = 8;
    const bool duplicateRegistered = serviceRegistry.Register<int>(MODULE_B, SERVICE_A, duplicateService);
    if (duplicateRegistered)
    {
        return Fail("duplicate service registration was not rejected");
    }

    serviceRegistry.UnregisterOwner(MODULE_A);
    if (serviceRegistry.Resolve<int>(SERVICE_A) != nullptr)
    {
        return Fail("service owner deregistration did not remove service");
    }

    const std::string lookupPolicy(ServiceRegistry::LookupPolicy);
    if (lookupPolicy != "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS")
    {
        return Fail("service lookup policy was not explicit");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_LIFECYCLE)
    {
        return KernelModuleLifecycleDependencyOrder();
    }

    if (testName == TEST_STARTUP_FAILURE)
    {
        return KernelModuleStartupFailureTearsDownStartedModules();
    }

    if (testName == TEST_SERVICE_REGISTRY)
    {
        return KernelServiceRegistryResolveAndMissingService();
    }

    return Fail("unknown test name");
}
