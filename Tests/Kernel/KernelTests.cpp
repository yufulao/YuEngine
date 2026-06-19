// 模块：Tests Kernel
// 文件：Tests/Kernel/KernelTests.cpp

#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "LifecycleTestModule.h"
#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Kernel/RuntimeApp.h"
#include "YuEngine/Kernel/RuntimeAppStatus.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"
#include "YuEngine/Kernel/ServiceRegistry.h"

using EngineKernel = yuengine::kernel::EngineKernel;
using yuengine::kernel::KernelStatus;
using RuntimeApp = yuengine::kernel::RuntimeApp;
using yuengine::kernel::RuntimeAppDesc;
using yuengine::kernel::RuntimeAppStatus;
using yuengine::kernel::RuntimeFrameInputSnapshotRef;
using yuengine::kernel::RuntimeFrameMode;
using yuengine::kernel::RuntimeFramePhase;
using ServiceRegistry = yuengine::kernel::ServiceRegistry;

namespace {
constexpr const char* TEST_LIFECYCLE = "Kernel_ModuleLifecycle_DependencyOrder";
constexpr const char* TEST_STARTUP_FAILURE = "Kernel_ModuleStartupFailure_TearsDownStartedModules";
constexpr const char* TEST_DEPENDENCY_CHAIN_SERVICE_LOOKUP = "Kernel_DependencyChainServiceLookup_UsesModuleNameIndex";
constexpr const char* TEST_SERVICE_REGISTRY = "Kernel_ServiceRegistry_ResolveAndMissingService";
constexpr const char* TEST_INVALID_LIFECYCLE = "Kernel_InvalidLifecycle_RejectsOutOfOrderCalls";
constexpr const char* TEST_RUNTIME_APP_ZERO_WORLD = "Kernel_RuntimeAppZeroWorld_FixedFrameLoop";
constexpr const char* TEST_RUNTIME_APP_FRAME_CONTEXT = "Kernel_RuntimeAppFrameContext_ExposesValueContract";
constexpr const char* TEST_RUNTIME_APP_FAILURE_PROPAGATION = "Kernel_RuntimeAppUpdateFailure_PropagatesAndShutsDown";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
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
constexpr const char* LATE_REGISTRATION_MESSAGE = "module registration succeeded after kernel start";
constexpr const char* PRESTART_SERVICE_REGISTRATION_MESSAGE = "service registration succeeded before kernel start";
constexpr const char* PRESTART_SERVICE_RESOLVE_MESSAGE = "prestart-registered service resolved after rejection";
constexpr const char* RUNTIME_SERVICE_REGISTRATION_MESSAGE = "service registration succeeded after kernel start";
constexpr const char* RUNTIME_SERVICE_RESOLVE_MESSAGE = "runtime-registered service resolved after rejection";
constexpr const char* STARTUP_WITHOUT_SERVICE_STATUS_MESSAGE = "startup failure without services had wrong status";
constexpr const char* STARTUP_WITHOUT_SERVICE_TRACE_MESSAGE = "startup failure without services did not roll back failed module";
constexpr const char* FAILED_START_SERVICE_REGISTRATION_MESSAGE = "service registration succeeded after failed kernel start";
constexpr const char* FAILED_START_SERVICE_RESOLVE_MESSAGE = "failed-start service resolved after rejection";
constexpr const char* STARTUP_CLEANUP_FAILURE_STATUS_MESSAGE = "startup cleanup failure had wrong status";
constexpr const char* STARTUP_CLEANUP_FAILURE_TRACE_MESSAGE = "startup cleanup failure did not continue started-module rollback";
constexpr const char* STARTUP_CLEANUP_FAILURE_PROVIDER_MESSAGE = "startup cleanup failure left provider service registered";
constexpr const char* STARTUP_CLEANUP_FAILURE_FAILED_MESSAGE = "startup cleanup failure left failed service registered";
constexpr const char* SELF_REQUIRED_SERVICE_MESSAGE = "self-published required service did not block startup";
constexpr const char* SELF_REQUIRED_SERVICE_STATUS_MESSAGE = "self-published required service had wrong status";
constexpr const char* SELF_REQUIRED_SERVICE_TRACE_MESSAGE = "self-published required service ran module startup";
constexpr const char* RUNTIME_APP_INIT_MESSAGE = "runtime app initialization failed";
constexpr const char* RUNTIME_APP_RUN_MESSAGE = "runtime app run failed";
constexpr const char* RUNTIME_APP_TRACE_MESSAGE = "runtime app lifecycle trace was not deterministic";
constexpr const char* RUNTIME_APP_PHASE_MESSAGE = "runtime app phase trace was not deterministic";
constexpr const char* RUNTIME_APP_SNAPSHOT_MESSAGE = "runtime app snapshot was not updated";
constexpr const char* RUNTIME_APP_CONTEXT_MESSAGE = "runtime app context value contract was not preserved";
constexpr const char* RUNTIME_APP_FAILURE_STATUS_MESSAGE = "runtime app failure had wrong status";
constexpr const char* RUNTIME_APP_KERNEL_STATUS_MESSAGE = "runtime app did not preserve kernel update status";
constexpr const char* RUNTIME_APP_FAILURE_SHUTDOWN_MESSAGE = "runtime app did not shutdown after update failure";
constexpr const char* DEPENDENCY_CHAIN_LOOKUP_START_MESSAGE = "indexed dependency-chain service lookup failed";
constexpr const char* DEPENDENCY_CHAIN_LOOKUP_SHUTDOWN_MESSAGE = "indexed dependency-chain shutdown failed";
constexpr const char* DEPENDENCY_CHAIN_LOOKUP_TRACE_MESSAGE = "indexed dependency-chain lifecycle was not deterministic";
constexpr const char* WRONG_SERVICE_TYPE_MESSAGE = "registered service resolved through wrong C++ type";
constexpr const char* TRACE_KERNEL_START = "kernel.start";
constexpr const char* TRACE_KERNEL_SHUTDOWN = "kernel.shutdown";
constexpr const char* TRACE_MODULE_START_A = "module.start.A";
constexpr const char* TRACE_MODULE_START_B = "module.start.B";
constexpr const char* TRACE_MODULE_START_C = "module.start.C";
constexpr const char* TRACE_MODULE_START_FAIL = "module.start.Fail";
constexpr const char* TRACE_MODULE_SHUTDOWN_A = "module.shutdown.A";
constexpr const char* TRACE_MODULE_SHUTDOWN_B = "module.shutdown.B";
constexpr const char* TRACE_MODULE_SHUTDOWN_C = "module.shutdown.C";
constexpr const char* TRACE_MODULE_SHUTDOWN_FAIL = "module.shutdown.Fail";
constexpr std::uint32_t FRAME_INDEX = 0U;
constexpr std::uint64_t TICK_TIME_NANOSECONDS = 1000U;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

void AppendExpectedRuntimeFramePhases(std::vector<RuntimeFramePhase>* phase_trace) {
    phase_trace->push_back(RuntimeFramePhase::BeginFrame);
    phase_trace->push_back(RuntimeFramePhase::PollPlatform);
    phase_trace->push_back(RuntimeFramePhase::PollInput);
    phase_trace->push_back(RuntimeFramePhase::LoadOrCommitResources);
    phase_trace->push_back(RuntimeFramePhase::UpdateWorld);
    phase_trace->push_back(RuntimeFramePhase::PrepareRender);
    phase_trace->push_back(RuntimeFramePhase::SubmitAudio);
    phase_trace->push_back(RuntimeFramePhase::SubmitRender);
    phase_trace->push_back(RuntimeFramePhase::Present);
    phase_trace->push_back(RuntimeFramePhase::EndFrame);
}

void AppendExpectedRuntimeFrameFailurePhases(std::vector<RuntimeFramePhase>* phase_trace) {
    phase_trace->push_back(RuntimeFramePhase::BeginFrame);
    phase_trace->push_back(RuntimeFramePhase::PollPlatform);
    phase_trace->push_back(RuntimeFramePhase::PollInput);
    phase_trace->push_back(RuntimeFramePhase::LoadOrCommitResources);
    phase_trace->push_back(RuntimeFramePhase::UpdateWorld);
}

int KernelModuleLifecycleDependencyOrder() {
    EngineKernel kernel;
    LifecycleTestModule module_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule module_b(MODULE_B, std::vector<std::string_view>{MODULE_A}, false);
    LifecycleTestModule module_c(MODULE_C, std::vector<std::string_view>{MODULE_B}, false);
    std::vector<std::string> lifecycle_trace;

    kernel.RegisterModule(module_c);
    kernel.RegisterModule(module_b);
    kernel.RegisterModule(module_a);

    const auto start_result = kernel.Start(lifecycle_trace);
    if (!start_result.succeeded) {
        return Fail("kernel startup failed");
    }

    const auto update_result = kernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, lifecycle_trace);
    if (!update_result.succeeded) {
        return Fail("kernel update failed");
    }

