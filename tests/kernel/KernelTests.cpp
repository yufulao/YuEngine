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
constexpr const char* TEST_INVALID_LIFECYCLE = "Kernel_InvalidLifecycle_RejectsOutOfOrderCalls";
constexpr const char* MODULE_A = "A";
constexpr const char* MODULE_B = "B";
constexpr const char* MODULE_C = "C";
constexpr const char* MODULE_D = "D";
constexpr const char* MODULE_FAIL = "Fail";
constexpr const char* SERVICE_A = "ServiceA";
constexpr const char* SERVICE_B = "ServiceB";
constexpr const char* MISSING_SERVICE = "MissingService";
constexpr const char* UPDATE_FAILURE_SHUTDOWN_MESSAGE = "update failure cleanup shutdown failed";
constexpr const char* UPDATE_BEFORE_START_MESSAGE = "update before start was not rejected";
constexpr const char* UPDATE_BEFORE_START_STATUS_MESSAGE = "update before start had wrong status";
constexpr const char* UPDATE_BEFORE_START_TRACE_MESSAGE = "update before start mutated lifecycle trace";
constexpr const char* SHUTDOWN_BEFORE_START_MESSAGE = "shutdown before start was not rejected";
constexpr const char* SHUTDOWN_BEFORE_START_STATUS_MESSAGE = "shutdown before start had wrong status";
constexpr const char* SHUTDOWN_BEFORE_START_TRACE_MESSAGE = "shutdown before start mutated lifecycle trace";
constexpr const char* DUPLICATE_START_SETUP_MESSAGE = "duplicate start setup failed";
constexpr const char* DUPLICATE_START_MESSAGE = "duplicate start was not rejected";
constexpr const char* DUPLICATE_START_STATUS_MESSAGE = "duplicate start had wrong status";
constexpr const char* DUPLICATE_START_TRACE_MESSAGE = "duplicate start mutated lifecycle trace";
constexpr const char* DUPLICATE_START_SHUTDOWN_MESSAGE = "duplicate start cleanup shutdown failed";
constexpr const char* WRONG_SERVICE_TYPE_MESSAGE = "registered service resolved through wrong C++ type";
constexpr const char* TRACE_KERNEL_START = "kernel.start";
constexpr const char* TRACE_KERNEL_SHUTDOWN = "kernel.shutdown";
constexpr const char* TRACE_MODULE_START_A = "module.start.A";
constexpr const char* TRACE_MODULE_SHUTDOWN_A = "module.shutdown.A";
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

    const auto updateFailureShutdownResult = updateFailureKernel.Shutdown(updateFailureTrace);
    if (!updateFailureShutdownResult.Succeeded)
    {
        return Fail(UPDATE_FAILURE_SHUTDOWN_MESSAGE);
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
        "module.shutdown.B",
        TRACE_KERNEL_SHUTDOWN,
        TRACE_MODULE_SHUTDOWN_A};

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

    double* wrongTypeService = serviceRegistry.Resolve<double>(SERVICE_A);
    if (wrongTypeService != nullptr)
    {
        return Fail(WRONG_SERVICE_TYPE_MESSAGE);
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

int KernelInvalidLifecycleRejectsOutOfOrderCalls()
{
    EngineKernel updateBeforeStartKernel;
    std::vector<std::string> updateBeforeStartTrace;

    const auto updateBeforeStartResult =
        updateBeforeStartKernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, updateBeforeStartTrace);
    if (updateBeforeStartResult.Succeeded)
    {
        return Fail(UPDATE_BEFORE_START_MESSAGE);
    }

    if (updateBeforeStartResult.Status != KernelStatus::InvalidLifecycle)
    {
        return Fail(UPDATE_BEFORE_START_STATUS_MESSAGE);
    }

    if (!updateBeforeStartTrace.empty())
    {
        return Fail(UPDATE_BEFORE_START_TRACE_MESSAGE);
    }

    EngineKernel shutdownBeforeStartKernel;
    std::vector<std::string> shutdownBeforeStartTrace;

    const auto shutdownBeforeStartResult = shutdownBeforeStartKernel.Shutdown(shutdownBeforeStartTrace);
    if (shutdownBeforeStartResult.Succeeded)
    {
        return Fail(SHUTDOWN_BEFORE_START_MESSAGE);
    }

    if (shutdownBeforeStartResult.Status != KernelStatus::InvalidLifecycle)
    {
        return Fail(SHUTDOWN_BEFORE_START_STATUS_MESSAGE);
    }

    if (!shutdownBeforeStartTrace.empty())
    {
        return Fail(SHUTDOWN_BEFORE_START_TRACE_MESSAGE);
    }

    EngineKernel duplicateStartKernel;
    LifecycleTestModule moduleA(MODULE_A, std::vector<std::string_view>(), false);
    std::vector<std::string> duplicateStartTrace;

    duplicateStartKernel.RegisterModule(moduleA);

    const auto firstStartResult = duplicateStartKernel.Start(duplicateStartTrace);
    if (!firstStartResult.Succeeded)
    {
        return Fail(DUPLICATE_START_SETUP_MESSAGE);
    }

    const auto duplicateStartResult = duplicateStartKernel.Start(duplicateStartTrace);
    if (duplicateStartResult.Succeeded)
    {
        return Fail(DUPLICATE_START_MESSAGE);
    }

    if (duplicateStartResult.Status != KernelStatus::InvalidLifecycle)
    {
        return Fail(DUPLICATE_START_STATUS_MESSAGE);
    }

    const std::vector<std::string> expectedDuplicateStartTrace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_A};

    if (duplicateStartTrace != expectedDuplicateStartTrace)
    {
        return Fail(DUPLICATE_START_TRACE_MESSAGE);
    }

    const auto shutdownResult = duplicateStartKernel.Shutdown(duplicateStartTrace);
    if (!shutdownResult.Succeeded)
    {
        return Fail(DUPLICATE_START_SHUTDOWN_MESSAGE);
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

    if (testName == TEST_INVALID_LIFECYCLE)
    {
        return KernelInvalidLifecycleRejectsOutOfOrderCalls();
    }

    return Fail("unknown test name");
}
