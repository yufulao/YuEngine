cmake_minimum_required(VERSION 3.24)

project(YuEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(yuengine_apply_compile_options target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive-)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endfunction()

add_library(YuDiagnostics
    src/yuengine/diagnostics/src/BoundedInMemoryLogSink.cpp
    src/yuengine/diagnostics/src/DefaultLogSink.cpp
    src/yuengine/diagnostics/src/DisabledLogSink.cpp
)
yuengine_apply_compile_options(YuDiagnostics)
target_include_directories(YuDiagnostics
    PUBLIC
        src/yuengine/diagnostics/include
)

add_library(YuPlatform
    src/yuengine/platform/src/FixedFrameClock.cpp
    src/yuengine/platform/src/HeadlessHost.cpp
    src/yuengine/platform/src/HostError.cpp
)
yuengine_apply_compile_options(YuPlatform)
target_include_directories(YuPlatform
    PUBLIC
        src/yuengine/platform/include
)
target_link_libraries(YuPlatform PUBLIC YuDiagnostics)

add_library(YuKernel
    src/yuengine/kernel/src/EngineKernel.cpp
    src/yuengine/kernel/src/KernelHostRuntime.cpp
    src/yuengine/kernel/src/KernelResult.cpp
    src/yuengine/kernel/src/ServiceRegistry.cpp
)
yuengine_apply_compile_options(YuKernel)
target_include_directories(YuKernel
    PUBLIC
        src/yuengine/kernel/include
)
target_link_libraries(YuKernel PUBLIC YuPlatform)

add_executable(YuHeadless
    apps/yuengine_headless/main.cpp
)
yuengine_apply_compile_options(YuHeadless)
target_link_libraries(YuHeadless PRIVATE YuKernel)

enable_testing()

add_executable(YuPlatformTests
    tests/platform/PlatformTests.cpp
)
yuengine_apply_compile_options(YuPlatformTests)
target_link_libraries(YuPlatformTests PRIVATE YuPlatform)
add_test(NAME Host_StartTickShutdown_Deterministic COMMAND YuPlatformTests Host_StartTickShutdown_Deterministic)
add_test(NAME Host_TimerMonotonic_ForFixedTicks COMMAND YuPlatformTests Host_TimerMonotonic_ForFixedTicks)

add_executable(YuDiagnosticsTests
    tests/diagnostics/DiagnosticsTests.cpp
)
yuengine_apply_compile_options(YuDiagnosticsTests)
target_link_libraries(YuDiagnosticsTests PRIVATE YuPlatform)
add_test(NAME Logging_DisabledSink_DoesNotChangeBehavior COMMAND YuDiagnosticsTests Logging_DisabledSink_DoesNotChangeBehavior)

add_executable(YuKernelTests
    tests/kernel/KernelTests.cpp
)
yuengine_apply_compile_options(YuKernelTests)
target_link_libraries(YuKernelTests PRIVATE YuKernel)
add_test(NAME Kernel_ModuleLifecycle_DependencyOrder COMMAND YuKernelTests Kernel_ModuleLifecycle_DependencyOrder)
add_test(NAME Kernel_ModuleStartupFailure_TearsDownStartedModules COMMAND YuKernelTests Kernel_ModuleStartupFailure_TearsDownStartedModules)
add_test(NAME Kernel_ServiceRegistry_ResolveAndMissingService COMMAND YuKernelTests Kernel_ServiceRegistry_ResolveAndMissingService)