    const auto shutdown_result = kernel.Shutdown(lifecycle_trace);
    if (!shutdown_result.succeeded) {
        return Fail("kernel shutdown failed");
    }

    const std::vector<std::string> expected_trace{
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

    if (lifecycle_trace != expected_trace) {
        return Fail("kernel lifecycle order did not match dependency order");
    }

    EngineKernel update_failure_kernel;
    LifecycleTestModule update_module_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule update_module_b(
        MODULE_B,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_B},
        false,
        true);
    LifecycleTestModule update_module_c(
        MODULE_C,
        std::vector<std::string_view>{MODULE_B},
        std::vector<std::string_view>{SERVICE_B},
        std::vector<std::string_view>(),
        false,
        false,
        false,
        true);
    std::vector<std::string> update_failure_trace;

    update_failure_kernel.RegisterModule(update_module_a);
    update_failure_kernel.RegisterModule(update_module_b);
    update_failure_kernel.RegisterModule(update_module_c);

    const auto update_failure_start = update_failure_kernel.Start(update_failure_trace);
    if (!update_failure_start.succeeded) {
        return Fail("update failure setup did not start");
    }

    const auto update_failure_result = update_failure_kernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, update_failure_trace);
    if (update_failure_result.succeeded) {
        return Fail("kernel update failure was not surfaced");
    }

    if (update_failure_result.status != KernelStatus::UpdateFailure) {
        return Fail("kernel update failure had wrong status");
    }

    if (update_failure_kernel.Services().Resolve<int>(SERVICE_B) != nullptr) {
        return Fail("failed module service was not deregistered after update failure");
    }

    const auto update_failure_shutdown_result = update_failure_kernel.Shutdown(update_failure_trace);
    if (!update_failure_shutdown_result.succeeded) {
        return Fail(UPDATE_FAILURE_SHUTDOWN_MESSAGE);
    }

    const std::vector<std::string> expected_update_failure_trace{
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

    if (update_failure_trace != expected_update_failure_trace) {
        return Fail("update failure did not stop failed module and dependents in reverse order");
    }

    EngineKernel independent_update_failure_kernel;
    LifecycleTestModule independent_module_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule independent_module_b(
        MODULE_B,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_B},
        false,
        true);
    LifecycleTestModule independent_module_c(MODULE_C, std::vector<std::string_view>(), false);
    std::vector<std::string> independent_update_failure_trace;

    independent_update_failure_kernel.RegisterModule(independent_module_a);
    independent_update_failure_kernel.RegisterModule(independent_module_b);
    independent_update_failure_kernel.RegisterModule(independent_module_c);

    const auto independent_update_failure_start = independent_update_failure_kernel.Start(independent_update_failure_trace);
    if (!independent_update_failure_start.succeeded) {
        return Fail("independent update failure setup did not start");
    }

    const auto independent_update_failure_result = independent_update_failure_kernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, independent_update_failure_trace);
    if (independent_update_failure_result.succeeded) {
        return Fail("independent update failure was not surfaced");
    }

    const auto independent_shutdown_result = independent_update_failure_kernel.Shutdown(independent_update_failure_trace);
    if (!independent_shutdown_result.succeeded) {
        return Fail("independent update failure cleanup shutdown failed");
    }

    const std::vector<std::string> expected_independent_update_failure_trace{
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

    if (independent_update_failure_trace != expected_independent_update_failure_trace) {
        return Fail("update failure stopped an independent later-started module");
    }

    return 0;
}

