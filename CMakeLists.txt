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

add_library(YuMemory
    src/yuengine/memory/src/CountingMemoryTracker.cpp
    src/yuengine/memory/src/DisabledMemoryTracker.cpp
)
yuengine_apply_compile_options(YuMemory)
target_include_directories(YuMemory
    PUBLIC
        src/yuengine/memory/include
)

add_library(YuDiagnostics
    src/yuengine/diagnostics/src/BoundedDiagnosticsChannel.cpp
    src/yuengine/diagnostics/src/BoundedInMemoryLogSink.cpp
    src/yuengine/diagnostics/src/DefaultLogSink.cpp
    src/yuengine/diagnostics/src/DisabledDiagnosticsChannel.cpp
    src/yuengine/diagnostics/src/DisabledLogSink.cpp
)
yuengine_apply_compile_options(YuDiagnostics)
target_include_directories(YuDiagnostics
    PUBLIC
        src/yuengine/diagnostics/include
)
target_link_libraries(YuDiagnostics PUBLIC YuMemory)

add_library(YuThread
    src/yuengine/thread/src/BoundedTaskQueue.cpp
    src/yuengine/thread/src/InlineTaskExecutor.cpp
)
yuengine_apply_compile_options(YuThread)
target_include_directories(YuThread
    PUBLIC
        src/yuengine/thread/include
)
target_link_libraries(YuThread PUBLIC YuMemory)