int KernelDependencyChainServiceLookupUsesModuleNameIndex() {
    EngineKernel kernel;
    LifecycleTestModule provider(
        MODULE_A,
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        false,
        false);
    LifecycleTestModule relay(MODULE_B, std::vector<std::string_view>{MODULE_A}, false);
    LifecycleTestModule consumer(
        MODULE_C,
        std::vector<std::string_view>{MODULE_B},
        std::vector<std::string_view>{SERVICE_A},
        std::vector<std::string_view>(),
        false,
        false,
        false,
        true);
    std::vector<std::string> lifecycle_trace;

    kernel.RegisterModule(consumer);
    kernel.RegisterModule(relay);
    kernel.RegisterModule(provider);

    const auto start_result = kernel.Start(lifecycle_trace);
    if (!start_result.succeeded) {
        return Fail(DEPENDENCY_CHAIN_LOOKUP_START_MESSAGE);
    }

    const auto shutdown_result = kernel.Shutdown(lifecycle_trace);
    if (!shutdown_result.succeeded) {
        return Fail(DEPENDENCY_CHAIN_LOOKUP_SHUTDOWN_MESSAGE);
    }

    const std::vector<std::string> expected_trace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_A,
        TRACE_MODULE_START_B,
        TRACE_MODULE_START_C,
        TRACE_KERNEL_SHUTDOWN,
        TRACE_MODULE_SHUTDOWN_C,
        TRACE_MODULE_SHUTDOWN_B,
        TRACE_MODULE_SHUTDOWN_A};

    if (lifecycle_trace != expected_trace) {
        return Fail(DEPENDENCY_CHAIN_LOOKUP_TRACE_MESSAGE);
    }

    return 0;
}

int KernelModuleStartupFailureTearsDownStartedModules() {
    EngineKernel kernel;
    LifecycleTestModule module_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule module_fail(
        MODULE_FAIL,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        true,
        false);
    std::vector<std::string> lifecycle_trace;

    kernel.RegisterModule(module_a);
    kernel.RegisterModule(module_fail);

    const auto start_result = kernel.Start(lifecycle_trace);
    if (start_result.succeeded) {
        return Fail("kernel startup failure was not surfaced");
    }

    if (start_result.status != KernelStatus::StartupFailure) {
        return Fail("kernel startup failure had wrong status");
    }

    const std::vector<std::string> expected_trace{
        "kernel.start",
        "module.start.A",
        "module.start.Fail",
        "module.shutdown.Fail",
        "module.shutdown.A"};

    if (lifecycle_trace != expected_trace) {
        return Fail("startup failure did not tear down started modules deterministically");
    }

    if (kernel.Services().Resolve<int>(SERVICE_A) != nullptr) {
        return Fail("partial startup failure did not deregister published services");
    }

    EngineKernel failure_without_service_kernel;
    LifecycleTestModule module_without_service_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule module_fail_without_service(MODULE_FAIL, std::vector<std::string_view>{MODULE_A}, true);
    std::vector<std::string> failure_without_service_trace;

    failure_without_service_kernel.RegisterModule(module_without_service_a);
    failure_without_service_kernel.RegisterModule(module_fail_without_service);

    const auto failure_without_service_result = failure_without_service_kernel.Start(failure_without_service_trace);
    if (failure_without_service_result.status != KernelStatus::StartupFailure) {
        return Fail(STARTUP_WITHOUT_SERVICE_STATUS_MESSAGE);
    }

    const std::vector<std::string> expected_failure_without_service_trace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_A,
        TRACE_MODULE_START_FAIL,
        TRACE_MODULE_SHUTDOWN_FAIL,
        TRACE_MODULE_SHUTDOWN_A};

    if (failure_without_service_trace != expected_failure_without_service_trace) {
        return Fail(STARTUP_WITHOUT_SERVICE_TRACE_MESSAGE);
    }

    int failed_startup_service = 10;
    const bool failed_startup_service_registered =
        failure_without_service_kernel.Services().Register<int>(MODULE_B, SERVICE_B, failed_startup_service);
    if (failed_startup_service_registered) {
        return Fail(FAILED_START_SERVICE_REGISTRATION_MESSAGE);
    }

    if (failure_without_service_kernel.Services().Resolve<int>(SERVICE_B) != nullptr) {
        return Fail(FAILED_START_SERVICE_RESOLVE_MESSAGE);
    }

    EngineKernel cleanup_failure_kernel;
    LifecycleTestModule cleanup_provider(
        MODULE_A,
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        false,
        false);
    LifecycleTestModule cleanup_failing_module(
        MODULE_FAIL,
        std::vector<std::string_view>{MODULE_A},
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_B},
        true,
        false,
        true);
    std::vector<std::string> cleanup_failure_trace;

    cleanup_failure_kernel.RegisterModule(cleanup_provider);
    cleanup_failure_kernel.RegisterModule(cleanup_failing_module);

    const auto cleanup_failure_result = cleanup_failure_kernel.Start(cleanup_failure_trace);
    if (cleanup_failure_result.status != KernelStatus::ShutdownFailure) {
        return Fail(STARTUP_CLEANUP_FAILURE_STATUS_MESSAGE);
    }

    const std::vector<std::string> expected_cleanup_failure_trace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_A,
        TRACE_MODULE_START_FAIL,
        TRACE_MODULE_SHUTDOWN_FAIL,
        TRACE_MODULE_SHUTDOWN_A};

    if (cleanup_failure_trace != expected_cleanup_failure_trace) {
        return Fail(STARTUP_CLEANUP_FAILURE_TRACE_MESSAGE);
    }

    if (cleanup_failure_kernel.Services().Resolve<int>(SERVICE_A) != nullptr) {
        return Fail(STARTUP_CLEANUP_FAILURE_PROVIDER_MESSAGE);
    }

    if (cleanup_failure_kernel.Services().Resolve<int>(SERVICE_B) != nullptr) {
        return Fail(STARTUP_CLEANUP_FAILURE_FAILED_MESSAGE);
    }

    EngineKernel missing_service_kernel;
    LifecycleTestModule module_requires_missing(
        MODULE_B,
        std::vector<std::string_view>(),
        std::vector<std::string_view>{MISSING_SERVICE},
        std::vector<std::string_view>(),
        false,
        false);
    std::vector<std::string> missing_service_trace;

    missing_service_kernel.RegisterModule(module_requires_missing);

    const auto missing_service_result = missing_service_kernel.Start(missing_service_trace);
    if (missing_service_result.succeeded) {
        return Fail("missing required service did not block startup");
    }

    if (missing_service_result.status != KernelStatus::MissingService) {
        return Fail("missing required service had wrong status");
    }

    const std::vector<std::string> expected_missing_service_trace{"kernel.start"};
    if (missing_service_trace != expected_missing_service_trace) {
        return Fail("missing required service ran module startup");
    }

    EngineKernel self_required_service_kernel;
    LifecycleTestModule self_required_publisher(
        MODULE_A,
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        std::vector<std::string_view>{SERVICE_A},
        false,
        false);
    std::vector<std::string> self_required_service_trace;

    self_required_service_kernel.RegisterModule(self_required_publisher);

    const auto self_required_service_result = self_required_service_kernel.Start(self_required_service_trace);
    if (self_required_service_result.succeeded) {
        return Fail(SELF_REQUIRED_SERVICE_MESSAGE);
    }

    if (self_required_service_result.status != KernelStatus::MissingService) {
        return Fail(SELF_REQUIRED_SERVICE_STATUS_MESSAGE);
    }

    const std::vector<std::string> expected_self_required_service_trace{"kernel.start"};
    if (self_required_service_trace != expected_self_required_service_trace) {
        return Fail(SELF_REQUIRED_SERVICE_TRACE_MESSAGE);
    }

    EngineKernel unguaranteed_service_kernel;
    LifecycleTestModule provider_module(
        MODULE_A,
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        false,
        false);
    LifecycleTestModule service_consumer_without_dependency(
        MODULE_D,
        std::vector<std::string_view>(),
        std::vector<std::string_view>{SERVICE_A},
        std::vector<std::string_view>(),
        false,
        false);
    std::vector<std::string> unguaranteed_service_trace;

    unguaranteed_service_kernel.RegisterModule(provider_module);
    unguaranteed_service_kernel.RegisterModule(service_consumer_without_dependency);

    const auto unguaranteed_service_result = unguaranteed_service_kernel.Start(unguaranteed_service_trace);
    if (unguaranteed_service_result.succeeded) {
        return Fail("available service without dependency edge did not block startup");
    }

    if (unguaranteed_service_result.status != KernelStatus::MissingService) {
        return Fail("unguaranteed service provider had wrong status");
    }

    const std::vector<std::string> expected_unguaranteed_service_trace{
        "kernel.start",
        "module.start.A",
        "module.shutdown.A"};

    if (unguaranteed_service_trace != expected_unguaranteed_service_trace) {
        return Fail("unguaranteed service provider ran consumer startup");
    }

    return 0;
}