add_library(YuFile
    src/yuengine/file/src/FileReadResult.cpp
    src/yuengine/file/src/LooseFileSource.cpp
    src/yuengine/file/src/MountId.cpp
    src/yuengine/file/src/MountPoint.cpp
    src/yuengine/file/src/MountTable.cpp
    src/yuengine/file/src/NormalizedPath.cpp
    src/yuengine/file/src/PathNormalizationResult.cpp
    src/yuengine/file/src/VirtualPath.cpp
)
yuengine_apply_compile_options(YuFile)
target_include_directories(YuFile
    PUBLIC
        src/yuengine/file/include
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
target_link_libraries(YuPlatform PUBLIC YuDiagnostics YuMemory)

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
add_test(NAME Platform_AllocationAccountingStatus_UsesMemoryHook COMMAND YuPlatformTests Platform_AllocationAccountingStatus_UsesMemoryHook)

add_executable(YuDiagnosticsTests
    tests/diagnostics/DiagnosticsTests.cpp
)
yuengine_apply_compile_options(YuDiagnosticsTests)
target_link_libraries(YuDiagnosticsTests PRIVATE YuPlatform)
add_test(NAME Logging_DisabledSink_DoesNotChangeBehavior COMMAND YuDiagnosticsTests Logging_DisabledSink_DoesNotChangeBehavior)
add_test(NAME Diagnostics_DisabledChannel_DoesNotChangeBehavior COMMAND YuDiagnosticsTests Diagnostics_DisabledChannel_DoesNotChangeBehavior)
add_test(NAME Diagnostics_BoundedChannel_RecordsEventsAndCounters COMMAND YuDiagnosticsTests Diagnostics_BoundedChannel_RecordsEventsAndCounters)
add_test(NAME Diagnostics_BoundedChannel_DropsWhenFull COMMAND YuDiagnosticsTests Diagnostics_BoundedChannel_DropsWhenFull)
add_test(NAME Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters COMMAND YuDiagnosticsTests Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters)
add_test(NAME Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown COMMAND YuDiagnosticsTests Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown)
add_test(NAME Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled COMMAND YuDiagnosticsTests Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled)
add_test(NAME Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate COMMAND YuDiagnosticsTests Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate)
add_test(NAME Diagnostics_NoReportDependency_ForRuntimeResults COMMAND YuDiagnosticsTests Diagnostics_NoReportDependency_ForRuntimeResults)
add_test(NAME Diagnostics_NoHiddenAllocation_UsesYuMemorySignal COMMAND YuDiagnosticsTests Diagnostics_NoHiddenAllocation_UsesYuMemorySignal)

add_executable(YuKernelTests
    tests/kernel/KernelTests.cpp
)
yuengine_apply_compile_options(YuKernelTests)
target_link_libraries(YuKernelTests PRIVATE YuKernel)
add_test(NAME Kernel_ModuleLifecycle_DependencyOrder COMMAND YuKernelTests Kernel_ModuleLifecycle_DependencyOrder)
add_test(NAME Kernel_ModuleStartupFailure_TearsDownStartedModules COMMAND YuKernelTests Kernel_ModuleStartupFailure_TearsDownStartedModules)
add_test(NAME Kernel_ServiceRegistry_ResolveAndMissingService COMMAND YuKernelTests Kernel_ServiceRegistry_ResolveAndMissingService)

add_executable(YuMemoryTests
    tests/memory/MemoryTests.cpp
)
yuengine_apply_compile_options(YuMemoryTests)
target_link_libraries(YuMemoryTests PRIVATE YuMemory)
add_test(NAME Memory_TrackerCountsAllocationAndFree COMMAND YuMemoryTests Memory_TrackerCountsAllocationAndFree)
add_test(NAME Memory_TrackerReportsPeakAndRetainedBytes COMMAND YuMemoryTests Memory_TrackerReportsPeakAndRetainedBytes)
add_test(NAME Memory_TrackerReportsLeakOnUnreleasedBytes COMMAND YuMemoryTests Memory_TrackerReportsLeakOnUnreleasedBytes)
add_test(NAME Memory_TrackerRejectsUnmatchedFree COMMAND YuMemoryTests Memory_TrackerRejectsUnmatchedFree)
add_test(NAME Memory_TrackerRecordsBudgetClass COMMAND YuMemoryTests Memory_TrackerRecordsBudgetClass)
add_test(NAME Memory_DisabledTracker_DoesNotChangeBehavior COMMAND YuMemoryTests Memory_DisabledTracker_DoesNotChangeBehavior)
add_test(NAME Memory_HotPathBudget_FailsOnTrackedAllocation COMMAND YuMemoryTests Memory_HotPathBudget_FailsOnTrackedAllocation)

add_executable(YuThreadTests
    tests/thread/ThreadTestContext.cpp
    tests/thread/ThreadTests.cpp
)
yuengine_apply_compile_options(YuThreadTests)
target_link_libraries(YuThreadTests PRIVATE YuThread)
add_test(NAME Thread_QueueEnqueueWithinCapacity_Succeeds COMMAND YuThreadTests Thread_QueueEnqueueWithinCapacity_Succeeds)
add_test(NAME Thread_QueueEnqueueBeyondCapacity_Rejects COMMAND YuThreadTests Thread_QueueEnqueueBeyondCapacity_Rejects)
add_test(NAME Thread_DrainExecutesTasks_InDeterministicOrder COMMAND YuThreadTests Thread_DrainExecutesTasks_InDeterministicOrder)
add_test(NAME Thread_TaskFailure_ReturnsFailedResult COMMAND YuThreadTests Thread_TaskFailure_ReturnsFailedResult)
add_test(NAME Thread_ShutdownRejectsNewSubmission COMMAND YuThreadTests Thread_ShutdownRejectsNewSubmission)
add_test(NAME Thread_ShutdownDrainPolicy_ExecutesQueuedTasks COMMAND YuThreadTests Thread_ShutdownDrainPolicy_ExecutesQueuedTasks)
add_test(NAME Thread_ShutdownCancelPolicy_CancelsQueuedTasks COMMAND YuThreadTests Thread_ShutdownCancelPolicy_CancelsQueuedTasks)
add_test(NAME Thread_QueueCapacity_DoesNotGrowDuringFixture COMMAND YuThreadTests Thread_QueueCapacity_DoesNotGrowDuringFixture)
add_test(NAME Thread_DiagnosticsDisabled_DoesNotChangeBehavior COMMAND YuThreadTests Thread_DiagnosticsDisabled_DoesNotChangeBehavior)

add_executable(YuFileTests
    tests/file/FileTests.cpp
)
yuengine_apply_compile_options(YuFileTests)
target_link_libraries(YuFileTests PRIVATE YuFile)
target_compile_definitions(YuFileTests PRIVATE YUENGINE_FILE_FIXTURE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/file")
add_test(NAME File_PathNormalize_RemovesDotAndRepeatedSeparators COMMAND YuFileTests File_PathNormalize_RemovesDotAndRepeatedSeparators)
add_test(NAME File_PathNormalize_RejectsTraversalOutsideRoot COMMAND YuFileTests File_PathNormalize_RejectsTraversalOutsideRoot)
add_test(NAME File_PathNormalize_RejectsEmptyAndAbsolutePath COMMAND YuFileTests File_PathNormalize_RejectsEmptyAndAbsolutePath)
add_test(NAME File_MountTable_RejectsDuplicateMount COMMAND YuFileTests File_MountTable_RejectsDuplicateMount)
add_test(NAME File_MountTable_UsesDeterministicPriorityOrder COMMAND YuFileTests File_MountTable_UsesDeterministicPriorityOrder)
add_test(NAME File_MountTable_ReportsMissingMountOrFile COMMAND YuFileTests File_MountTable_ReportsMissingMountOrFile)
add_test(NAME File_LooseFixtureRead_ReturnsExactBytes COMMAND YuFileTests File_LooseFixtureRead_ReturnsExactBytes)
add_test(NAME File_ReadSnapshot_RecordsCountsAndBytes COMMAND YuFileTests File_ReadSnapshot_RecordsCountsAndBytes)
add_test(NAME File_DiagnosticsDisabled_DoesNotChangeBehavior COMMAND YuFileTests File_DiagnosticsDisabled_DoesNotChangeBehavior)