int KernelServiceRegistryResolveAndMissingService() {
    ServiceRegistry service_registry;
    int service = 7;
    const bool registered = service_registry.Register<int>(MODULE_A, SERVICE_A, service);
    if (!registered) {
        return Fail("registered service was rejected");
    }

    int* resolved_service = service_registry.Resolve<int>(SERVICE_A);
    if (resolved_service == nullptr) {
        return Fail("registered service did not resolve");
    }

    if (*resolved_service != service) {
        return Fail("registered service resolved to wrong instance");
    }

    double* wrong_type_service = service_registry.Resolve<double>(SERVICE_A);
    if (wrong_type_service != nullptr) {
        return Fail(WRONG_SERVICE_TYPE_MESSAGE);
    }

    double* missing_service = service_registry.Resolve<double>(MISSING_SERVICE);
    if (missing_service != nullptr) {
        return Fail("missing service did not report missing");
    }

    int duplicate_service = 8;
    const bool duplicate_registered = service_registry.Register<int>(MODULE_B, SERVICE_A, duplicate_service);
    if (duplicate_registered) {
        return Fail("duplicate service registration was not rejected");
    }

    service_registry.UnregisterOwner(MODULE_A);
    if (service_registry.Resolve<int>(SERVICE_A) != nullptr) {
        return Fail("service owner deregistration did not remove service");
    }

    const std::string lookup_policy(ServiceRegistry::LOOKUP_POLICY);
    if (lookup_policy != "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS") {
        return Fail("service lookup policy was not explicit");
    }

    return 0;
}

int KernelInvalidLifecycleRejectsOutOfOrderCalls() {
    EngineKernel update_before_start_kernel;
    std::vector<std::string> update_before_start_trace;

    const auto update_before_start_result =
        update_before_start_kernel.Update(FRAME_INDEX, TICK_TIME_NANOSECONDS, update_before_start_trace);
    if (update_before_start_result.succeeded) {
        return Fail(UPDATE_BEFORE_START_MESSAGE);
    }

    if (update_before_start_result.status != KernelStatus::InvalidLifecycle) {
        return Fail(UPDATE_BEFORE_START_STATUS_MESSAGE);
    }

    if (!update_before_start_trace.empty()) {
        return Fail(UPDATE_BEFORE_START_TRACE_MESSAGE);
    }

    EngineKernel shutdown_before_start_kernel;
    std::vector<std::string> shutdown_before_start_trace;

    const auto shutdown_before_start_result = shutdown_before_start_kernel.Shutdown(shutdown_before_start_trace);
    if (shutdown_before_start_result.succeeded) {
        return Fail(SHUTDOWN_BEFORE_START_MESSAGE);
    }

    if (shutdown_before_start_result.status != KernelStatus::InvalidLifecycle) {
        return Fail(SHUTDOWN_BEFORE_START_STATUS_MESSAGE);
    }

    if (!shutdown_before_start_trace.empty()) {
        return Fail(SHUTDOWN_BEFORE_START_TRACE_MESSAGE);
    }

    EngineKernel pre_start_service_kernel;
    int pre_start_service = 8;
    const bool pre_start_service_registered =
        pre_start_service_kernel.Services().Register<int>(MODULE_A, SERVICE_A, pre_start_service);
    if (pre_start_service_registered) {
        return Fail(PRESTART_SERVICE_REGISTRATION_MESSAGE);
    }

    if (pre_start_service_kernel.Services().Resolve<int>(SERVICE_A) != nullptr) {
        return Fail(PRESTART_SERVICE_RESOLVE_MESSAGE);
    }

    EngineKernel duplicate_start_kernel;
    LifecycleTestModule module_a(MODULE_A, std::vector<std::string_view>(), false);
    LifecycleTestModule module_b(MODULE_B, std::vector<std::string_view>(), false);
    std::vector<std::string> duplicate_start_trace;

    duplicate_start_kernel.RegisterModule(module_a);

    const auto first_start_result = duplicate_start_kernel.Start(duplicate_start_trace);
    if (!first_start_result.succeeded) {
        return Fail(DUPLICATE_START_SETUP_MESSAGE);
    }

    const bool late_module_registered = duplicate_start_kernel.RegisterModule(module_b);
    if (late_module_registered) {
        return Fail(LATE_REGISTRATION_MESSAGE);
    }

    int runtime_service = 9;
    const bool runtime_service_registered = duplicate_start_kernel.Services().Register<int>(MODULE_B, SERVICE_B, runtime_service);
    if (runtime_service_registered) {
        return Fail(RUNTIME_SERVICE_REGISTRATION_MESSAGE);
    }

    if (duplicate_start_kernel.Services().Resolve<int>(SERVICE_B) != nullptr) {
        return Fail(RUNTIME_SERVICE_RESOLVE_MESSAGE);
    }

    const auto duplicate_start_result = duplicate_start_kernel.Start(duplicate_start_trace);
    if (duplicate_start_result.succeeded) {
        return Fail(DUPLICATE_START_MESSAGE);
    }

    if (duplicate_start_result.status != KernelStatus::InvalidLifecycle) {
        return Fail(DUPLICATE_START_STATUS_MESSAGE);
    }

    const std::vector<std::string> expected_duplicate_start_trace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_A};

    if (duplicate_start_trace != expected_duplicate_start_trace) {
        return Fail(DUPLICATE_START_TRACE_MESSAGE);
    }

    const auto shutdown_result = duplicate_start_kernel.Shutdown(duplicate_start_trace);
    if (!shutdown_result.succeeded) {
        return Fail(DUPLICATE_START_SHUTDOWN_MESSAGE);
    }

    return 0;
}

int KernelRuntimeAppZeroWorldFixedFrameLoop() {
    EngineKernel kernel;
    RuntimeApp runtime_app;
    RuntimeAppDesc desc;
    desc.frame_count = 3U;
    desc.fixed_delta_time_nanoseconds = TICK_TIME_NANOSECONDS;
    std::vector<std::string> lifecycle_trace;
    std::vector<RuntimeFramePhase> phase_trace;

    const bool initialized = runtime_app.Initialize(&kernel, desc);
    if (!initialized) {
        return Fail(RUNTIME_APP_INIT_MESSAGE);
    }

    const auto run_result = runtime_app.RunFixedFrames(&lifecycle_trace, &phase_trace);
    if (!run_result.succeeded) {
        return Fail(RUNTIME_APP_RUN_MESSAGE);
    }

    const std::vector<std::string> expected_lifecycle_trace{
        TRACE_KERNEL_START,
        "kernel.update",
        "kernel.update",
        "kernel.update",
        TRACE_KERNEL_SHUTDOWN};

    if (lifecycle_trace != expected_lifecycle_trace) {
        return Fail(RUNTIME_APP_TRACE_MESSAGE);
    }

    std::vector<RuntimeFramePhase> expected_phase_trace;
    AppendExpectedRuntimeFramePhases(&expected_phase_trace);
    AppendExpectedRuntimeFramePhases(&expected_phase_trace);
    AppendExpectedRuntimeFramePhases(&expected_phase_trace);
    if (phase_trace != expected_phase_trace) {
        return Fail(RUNTIME_APP_PHASE_MESSAGE);
    }

    const auto snapshot = runtime_app.Snapshot();
    if (snapshot.status != RuntimeAppStatus::Success || snapshot.completed_frame_count != desc.frame_count || snapshot.running) {
        return Fail(RUNTIME_APP_SNAPSHOT_MESSAGE);
    }

    if (run_result.last_frame_context.frame_index != 2U || run_result.last_frame_context.phase != RuntimeFramePhase::EndFrame) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    return 0;
}

int KernelRuntimeAppFrameContextExposesValueContract() {
    EngineKernel kernel;
    RuntimeApp runtime_app;
    RuntimeAppDesc desc;
    int input_snapshot = 13;
    RuntimeFrameInputSnapshotRef input_snapshot_ref;
    input_snapshot_ref.snapshot = &input_snapshot;
    input_snapshot_ref.snapshot_version = 17U;
    desc.frame_count = 1U;
    desc.fixed_delta_time_nanoseconds = 33333333U;
    desc.frame_mode = RuntimeFrameMode::Fixed;
    desc.input_snapshot = input_snapshot_ref;
    std::vector<std::string> lifecycle_trace;
    std::vector<RuntimeFramePhase> phase_trace;

    const bool initialized = runtime_app.Initialize(&kernel, desc);
    if (!initialized) {
        return Fail(RUNTIME_APP_INIT_MESSAGE);
    }

    const auto run_result = runtime_app.RunFixedFrames(&lifecycle_trace, &phase_trace);
    if (!run_result.succeeded) {
        return Fail(RUNTIME_APP_RUN_MESSAGE);
    }

    const auto context = runtime_app.FrameContext();
    if (context.frame_index != 0U) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.delta_time_nanoseconds != desc.fixed_delta_time_nanoseconds) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.fixed_time_nanoseconds != desc.fixed_delta_time_nanoseconds) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.frame_mode != RuntimeFrameMode::Fixed) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.input_snapshot.snapshot != &input_snapshot || context.input_snapshot.snapshot_version != 17U) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.diagnostics_sink != nullptr) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    if (context.phase != RuntimeFramePhase::EndFrame) {
        return Fail(RUNTIME_APP_CONTEXT_MESSAGE);
    }

    return 0;
}

int KernelRuntimeAppUpdateFailurePropagatesAndShutsDown() {
    EngineKernel kernel;
    RuntimeApp runtime_app;
    RuntimeAppDesc desc;
    LifecycleTestModule failing_module(
        MODULE_FAIL,
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        std::vector<std::string_view>(),
        false,
        true);
    std::vector<std::string> lifecycle_trace;
    std::vector<RuntimeFramePhase> phase_trace;

    kernel.RegisterModule(failing_module);
    desc.frame_count = 2U;
    desc.fixed_delta_time_nanoseconds = TICK_TIME_NANOSECONDS;

    const bool initialized = runtime_app.Initialize(&kernel, desc);
    if (!initialized) {
        return Fail(RUNTIME_APP_INIT_MESSAGE);
    }

    const auto run_result = runtime_app.RunFixedFrames(&lifecycle_trace, &phase_trace);
    if (run_result.succeeded) {
        return Fail(RUNTIME_APP_FAILURE_STATUS_MESSAGE);
    }

    if (run_result.status != RuntimeAppStatus::KernelUpdateFailure) {
        return Fail(RUNTIME_APP_FAILURE_STATUS_MESSAGE);
    }

    if (run_result.kernel_status != KernelStatus::UpdateFailure) {
        return Fail(RUNTIME_APP_KERNEL_STATUS_MESSAGE);
    }

    if (run_result.shutdown_kernel_status != KernelStatus::Success) {
        return Fail(RUNTIME_APP_FAILURE_SHUTDOWN_MESSAGE);
    }

    if (run_result.completed_frame_count != 0U) {
        return Fail(RUNTIME_APP_FAILURE_STATUS_MESSAGE);
    }

    const std::vector<std::string> expected_lifecycle_trace{
        TRACE_KERNEL_START,
        TRACE_MODULE_START_FAIL,
        "kernel.update",
        "module.update.Fail",
        TRACE_MODULE_SHUTDOWN_FAIL,
        TRACE_KERNEL_SHUTDOWN};

    if (lifecycle_trace != expected_lifecycle_trace) {
        return Fail(RUNTIME_APP_TRACE_MESSAGE);
    }

    std::vector<RuntimeFramePhase> expected_phase_trace;
    AppendExpectedRuntimeFrameFailurePhases(&expected_phase_trace);
    if (phase_trace != expected_phase_trace) {
        return Fail(RUNTIME_APP_PHASE_MESSAGE);
    }

    const auto snapshot = runtime_app.Snapshot();
    if (snapshot.status != RuntimeAppStatus::KernelUpdateFailure || snapshot.running) {
        return Fail(RUNTIME_APP_SNAPSHOT_MESSAGE);
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_LIFECYCLE, KernelModuleLifecycleDependencyOrder},
        {TEST_STARTUP_FAILURE, KernelModuleStartupFailureTearsDownStartedModules},
        {TEST_DEPENDENCY_CHAIN_SERVICE_LOOKUP, KernelDependencyChainServiceLookupUsesModuleNameIndex},
        {TEST_SERVICE_REGISTRY, KernelServiceRegistryResolveAndMissingService},
        {TEST_INVALID_LIFECYCLE, KernelInvalidLifecycleRejectsOutOfOrderCalls},
        {TEST_RUNTIME_APP_ZERO_WORLD, KernelRuntimeAppZeroWorldFixedFrameLoop},
        {TEST_RUNTIME_APP_FRAME_CONTEXT, KernelRuntimeAppFrameContextExposesValueContract},
        {TEST_RUNTIME_APP_FAILURE_PROPAGATION, KernelRuntimeAppUpdateFailurePropagatesAndShutsDown}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
