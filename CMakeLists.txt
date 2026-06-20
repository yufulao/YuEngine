cmake_minimum_required(VERSION 3.24)

project(YuEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(yuengine_apply_compile_options target_name)
    target_compile_features(${target_name} PUBLIC cxx_std_20)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /source-charset:utf-8)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
endfunction()

add_library(YuMemory
    Src/YuEngine/Memory/Src/CountingMemoryTracker.cpp
    Src/YuEngine/Memory/Src/DisabledMemoryTracker.cpp
)
yuengine_apply_compile_options(YuMemory)
target_include_directories(YuMemory
    PUBLIC
        Src/YuEngine/Memory/Include
)

add_library(YuScript
    Src/YuEngine/Script/Src/ScriptNativeRegistry.cpp
    Src/YuEngine/Script/Src/ScriptRuntimePhaseDispatchAdapter.cpp
)
yuengine_apply_compile_options(YuScript)
target_include_directories(YuScript
    PUBLIC
        Src/YuEngine/Script/Include
)
target_link_libraries(YuScript PUBLIC YuMemory)

add_library(YuWorld
    Src/YuEngine/World/Src/WorldInstance.cpp
    Src/YuEngine/World/Src/WorldComponentAttachmentBridge.cpp
    Src/YuEngine/World/Src/WorldComponentAttachmentSnapshotBridge.cpp
    Src/YuEngine/World/Src/WorldComponentQueryBridge.cpp
    Src/YuEngine/World/Src/WorldComponentResourceBindingBridge.cpp
    Src/YuEngine/World/Src/WorldComponentResourceBindingRestoreBridge.cpp
    Src/YuEngine/World/Src/WorldComponentResourceBindingSnapshotBridge.cpp
    Src/YuEngine/World/Src/WorldSceneAssemblyBridge.cpp
    Src/YuEngine/World/Src/WorldSceneAssemblyManifestStreamBridge.cpp
    Src/YuEngine/World/Src/WorldSceneActiveRestoreGateBridge.cpp
    Src/YuEngine/World/Src/WorldSceneApplyTimeRestoreProofBridge.cpp
    Src/YuEngine/World/Src/WorldSceneDecodedRestorePlanBridge.cpp
    Src/YuEngine/World/Src/WorldSceneObjectTransformManifestStreamBridge.cpp
    Src/YuEngine/World/Src/WorldSceneObjectTransformRestoreBridge.cpp
    Src/YuEngine/World/Src/WorldSceneRecordValueStreamBridge.cpp
    Src/YuEngine/World/Src/WorldIdentityBaseline.cpp
    Src/YuEngine/World/Src/WorldObjectIdentityBridge.cpp
    Src/YuEngine/World/Src/WorldKernelModule.cpp
    Src/YuEngine/World/Src/WorldResourceBindingBridge.cpp
    Src/YuEngine/World/Src/WorldScriptDispatchBridge.cpp
    Src/YuEngine/World/Src/WorldSerializeSnapshotBridge.cpp
    Src/YuEngine/World/Src/WorldTransformBridge.cpp
)
yuengine_apply_compile_options(YuWorld)
target_include_directories(YuWorld
    PUBLIC
        Src/YuEngine/World/Include
)
target_link_libraries(YuWorld PUBLIC YuMemory)
target_link_libraries(YuWorld PUBLIC YuResource)
target_link_libraries(YuWorld PUBLIC YuScript)
target_link_libraries(YuWorld PUBLIC YuSerialize)

add_library(YuDiagnostics
    Src/YuEngine/Diagnostics/Src/BoundedDiagnosticsChannel.cpp
    Src/YuEngine/Diagnostics/Src/BoundedInMemoryLogSink.cpp
    Src/YuEngine/Diagnostics/Src/DefaultLogSink.cpp
    Src/YuEngine/Diagnostics/Src/DisabledDiagnosticsChannel.cpp
    Src/YuEngine/Diagnostics/Src/DisabledLogSink.cpp
    Src/YuEngine/Diagnostics/Src/RuntimeDiagnosticsCounterRecorder.cpp
    Src/YuEngine/Diagnostics/Src/RuntimeDiagnosticsOverlayHook.cpp
)
yuengine_apply_compile_options(YuDiagnostics)
target_include_directories(YuDiagnostics
    PUBLIC
        Src/YuEngine/Diagnostics/Include
)
target_link_libraries(YuDiagnostics PUBLIC YuMemory)

add_library(YuThread
    Src/YuEngine/Thread/Src/BoundedTaskQueue.cpp
    Src/YuEngine/Thread/Src/InlineTaskExecutor.cpp
    Src/YuEngine/Thread/Src/ThreadWorker.cpp
)
yuengine_apply_compile_options(YuThread)
target_include_directories(YuThread
    PUBLIC
        Src/YuEngine/Thread/Include
)
target_link_libraries(YuThread PUBLIC YuMemory)

add_library(YuFile
    Src/YuEngine/File/Src/AsyncFileReadQueue.cpp
    Src/YuEngine/File/Src/FileReadResult.cpp
    Src/YuEngine/File/Src/FileWriteResult.cpp
    Src/YuEngine/File/Src/LooseFileSource.cpp
    Src/YuEngine/File/Src/MountId.cpp
    Src/YuEngine/File/Src/MountPoint.cpp
    Src/YuEngine/File/Src/MountTable.cpp
    Src/YuEngine/File/Src/NormalizedPath.cpp
    Src/YuEngine/File/Src/PathNormalizationResult.cpp
    Src/YuEngine/File/Src/VirtualPath.cpp
)
yuengine_apply_compile_options(YuFile)
target_include_directories(YuFile
    PUBLIC
        Src/YuEngine/File/Include
)
target_link_libraries(YuFile PUBLIC YuMemory)
target_link_libraries(YuFile PRIVATE YuThread)

add_library(YuRHI
    Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
    Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
    Src/YuEngine/Rhi/Src/RhiCommandList.cpp
)
yuengine_apply_compile_options(YuRHI)
target_include_directories(YuRHI
    PUBLIC
        Src/YuEngine/Rhi/Include
)
if(WIN32)
    target_link_libraries(YuRHI PRIVATE d3d11 dxgi)
endif()

add_library(YuAudio
    Src/YuEngine/Audio/Src/AudioCallbackDeviceWindows.cpp
    Src/YuEngine/Audio/Src/AudioPcmStreamQueueCallbackBridge.cpp
    Src/YuEngine/Audio/Src/TestAudioDevice.cpp
)
yuengine_apply_compile_options(YuAudio)
target_include_directories(YuAudio
    PUBLIC
        Src/YuEngine/Audio/Include
)
if(WIN32)
    target_link_libraries(YuAudio PRIVATE xaudio2 ole32)
endif()

add_library(YuResource
    Src/YuEngine/Resource/Src/ResourceLogicalKey.cpp
    Src/YuEngine/Resource/Src/ResourceRegistry.cpp
)
yuengine_apply_compile_options(YuResource)
target_include_directories(YuResource
    PUBLIC
        Src/YuEngine/Resource/Include
)
target_link_libraries(YuResource PUBLIC YuMemory)

add_library(YuAudioResource
    Src/YuEngine/AudioResource/Src/AudioResourcePcmPacketImportBridge.cpp
)
yuengine_apply_compile_options(YuAudioResource)
target_include_directories(YuAudioResource
    PUBLIC
        Src/YuEngine/AudioResource/Include
)
target_link_libraries(YuAudioResource PUBLIC YuAudio YuResource)

add_library(YuObject
    Src/YuEngine/Object/Src/ObjectRegistry.cpp
)
yuengine_apply_compile_options(YuObject)
target_include_directories(YuObject
    PUBLIC
        Src/YuEngine/Object/Include
)
target_link_libraries(YuObject PUBLIC YuMemory)
target_link_libraries(YuWorld PUBLIC YuObject)

add_library(YuUiCore
    Src/YuEngine/UiCore/Src/UiButtonComponent.cpp
    Src/YuEngine/UiCore/Src/UiDirtyTracker.cpp
    Src/YuEngine/UiCore/Src/UiDrawBatcher.cpp
    Src/YuEngine/UiCore/Src/UiDrawListBuilder.cpp
    Src/YuEngine/UiCore/Src/UiDynamicAtlasPacker.cpp
    Src/YuEngine/UiCore/Src/UiFontGlyphAtlas.cpp
    Src/YuEngine/UiCore/Src/UiGridViewSemantics.cpp
    Src/YuEngine/UiCore/Src/UiGridViewVirtualizer.cpp
    Src/YuEngine/UiCore/Src/UiHitTestResolver.cpp
    Src/YuEngine/UiCore/Src/UiImageComponent.cpp
    Src/YuEngine/UiCore/Src/UiInvalidationModel.cpp
    Src/YuEngine/UiCore/Src/UiLayoutPass.cpp
    Src/YuEngine/UiCore/Src/UiNodeTree.cpp
    Src/YuEngine/UiCore/Src/UiRectMath.cpp
    Src/YuEngine/UiCore/Src/UiSliderComponent.cpp
    Src/YuEngine/UiCore/Src/UiStaticAtlasMetadata.cpp
    Src/YuEngine/UiCore/Src/UiTextComponent.cpp
)
yuengine_apply_compile_options(YuUiCore)
target_include_directories(YuUiCore
    PUBLIC
        Src/YuEngine/UiCore/Include
)

add_library(YuSerialize
    Src/YuEngine/Serialize/Src/RuntimeConfigStream.cpp
    Src/YuEngine/Serialize/Src/SerializeReader.cpp
    Src/YuEngine/Serialize/Src/SerializeWriter.cpp
)
yuengine_apply_compile_options(YuSerialize)
target_include_directories(YuSerialize
    PUBLIC
        Src/YuEngine/Serialize/Include
)
target_link_libraries(YuSerialize PUBLIC YuMemory)

add_library(YuPackage
    Src/YuEngine/Package/Src/PackageLoadPlanResult.cpp
    Src/YuEngine/Package/Src/PackageRegistrationResult.cpp
    Src/YuEngine/Package/Src/PackageRegistry.cpp
    Src/YuEngine/Package/Src/PackageSourceKey.cpp
)
yuengine_apply_compile_options(YuPackage)
target_include_directories(YuPackage
    PUBLIC
        Src/YuEngine/Package/Include
)
target_link_libraries(YuPackage PUBLIC YuMemory YuResource)

add_library(YuStreaming
    Src/YuEngine/Streaming/Src/PackageResourceStagingQueue.cpp
    Src/YuEngine/Streaming/Src/ResourceDecodedTextureBridge.cpp
    Src/YuEngine/Streaming/Src/ResourceStreamingPipeline.cpp
    Src/YuEngine/Streaming/Src/ResourceUploadCommitQueue.cpp
    Src/YuEngine/Streaming/Src/ResourceUploadQueue.cpp
)
yuengine_apply_compile_options(YuStreaming)
target_include_directories(YuStreaming
    PUBLIC
        Src/YuEngine/Streaming/Include
)
target_link_libraries(YuStreaming PUBLIC YuMemory YuPackage YuResource YuFile YuRHI)

add_library(YuAsset
    Src/YuEngine/Asset/Src/AssetManager.cpp
    Src/YuEngine/Asset/Src/AssetRuntimeFixture.cpp
)
yuengine_apply_compile_options(YuAsset)
target_include_directories(YuAsset
    PUBLIC
        Src/YuEngine/Asset/Include
)
target_link_libraries(YuAsset PUBLIC YuResource YuStreaming YuAudioResource)

add_library(YuRenderCore
    Src/YuEngine/RenderCore/Src/MaterialBindingFixture.cpp
    Src/YuEngine/RenderCore/Src/RenderCamera.cpp
    Src/YuEngine/RenderCore/Src/RenderCameraShaderConstantsWriter.cpp
    Src/YuEngine/RenderCore/Src/RenderDrawPacket.cpp
    Src/YuEngine/RenderCore/Src/RenderDrawableFramePipeline.cpp
    Src/YuEngine/RenderCore/Src/RenderFixturePass.cpp
    Src/YuEngine/RenderCore/Src/RenderFramePacketFixture.cpp
    Src/YuEngine/RenderCore/Src/RenderGraphExecutionPlan.cpp
    Src/YuEngine/RenderCore/Src/RenderGraphSkeleton.cpp
    Src/YuEngine/RenderCore/Src/RenderMaterial.cpp
    Src/YuEngine/RenderCore/Src/RenderShaderProgram.cpp
    Src/YuEngine/RenderCore/Src/RenderSubmissionBatchFixture.cpp
    Src/YuEngine/RenderCore/Src/RenderSwapchainFramePipeline.cpp
    Src/YuEngine/RenderCore/Src/RenderViewPacket.cpp
)
yuengine_apply_compile_options(YuRenderCore)
target_include_directories(YuRenderCore
    PUBLIC
        Src/YuEngine/RenderCore/Include
)
target_link_libraries(YuRenderCore PUBLIC YuRHI)

add_library(YuUiRenderCoreBridge
    Src/YuEngine/UiRenderCoreBridge/Src/UiRenderCoreBridge.cpp
)
yuengine_apply_compile_options(YuUiRenderCoreBridge)
target_include_directories(YuUiRenderCoreBridge
    PUBLIC
        Src/YuEngine/UiRenderCoreBridge/Include
)
target_link_libraries(YuUiRenderCoreBridge PUBLIC YuUiCore YuRenderCore)

add_library(YuUiRuntime
    Src/YuEngine/UiRuntime/Src/BaseUiController.cpp
    Src/YuEngine/UiRuntime/Src/UiManagerFullscreenStack.cpp
    Src/YuEngine/UiRuntime/Src/UiManagerLayerModel.cpp
    Src/YuEngine/UiRuntime/Src/UiManagerPanelMap.cpp
    Src/YuEngine/UiRuntime/Src/UiManagerPopupStack.cpp
    Src/YuEngine/UiRuntime/Src/UiSimplePopupToastRepresentativeController.cpp
    Src/YuEngine/UiRuntime/Src/UiPanelRegistry.cpp
)
yuengine_apply_compile_options(YuUiRuntime)
target_include_directories(YuUiRuntime
    PUBLIC
        Src/YuEngine/UiRuntime/Include
)

add_library(YuRenderScene
    Src/YuEngine/RenderScene/Src/RenderSceneContractQueue.cpp
)
yuengine_apply_compile_options(YuRenderScene)
target_include_directories(YuRenderScene
    PUBLIC
        Src/YuEngine/RenderScene/Include
)
target_link_libraries(YuRenderScene PUBLIC YuAsset YuRenderCore YuWorld)

add_library(YuAudioScene
    Src/YuEngine/AudioScene/Src/AudioSceneContractQueue.cpp
)
yuengine_apply_compile_options(YuAudioScene)
target_include_directories(YuAudioScene
    PUBLIC
        Src/YuEngine/AudioScene/Include
)
target_link_libraries(YuAudioScene PUBLIC YuAsset YuAudio)

add_library(YuInput
    Src/YuEngine/Input/Src/InputCommandMapper.cpp
    Src/YuEngine/Input/Src/InputBridge.cpp
    Src/YuEngine/Input/Src/InputBridgeWindows.cpp
    Src/YuEngine/Input/Src/InputReplay.cpp
)
yuengine_apply_compile_options(YuInput)
target_include_directories(YuInput
    PUBLIC
        Src/YuEngine/Input/Include
)
if(WIN32)
    target_link_libraries(YuInput PRIVATE xinput9_1_0)
endif()

add_library(YuPlatform
    Src/YuEngine/Platform/Src/FixedFrameClock.cpp
    Src/YuEngine/Platform/Src/HeadlessHost.cpp
    Src/YuEngine/Platform/Src/HostError.cpp
    Src/YuEngine/Platform/Src/WindowsPlatformWindow.cpp
)
yuengine_apply_compile_options(YuPlatform)
target_include_directories(YuPlatform
    PUBLIC
        Src/YuEngine/Platform/Include
)
target_link_libraries(YuPlatform PUBLIC YuDiagnostics YuMemory)
if(WIN32)
    target_link_libraries(YuPlatform PRIVATE user32)
endif()

add_library(YuKernel
    Src/YuEngine/Kernel/Src/EngineKernel.cpp
    Src/YuEngine/Kernel/Src/KernelHostRuntime.cpp
    Src/YuEngine/Kernel/Src/KernelResult.cpp
    Src/YuEngine/Kernel/Src/RuntimeApp.cpp
    Src/YuEngine/Kernel/Src/ServiceRegistry.cpp
)
yuengine_apply_compile_options(YuKernel)
target_include_directories(YuKernel
    PUBLIC
        Src/YuEngine/Kernel/Include
)
target_link_libraries(YuKernel PUBLIC YuPlatform)
target_link_libraries(YuWorld PUBLIC YuKernel)

add_library(YuHardware
    Src/YuEngine/Hardware/Src/HardwareFrameHost.cpp
)
yuengine_apply_compile_options(YuHardware)
target_include_directories(YuHardware
    PUBLIC
        Src/YuEngine/Hardware/Include
)
target_link_libraries(YuHardware PUBLIC YuAudio YuInput YuPlatform YuRenderCore YuRHI)

add_executable(YuHeadless
    Apps/YuEngineHeadless/Main.cpp
)
yuengine_apply_compile_options(YuHeadless)
target_link_libraries(YuHeadless PRIVATE YuKernel)

add_library(YuUiEditorShellCore
    Tools/UiEditor/Src/UiEditorResourceReferenceValidator.cpp
    Tools/UiEditor/Src/UiEditorShellState.cpp
)
yuengine_apply_compile_options(YuUiEditorShellCore)
target_include_directories(YuUiEditorShellCore
    PUBLIC
        Tools/UiEditor/Include
)
target_link_libraries(YuUiEditorShellCore PUBLIC YuUiCore)
target_link_libraries(YuUiEditorShellCore PRIVATE YuUiRenderCoreBridge YuRHI)

add_executable(YuUiEditorShell
    Tools/UiEditor/Src/Main.cpp
)
yuengine_apply_compile_options(YuUiEditorShell)
target_link_libraries(YuUiEditorShell PRIVATE YuUiEditorShellCore)

add_executable(YuUiCoreSmokeSample
    Samples/UiCoreSmoke/Source/Main.cpp
    Samples/UiCoreSmoke/Source/UiCoreSmokeSample.cpp
)
yuengine_apply_compile_options(YuUiCoreSmokeSample)
target_include_directories(YuUiCoreSmokeSample PRIVATE Samples/UiCoreSmoke/Source)
target_compile_definitions(YuUiCoreSmokeSample PRIVATE
    YU_UI_CORE_SMOKE_DEFAULT_LAYOUT="${CMAKE_CURRENT_SOURCE_DIR}/Samples/UiCoreSmoke/Layouts/SimpleWindow.YuUILayout.json"
)
target_link_libraries(YuUiCoreSmokeSample PRIVATE YuUiRenderCoreBridge YuRHI)

add_executable(YuUiComponentSmokeSample
    Samples/UiComponentSmoke/Source/Main.cpp
    Samples/UiComponentSmoke/Source/UiComponentSmokeSample.cpp
)
yuengine_apply_compile_options(YuUiComponentSmokeSample)
target_include_directories(YuUiComponentSmokeSample PRIVATE Samples/UiComponentSmoke/Source)
target_compile_definitions(YuUiComponentSmokeSample PRIVATE
    YU_UI_COMPONENT_SMOKE_DEFAULT_LAYOUT="${CMAKE_CURRENT_SOURCE_DIR}/Samples/UiComponentSmoke/Layouts/ComponentWindow.YuUILayout.json"
)
target_link_libraries(YuUiComponentSmokeSample PRIVATE YuUiCore)

enable_testing()

function(yuengine_add_labeled_test test_name test_target)
    if(ARGC LESS 3)
        message(FATAL_ERROR "yuengine_add_labeled_test requires at least one label")
    endif()

    set(labels ${ARGN})
    list(REMOVE_DUPLICATES labels)

    add_test(NAME ${test_name} COMMAND ${test_target} ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS "${labels}")
endfunction()

add_executable(YuPlatformTests
    Tests/Platform/PlatformTests.cpp
)
yuengine_apply_compile_options(YuPlatformTests)
target_link_libraries(YuPlatformTests PRIVATE YuPlatform)
yuengine_add_labeled_test(Host_StartTickShutdown_Deterministic YuPlatformTests Fast ModuleFixture Platform EvidenceOracle)
yuengine_add_labeled_test(Host_TimerMonotonic_ForFixedTicks YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(Platform_AllocationAccountingStatus_UsesMemoryHook YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(PlatformWindow_DescDefaultValuesAreBounded YuPlatformTests Fast ModuleFixture Platform EvidenceOracle)
yuengine_add_labeled_test(PlatformWindow_InvalidDescriptorRejected YuPlatformTests Fast ModuleFixture Platform EvidenceOracle)
yuengine_add_labeled_test(PlatformWindow_SurfaceDefaultInvalid YuPlatformTests Fast ModuleFixture Platform EvidenceOracle)
yuengine_add_labeled_test(PlatformWindow_InitialSnapshotIsEmpty YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(PlatformWindow_DestroyIsIdempotent YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(PlatformWindow_PollEventsRejectsNullOutputBuffer YuPlatformTests Fast ModuleFixture Platform EvidenceOracle)
yuengine_add_labeled_test(PlatformWindow_PollEventsBeforeCreateReturnsNotCreated YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(PlatformWindow_OperationsBeforeCreateReturnNotCreated YuPlatformTests Fast ModuleFixture Platform)
yuengine_add_labeled_test(PlatformWindow_QueueCapacityLimitIsBounded YuPlatformTests Fast ModuleFixture Platform PerformanceSmoke)
yuengine_add_labeled_test(PlatformWindow_PublicTypesArePlainValues YuPlatformTests Fast ModuleFixture Platform PerformanceSmoke EvidenceOracle)

add_executable(YuDiagnosticsTests
    Tests/Diagnostics/DiagnosticsTests.cpp
)
yuengine_apply_compile_options(YuDiagnosticsTests)
target_link_libraries(YuDiagnosticsTests PRIVATE YuPlatform)
yuengine_add_labeled_test(Logging_DisabledSink_DoesNotChangeBehavior YuDiagnosticsTests Fast ModuleFixture Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Logging_ModuleFilterAndDynamicSwitch YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_DisabledChannel_DoesNotChangeBehavior YuDiagnosticsTests Fast ModuleFixture Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_BoundedChannel_RecordsEventsAndCounters YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_BoundedChannel_DropsWhenFull YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_ChannelSnapshot_ReportsAcceptedDroppedAndCounters YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_ChannelStopped_DoesNotMutateAfterShutdown YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_ChannelRejectsUnknownIds_WhenValidationEnabled YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_CounterOverflow_ReturnsExplicitStatusAndDoesNotMutate YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_NoReportDependency_ForRuntimeResults YuDiagnosticsTests Fast ModuleFixture Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_NoHiddenAllocation_UsesYuMemorySignal YuDiagnosticsTests Fast ModuleFixture Diagnostics PerformanceSmoke)
yuengine_add_labeled_test(Diagnostics_RuntimeCounters_RegisterAndRecordBoundedValues YuDiagnosticsTests Fast ModuleFixture Diagnostics)
yuengine_add_labeled_test(Diagnostics_RuntimeCounters_DisabledChannelDoesNotChangeRuntimeValues YuDiagnosticsTests Fast ModuleFixture Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_RuntimeCounters_RejectNullChannelWithoutMutation YuDiagnosticsTests Fast ModuleFixture Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_RuntimeCounters_ValueTypesArePlainValues YuDiagnosticsTests Fast ModuleFixture Diagnostics PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_OverlayHookProposalStaysOptionalToolingPlane YuDiagnosticsTests Fast ModuleFixture Diagnostics L1 EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_OverlayHookRejectsRuntimeDependency YuDiagnosticsTests Fast ModuleFixture Diagnostics L1 EvidenceOracle)
yuengine_add_labeled_test(Diagnostics_OverlayHookDisabledDoesNotChangeRuntimeValues YuDiagnosticsTests Fast ModuleFixture Diagnostics L1 EvidenceOracle)

add_executable(YuKernelTests
    Tests/Kernel/KernelTests.cpp
)
yuengine_apply_compile_options(YuKernelTests)
target_link_libraries(YuKernelTests PRIVATE YuKernel)
yuengine_add_labeled_test(Kernel_ModuleLifecycle_DependencyOrder YuKernelTests Fast ModuleFixture Kernel)
yuengine_add_labeled_test(Kernel_ModuleStartupFailure_TearsDownStartedModules YuKernelTests Fast ModuleFixture Kernel)
yuengine_add_labeled_test(Kernel_DependencyChainServiceLookup_UsesModuleNameIndex YuKernelTests Fast ModuleFixture Kernel)
yuengine_add_labeled_test(Kernel_ServiceRegistry_ResolveAndMissingService YuKernelTests Fast ModuleFixture Kernel)
yuengine_add_labeled_test(Kernel_InvalidLifecycle_RejectsOutOfOrderCalls YuKernelTests Fast ModuleFixture Kernel)
yuengine_add_labeled_test(Kernel_RuntimeAppZeroWorld_FixedFrameLoop YuKernelTests Fast ModuleFixture Kernel RuntimeApp EvidenceOracle)
yuengine_add_labeled_test(Kernel_RuntimeAppFrameContext_ExposesValueContract YuKernelTests Fast ModuleFixture Kernel RuntimeApp EvidenceOracle)
yuengine_add_labeled_test(Kernel_RuntimeAppUpdateFailure_PropagatesAndShutsDown YuKernelTests Fast ModuleFixture Kernel RuntimeApp EvidenceOracle)

add_executable(YuMemoryTests
    Tests/Memory/MemoryTests.cpp
)
yuengine_apply_compile_options(YuMemoryTests)
target_link_libraries(YuMemoryTests PRIVATE YuMemory)
yuengine_add_labeled_test(Memory_TrackerCountsAllocationAndFree YuMemoryTests Fast ModuleFixture Memory)
yuengine_add_labeled_test(Memory_TrackerReportsPeakAndRetainedBytes YuMemoryTests Fast ModuleFixture Memory)
yuengine_add_labeled_test(Memory_TrackerReportsLeakOnUnreleasedBytes YuMemoryTests Fast ModuleFixture Memory)
yuengine_add_labeled_test(Memory_TrackerRejectsUnmatchedFree YuMemoryTests Fast ModuleFixture Memory)
yuengine_add_labeled_test(Memory_TrackerRecordsBudgetClass YuMemoryTests Fast ModuleFixture Memory PerformanceSmoke)
yuengine_add_labeled_test(Memory_DisabledTracker_DoesNotChangeBehavior YuMemoryTests Fast ModuleFixture Memory EvidenceOracle)
yuengine_add_labeled_test(Memory_HotPathBudget_FailsOnTrackedAllocation YuMemoryTests Fast ModuleFixture Memory PerformanceSmoke)
yuengine_add_labeled_test(Memory_TrackerRejectsBeyondFixedCapacityWithoutMutation YuMemoryTests Fast ModuleFixture Memory)
yuengine_add_labeled_test(Memory_TrackerEnforcesOwnerAndTagByteCapsWithoutMutation YuMemoryTests Fast ModuleFixture Memory)

add_executable(YuScriptTests
    Tests/Script/ScriptTests.cpp
)
yuengine_apply_compile_options(YuScriptTests)
target_link_libraries(YuScriptTests PRIVATE YuScript)
yuengine_add_labeled_test(Script_RegisterNativeCall_ReturnsStableId YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_RegisterDuplicateCall_ReturnsExplicitStatus YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_RegisterRejectsNullFunction YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_RegistryCapacityOverflow_DoesNotMutate YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_InvokeNativeCall_WritesResultDeterministically YuScriptTests Fast ModuleFixture Script EvidenceOracle)
yuengine_add_labeled_test(Script_InvokeRejectsUnknownCallId YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_InvokeRejectsArgumentCountMismatch YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_InvokeRejectsArgumentTypeMismatch YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_InvokeRejectsResultCountMismatch YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_NativeFailure_ReturnsExplicitStatus YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_CallPath_DoesNotGrowRegistryStorage YuScriptTests Fast ModuleFixture Script PerformanceSmoke)
yuengine_add_labeled_test(Script_NoWorldResourcePackageOrGameAdapterDependency YuScriptTests Fast ModuleFixture Script EvidenceOracle)
yuengine_add_labeled_test(Script_NoHiddenAllocation_UsesYuMemorySignal YuScriptTests Fast ModuleFixture Script PerformanceSmoke)
yuengine_add_labeled_test(Script_SnapshotReportsCountsAndLastStatus YuScriptTests Fast ModuleFixture Script)
yuengine_add_labeled_test(Script_RuntimePhaseDispatch_TraceMapsToCallIds YuScriptTests Fast ModuleFixture Script L1Runtime EvidenceOracle)
yuengine_add_labeled_test(Script_RuntimePhaseDispatch_MissingCallReturnsExplicitStatus YuScriptTests Fast ModuleFixture Script L1Runtime EvidenceOracle)
yuengine_add_labeled_test(Script_RuntimePhaseDispatch_InvalidSlotsReturnExplicitStatus YuScriptTests Fast ModuleFixture Script L1Runtime EvidenceOracle)
yuengine_add_labeled_test(Script_RuntimePhaseDispatch_InvalidPhaseDoesNotMutate YuScriptTests Fast ModuleFixture Script L1Runtime EvidenceOracle)

add_executable(YuWorldTests
    Tests/World/WorldTests.cpp
)
yuengine_apply_compile_options(YuWorldTests)
target_link_libraries(YuWorldTests PRIVATE YuWorld YuPlatform)
yuengine_add_labeled_test(World_CreateWithFixedCapacity_ReportsSnapshot YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_StartStop_RunsDeterministicLifecycle YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(World_UpdateRunsPhasesInFixedOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_UpdateBeforeStart_ReturnsExplicitStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_UpdateAfterStop_ReturnsExplicitStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_RegisterDuplicateObject_DoesNotMutate YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_RegisterOverflow_DoesNotMutate YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_DisabledObject_IsSkipped YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_UpdatePath_DoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(World_StopClearsActiveEntries YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_NoScriptResourcePackageFileOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(World_NoActorComponentOrTransformHierarchy YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(World_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_StartPublishesWorldService YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_UpdateTicksWorldInKernelOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_ShutdownStopsWorld YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_StartFailurePropagatesExplicitStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_UpdateFailureTriggersKernelTeardown YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_HeadlessHostDrivesWorldDeterministically YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldKernelModule_UpdatePathDoesNotGrowWorldStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldKernelModule_NoScriptResourcePackageFileOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldKernelModule_NoActorComponentOrTransformHierarchy YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldKernelModule_CoreWorldInstanceRemainsKernelFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindValidObject_AcquiresHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindRejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindRejectsInvalidObjectHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindRejectsDuplicateWorldObjectId YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_BindRejectsDuplicateObjectHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_RemoveReleasesHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_ClearReleasesAllHandles YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_StaleGenerationInvalidatesBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_UpdatePathDoesNotGrowWorldStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldObjectIdentityBridge_NoScriptResourcePackageFileOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldObjectIdentityBridge_NoActorComponentOrTransformHierarchy YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldObjectIdentityBridge_WorldInstanceCoreRemainsObjectFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RegisterValidObject_StoresTransform YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RegisterRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RegisterRejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RegisterRejectsDuplicateWorldObjectId YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RegisterRejectsCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_SetUpdatesExistingRecord YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_SetRejectsMissingRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_QueryReturnsStoredTransform YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_RemoveClearsRecord YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_ClearRemovesAllRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_UpdatePathDoesNotGrowWorldStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldTransformBridge_NoScriptResourcePackageFileObjectOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldTransformBridge_NoActorComponentSceneGraphOrHierarchy YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldTransformBridge_WorldInstanceCoreRemainsTransformStorageFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_BindPhaseCall_ReturnsStableBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_BindRejectsInvalidCallIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_BindRejectsDuplicatePhaseWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_BindRejectsCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchTraceInvokesPhasesInTraceOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchSkipsUnboundPhase YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchRejectsInvalidTraceBuffer YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchRejectsInvalidSlotBuffers YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchPropagatesScriptFailure YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_DispatchPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldScriptDispatchBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_NoActorComponentSceneGraphOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldScriptDispatchBridge_NoResourcePackageFileSerializeOrObjectOwnershipDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldScriptDispatchBridge_WorldInstanceCoreRemainsScriptFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldScriptDispatchBridge_ScriptRegistryCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WriteWorldSnapshot_RoundTripsDeterministically YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WritePhaseTraceRecordsInOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WriteOptionalTransformSnapshotCounters YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_ReadRejectsSmallTraceOutputWithoutOverrun YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WriteRejectsInvalidTraceBufferWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WriteRejectsTraceOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_SerializeFailureMapsExplicitStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_ReadFailureMapsExplicitStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_ReadRejectsInvalidEnumValuesWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_NoWorldMutationDuringReadWrite YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_ReadWritePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_NoFilePackageResourceSaveGameOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_NoActorComponentSceneGraphOrGameplayDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_WorldInstanceCoreRemainsSerializeFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSerializeSnapshotBridge_SerializeCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindValidResource_AcquiresHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsStaleResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsDuplicateWorldObjectId YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_RemoveReleasesHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_RemoveRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_RemoveRejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_RemoveReleaseFailureKeepsBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_ClearReleasesAllHandles YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_ClearRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_ClearReleaseFailurePreservesUnreleasedBindings YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_BoundResourceCannotRetireUntilReleased YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_QueryReturnsStoredBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_QueryIsReadOnlyAndBounded YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_UpdatePathDoesNotGrowWorldStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldResourceBindingBridge_DoesNotQueryOrMutateWorldInstance YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldResourceBindingBridge_WorldInstanceCoreRemainsResourceFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldResourceBindingBridge_ResourceCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindValidAttachmentResource_AcquiresHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsNullAttachmentSourceWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsInvalidComponentSlotWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsMissingAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsStaleResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsDuplicateComponentBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_RemoveReleasesHandle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_RemoveRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_RemoveRejectsMissingBindingWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_RemoveReleaseFailureKeepsBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_ClearReleasesAllHandles YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_ClearRejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_ClearReleaseFailurePreservesUnreleasedBindings YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_BoundResourceCannotRetireUntilReleased YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_QueryReturnsStoredBinding YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_QueryIsReadOnlyAndBounded YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_UpdatePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_DoesNotQueryOrMutateWorldInstance YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_NoActorComponentPayloadOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_WorldInstanceCoreRemainsComponentResourceFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingBridge_ResourceCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteReadRoundTripsBindingsInSlotOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteEmptyBridgeProducesZeroRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullSourceWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullWriterWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadWritesCallerOwnedRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullReaderWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadRejectsDuplicateBindingWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ReadDoesNotAcquireOrReleaseResources YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WriteReadPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_NoActorComponentPayloadOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingSnapshotBridge_ResourceCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RestoresRecordsInInputOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RestoresEmptyInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsNullDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsNullAttachmentSourceWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsNullInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentSlotWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsMissingAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsInvalidResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsStaleResourceHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsResourceTypeMismatchWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsDuplicateInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsDestinationCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_RejectsNonEmptyDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_AcquiresOnlyAfterPreflight YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_ResourceAcquireFailureDoesNotPartiallyRestore YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_NoActorComponentPayloadOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_WorldInstanceCoreRemainsRestoreFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentResourceBindingRestoreBridge_ResourceCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RestoresAttachmentAndBindingRecordsInInputOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RestoresEmptyAssemblyWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNullAttachmentDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNullBindingDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNullRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNullAttachmentInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNullBindingInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsInvalidAttachmentRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsInvalidBindingRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsMissingAttachmentForBindingWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsDuplicateAttachmentInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsDuplicateBindingInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsAttachmentCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsBindingCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RejectsNonEmptyDestinationsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_ValidatesResourceHandlesBeforeMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_BindingPreflightFailureDoesNotRestoreAttachments YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_ResourceAcquireFailureDoesNotPartiallyAssemble YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_RestorePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_NoActorComponentPayloadOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_NoObjectScriptSerializeThreadPlatformDiagnosticsDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_WorldInstanceCoreRemainsAssemblyFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyBridge_ResourceCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteReadRoundTripsAssemblyRecordsInInputOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteEmptyManifestProducesZeroRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullWriterWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullAttachmentInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullBindingInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsWriterCapacityBeforePartialWrite YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsInvalidAttachmentRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsInvalidBindingRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsMissingAttachmentForBindingWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsDuplicateAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteRejectsDuplicateBindingWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadWritesCallerOwnedOutputs YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullReaderWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullAttachmentOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullBindingOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsUnknownVersionWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsMalformedRecordCountWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsInvalidRecordsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadRejectsDuplicateRecordsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_ReadDoesNotRestoreActiveSidecars YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WriteReadPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_NoObjectTransformSceneLoadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_NoFilePackageResourceLoadDecodeUploadDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_WorldInstanceCoreRemainsManifestFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneAssemblyManifestStreamBridge_SerializeCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteReadRoundTripsObjectTransformRecordsInInputOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteEmptyManifestProducesZeroRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullWriterWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullIdentityInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullTransformInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsWriterCapacityBeforePartialWrite YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsInvalidIdentityRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsInvalidTransformRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsMissingIdentityForTransformWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateObjectHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateTransformWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadWritesCallerOwnedOutputs YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullReaderWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullIdentityOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullTransformOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsUnknownVersionWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsMalformedRecordCountWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsInvalidRecordsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsDuplicateRecordsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadRejectsTransformWithoutIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_ReadDoesNotRestoreActiveIdentityOrTransformSidecars YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteReadUsesDeterministicFixedByteTransformEncoding YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WriteReadPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_NoObjectConstructionOrDestruction YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_NoTransformHierarchySceneLoadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_NoFilePackageResourceLoadDecodeUploadDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformManifestStreamBridge_WorldObjectSerializeCoresRemainManifestFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneRecordValueStreamBridge_WriteReadRoundTripsAllSceneRecordFamilies YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneRecordValueStreamBridge_WriteRejectsCombinedWriterCapacityBeforePartialWrite YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneRecordValueStreamBridge_ReadRejectsUnsupportedAssemblyVersionWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneRecordValueStreamBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_PlansAllDecodedRecordFamiliesInDeterministicOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_PlansEmptyDecodedScene YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullIdentityInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullTransformInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullAttachmentInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullBindingInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullPlanOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNullWorldWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsPlanOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsIdentityCountExceededWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsTransformCountExceededWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsAttachmentCountExceededWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsBindingCountExceededWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsInvalidIdentityRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsInvalidTransformRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsInvalidAttachmentRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsInvalidBindingRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDuplicateIdentityWorldObjectIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDuplicateObjectHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDuplicateTransformWorldObjectIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDuplicateAttachmentTupleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDuplicateBindingTupleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsTransformWithoutIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsAttachmentWithoutIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsBindingWithoutAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsBindingWithoutIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsObjectAcquirePreflightFailureWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsResourceAcquirePreflightFailureWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyIdentityDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyTransformDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyAttachmentDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyBindingDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_RejectsDestinationCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_DoesNotCallActiveRestoreBridges YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_DoesNotMutateObjectOrResourceRegistries YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_PlanPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_NoStreamDecodeFilePackageResourceLoadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_NoObjectConstructionTransformHierarchyOrComponentLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneDecodedRestorePlanBridge_WorldActiveRestoreBridgesRemainPlanFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_ProvesAllFamiliesInDeterministicApplyOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_ProvesEmptyDecodedScene YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullIdentityInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullTransformInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullAttachmentInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullBindingInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullIdentityDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullTransformDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullAttachmentDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullBindingDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullWorldWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullObjectRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullResourceRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullPlanScratchWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullProofOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNullSliceOutputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsPlanScratchCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsProofOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsSliceOutputCapacityTooSmallWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsPlanFailureWithoutProofMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsObjectAcquirePreflightFailureWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsResourceAcquirePreflightFailureWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyIdentityDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyTransformDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyAttachmentDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyBindingDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_RejectsDestinationCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_ReplansCurrentInputsWithoutAcceptingCachedPlan YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_EmitsIdentityTransformAttachmentBindingSlicesOnly YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_DoesNotCallActiveRestoreBridges YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_DoesNotMutateDestinations YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_DoesNotMutateObjectOrResourceRegistries YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_ProofPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_NoStreamDecodeFilePackageResourceLoadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneApplyTimeRestoreProofBridge_NoCleanupRollbackOrCompensationPath YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneActiveRestoreGateBridge_EmitsCleanupPoliciesAfterApplyTimeProof YuWorldTests Fast ModuleFixture World L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldSceneActiveRestoreGateBridge_DoesNotMutateDestinationsOrRegistries YuWorldTests Fast ModuleFixture World L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldSceneActiveRestoreGateBridge_DestroyedObjectHandleRejectedWithoutSidecarMutation YuWorldTests Fast ModuleFixture World Object L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldSceneActiveRestoreGateBridge_GatePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World L1Runtime PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RestoresIdentityAndTransformRecordsInInputOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RestoresIdentityOnlyRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RestoresEmptyInputsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullWorldWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullObjectRegistryWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullIdentityDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullTransformDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullIdentityInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNullTransformInputWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsInvalidIdentityRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsInvalidTransformRecordWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsMissingWorldObjectWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsMissingIdentityForTransformWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsDuplicateIdentityWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsDuplicateObjectHandleWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsDuplicateTransformWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsIdentityCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsTransformCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RejectsNonEmptyDestinationsWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_ValidatesObjectHandlesBeforeMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_ObjectAcquirePreflightFailureDoesNotRestoreIdentitiesOrTransforms YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_RestorePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_NoHiddenAllocation_UsesYuMemorySignal YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_NoObjectConstructionOrDestruction YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_NoTransformHierarchyOrSceneGraph YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_NoComponentPayloadLifecycleOrScriptDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_NoSerializeFilePackageResourceLoadOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_WorldInstanceCoreRemainsObjectTransformRestoreFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldSceneObjectTransformRestoreBridge_ObjectCoreRemainsWorldFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddValidAttachment_StoresRecord YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddRejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddRejectsInvalidComponentSlotWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddRejectsDuplicateTypeForWorldObject YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_AddRejectsCapacityOverflowWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_QueryReturnsStoredAttachment YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_QueryRejectsMissingAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_QueryIsReadOnlyAndBounded YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_RemoveClearsAttachment YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_RemoveRejectsMissingAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_ClearRemovesAllAttachmentsInSlotOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_UpdatePathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_DoesNotQueryOrMutateWorldInstance YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_NoActorComponentBehaviorOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_NoFilePackageThreadPlatformDiagnosticsDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentBridge_WorldInstanceCoreRemainsAttachmentFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryTypeReturnsMatchingWorldObjectsInSlotOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryTypeReturnsZeroForMissingType YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryObjectReturnsMatchingAttachmentsInSlotOrder YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryObjectReturnsZeroForMissingObject YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryRejectsNullSourceWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryRejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryRejectsNullOutputWhenCapacityNonZero YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryRejectsOutputOverflowWithoutOverrun YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryIsReadOnlyForAttachmentStorage YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_QueryPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldComponentQueryBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_NoActorComponentBehaviorOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentQueryBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentQueryBridge_NoFilePackageThreadPlatformDiagnosticsDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentQueryBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentQueryBridge_WorldInstanceCoreRemainsQueryFree YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteReadRoundTripsAttachmentsInSlotOrder YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteEmptyBridgeProducesZeroRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteRejectsNullSourceWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteRejectsNullWriterWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRestoresAttachmentRecords YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadZeroRecordStreamClearsDestination YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsNullReaderWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsNullDestinationWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_ReadRejectsDuplicateAttachmentWithoutMutation YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WriteReadPathDoesNotGrowStorage YuWorldTests Fast ModuleFixture World PerformanceSmoke)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_SnapshotReportsCountsAndLastStatus YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_NoActorComponentPayloadOrLifecycle YuWorldTests Fast ModuleFixture World)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_NoObjectResourceScriptFilePackageOrGameAdapterDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency YuWorldTests Fast ModuleFixture World EvidenceOracle)
yuengine_add_labeled_test(WorldComponentAttachmentSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree YuWorldTests Fast ModuleFixture World)

add_executable(YuWorldIdentityBaselineTests
    Tests/World/WorldIdentityBaselineTests.cpp
)
yuengine_apply_compile_options(YuWorldIdentityBaselineTests)
target_link_libraries(YuWorldIdentityBaselineTests PRIVATE YuWorld)
yuengine_add_labeled_test(WorldIdentityBaseline_CreateQueryExportDestroyDeterministic YuWorldIdentityBaselineTests Fast ModuleFixture World Object L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldIdentityBaseline_InvalidComponentDoesNotMutate YuWorldIdentityBaselineTests Fast ModuleFixture World Object L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldIdentityBaseline_DuplicateWorldObjectDoesNotMutate YuWorldIdentityBaselineTests Fast ModuleFixture World Object L1Runtime EvidenceOracle)
yuengine_add_labeled_test(WorldIdentityBaseline_DestroyMissingDoesNotMutate YuWorldIdentityBaselineTests Fast ModuleFixture World Object L1Runtime EvidenceOracle)

add_executable(YuThreadTests
    Tests/Thread/ThreadTestContext.cpp
    Tests/Thread/ThreadTests.cpp
)
yuengine_apply_compile_options(YuThreadTests)
target_link_libraries(YuThreadTests PRIVATE YuThread)
yuengine_add_labeled_test(Thread_QueueEnqueueWithinCapacity_Succeeds YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_QueueEnqueueBeyondCapacity_Rejects YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_DrainExecutesTasks_InDeterministicOrder YuThreadTests Fast ModuleFixture Thread EvidenceOracle)
yuengine_add_labeled_test(Thread_TaskFailure_ReturnsFailedResult YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_ShutdownRejectsNewSubmission YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_ShutdownDrainPolicy_ExecutesQueuedTasks YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_ShutdownCancelPolicy_CancelsQueuedTasks YuThreadTests Fast ModuleFixture Thread)
yuengine_add_labeled_test(Thread_QueueCapacity_DoesNotGrowDuringFixture YuThreadTests Fast ModuleFixture Thread PerformanceSmoke)
yuengine_add_labeled_test(Thread_DiagnosticsDisabled_DoesNotChangeBehavior YuThreadTests Fast ModuleFixture Thread EvidenceOracle)
yuengine_add_labeled_test(Thread_WorkerSubmitBeforeStart_ReturnsExplicitStatus YuThreadTests Fast ModuleFixture Thread EvidenceOracle)
yuengine_add_labeled_test(Thread_WorkerDrainShutdown_WritesCompletionRecords YuThreadTests Fast ModuleFixture Thread AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Thread_WorkerCancelShutdown_CancelsQueuedWork YuThreadTests Fast ModuleFixture Thread AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Thread_WorkerCompletionCapacity_RejectsWithoutMutation YuThreadTests Fast ModuleFixture Thread AsyncIO PerformanceSmoke)
yuengine_add_labeled_test(Thread_WorkerCompletionDrain_UsesCallerStorageLimit YuThreadTests Fast ModuleFixture Thread AsyncIO PerformanceSmoke EvidenceOracle)

add_executable(YuFileTests
    Tests/File/FileTests.cpp
)
yuengine_apply_compile_options(YuFileTests)
target_link_libraries(YuFileTests PRIVATE YuFile)
target_compile_definitions(YuFileTests PRIVATE YUENGINE_FILE_FIXTURE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures/File")
yuengine_add_labeled_test(File_PathNormalize_RemovesDotAndRepeatedSeparators YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_PathNormalize_RejectsTraversalOutsideRoot YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_PathNormalize_RejectsEmptyAndAbsolutePath YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_MountTable_RejectsDuplicateMount YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_MountTable_UsesDeterministicPriorityOrder YuFileTests Fast ModuleFixture File EvidenceOracle)
yuengine_add_labeled_test(File_MountTable_ReportsMissingMountOrFile YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_LooseFixtureRead_ReturnsExactBytes YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_LooseFileSourceRejectsForgedNormalizedPathEscape YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_LooseFixtureWrite_RoundTripsExactBytes YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_LooseFixtureWrite_RejectsForgedPathAndOversizedBuffer YuFileTests Fast ModuleFixture File EvidenceOracle)
yuengine_add_labeled_test(File_MountTableWrite_RecordsSnapshotAndMissingMount YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_ReadSnapshot_RecordsCountsAndBytes YuFileTests Fast ModuleFixture File)
yuengine_add_labeled_test(File_DiagnosticsDisabled_DoesNotChangeBehavior YuFileTests Fast ModuleFixture File EvidenceOracle)
yuengine_add_labeled_test(File_AsyncReadQueue_ReadsFixtureIntoCallerStorage YuFileTests Fast ModuleFixture File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(File_AsyncReadQueue_RejectsCapacityOverflow YuFileTests Fast ModuleFixture File AsyncIO PerformanceSmoke)
yuengine_add_labeled_test(File_AsyncReadQueue_ReportsReadFailureCompletion YuFileTests Fast ModuleFixture File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(File_AsyncReadQueue_RejectsSmallOutputWithoutOverrun YuFileTests Fast ModuleFixture File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(File_AsyncReadQueue_ShutdownRejectsSubmission YuFileTests Fast ModuleFixture File AsyncIO)
yuengine_add_labeled_test(File_AsyncReadQueue_SnapshotReportsBoundedCounters YuFileTests Fast ModuleFixture File AsyncIO PerformanceSmoke EvidenceOracle)

add_executable(YuRHITests
    Tests/Rhi/RhiTests.cpp
)
yuengine_apply_compile_options(YuRHITests)
target_link_libraries(YuRHITests PRIVATE YuRHI)
yuengine_add_labeled_test(RHI_CreateNullDevice_ReturnsCapabilities YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CreateDevice_RejectsUnsupportedBackend YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_Interface_CreateNullDevice_ReturnsCapabilities YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_Interface_ClearSubmitPresentCapture_MatchesNullDevice YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_CreateNullDevice_UsesCallerOwnedStorage YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_NullStorageRejectedWithoutOutput YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_D3D11BackendUnsupportedWithoutMutation YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_SurfaceRequiredForNullBackendRejectedWithoutMutation YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_InvalidNativeSurfaceRejectedBeforeMutation YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_NativeSurfaceDesc_DefaultIsInvalidPlainValue YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_RawStorageCreatesNullDevice YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_RawStorageRejectsSmallBuffer YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Factory_D3D11InvalidSurfaceFailsBeforeHardware YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_SwapchainDesc_DefaultIsBoundedPlainValue YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_NullBackend_SwapchainQueryReturnsUnsupported YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SwapchainResize_DefaultContractsAreExplicit YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_NullBackend_SwapchainResizeReturnsUnsupportedWithoutTarget YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_CreateTarget_ReturnsGenerationHandle YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CreateColorTarget_ReturnsGenerationHandle YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CreateColorTarget_RejectsInvalidDescriptor YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_TargetCapacityOverflow_DoesNotMutate YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_DestroyTarget_InvalidatesStaleHandle YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_Reinitialize_InvalidatesPriorTargetHandle YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CommandList_RecordsClearWithinCapacity YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CommandListCapacityOverflow_DoesNotMutate YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SubmitRejectsOversizedCommandListWithoutMutation YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_RecordClear_RejectsInvalidTargetHandle YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SubmitRejectsIncompleteCommandListWithoutMutation YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SubmitRejectsMismatchedRecordedTargetWithoutMutation YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SubmitRejectsStaleRecordedTargetWithoutMutation YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_SubmitExecutesClearIntoNullTarget YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_PresentRequiresSuccessfulSubmit YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_PresentRejectsDestroyedSubmittedTargetWithoutMutation YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_ClearSubmitPresent_UpdatesPresentedCounter YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CaptureBeforePresent_ReturnsExplicitStatus YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_ClearColor_UsesExactRgba8ByteChannels YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_CapturePresentedTarget_WritesDeterministicRgba8Bytes YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_CaptureRejectsDestroyedPresentedTargetWithoutMutation YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_CaptureRejectsUndersizedBufferWithoutWritingBytes YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_CaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_FrameSubmitPresentCapture_DoesNotGrowCommandStorage YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_DisabledDiagnosticsDoesNotChangeResults YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_PrimitiveCapabilities_ReportBoundedCapacities YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_CreateBuffer_ReturnsGenerationHandleAndSnapshot YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_UpdateBuffer_SignalsFenceAndRecordsBytes YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_BufferCapacityOverflow_DoesNotMutate YuRHITests Fast ModuleFixture RHI PerformanceSmoke)
yuengine_add_labeled_test(RHI_TextureCreateUpdateDestroy_TracksSnapshot YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_SamplerCreateDestroy_TracksSnapshot YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_ShaderModuleRejectsEmptyBytecode YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_PipelineRequiresValidShaderModules YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_PipelineCreateDestroy_UsesShaderModuleHandles YuRHITests Fast ModuleFixture RHI)
yuengine_add_labeled_test(RHI_PrimitiveDestroyInvalidatesStaleHandles YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Interface_PrimitiveLifecycle_MatchesNullDevice YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveSnapshot_IsIncludedInDeviceSnapshotComparison YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_DefaultContractsAreExplicit YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_RequestCreatesPendingRecord YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_DrainInvalidatesHandleAndTracksCounters YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_RejectsInvalidWrongDuplicateAndCapacity YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_FenceNotReadyDrainKeepsPending YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_PrimitiveRetirement_ImmediateDestroyCompatibility YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_Interface_PrimitiveRetirementMatchesNullDevice YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_CommandList_RecordsVisibleTriangleCommandsWithinCapacity YuRHITests Fast ModuleFixture RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitDrawRejectsMissingPipelineWithoutMutation YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitDrawRejectsVertexBufferRangeOverflow YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitDraw_UpdatesNullSnapshot YuRHITests Fast ModuleFixture RHI EvidenceOracle)
yuengine_add_labeled_test(RHI_IndexFormat_DefaultDescriptorsAreUnsupported YuRHITests Fast ModuleFixture RHI StaticMesh IndexedDraw EvidenceOracle)
yuengine_add_labeled_test(RHI_CommandList_RecordsIndexedStaticMeshWithinCapacity YuRHITests Fast ModuleFixture RHI StaticMesh IndexedDraw PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitIndexedDrawRejectsMissingIndexBufferWithoutMutation YuRHITests Fast ModuleFixture RHI StaticMesh IndexedDraw EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitIndexedDrawRejectsIndexRangeOverflow YuRHITests Fast ModuleFixture RHI StaticMesh IndexedDraw EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitIndexedDraw_UpdatesNullSnapshot YuRHITests Fast ModuleFixture RHI StaticMesh IndexedDraw EvidenceOracle)
yuengine_add_labeled_test(RHI_TextureSampling_DefaultBindingsAreBoundedValues YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_CommandList_RecordsTextureSamplingWithinCapacity YuRHITests Fast ModuleFixture RHI TextureSampling Sampler PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RHI_TextureSamplingSlotOverflow_DoesNotMutate YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitTextureSamplingRejectsMissingSamplerWithoutMutation YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitSamplerRejectsMissingTextureWithoutMutation YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitTextureSamplingRejectsStaleTextureHandle YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitTextureSamplingRejectsStaleSamplerHandle YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_SubmitTextureSampling_UpdatesNullSnapshot YuRHITests Fast ModuleFixture RHI TextureSampling Sampler EvidenceOracle)
yuengine_add_labeled_test(RHI_NoResourceFileUploadShaderUiDependency YuRHITests Fast ModuleFixture RHI EvidenceOracle)

add_executable(YuRenderCoreTests
    Tests/RenderCore/RenderCoreTests.cpp
)
yuengine_apply_compile_options(YuRenderCoreTests)
target_link_libraries(YuRenderCoreTests PRIVATE YuRenderCore YuRHI)
yuengine_add_labeled_test(RenderCore_FixturePass_ExecutesSampledIndexedPass YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsNullRhiDeviceWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsInvalidTargetWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsInvalidPipelineWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsMissingVertexBufferWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsMissingIndexBufferWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsInvalidTextureBindingWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsInvalidSamplerBindingWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsInvalidDrawWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsSmallCaptureStorageWithoutMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsCommandCapacityWithoutRhiMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_RejectsPassRecordCapacityWithoutRhiMutation YuRenderCoreTests Fast ModuleFixture RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_TracksRhiFailureWithoutWritingCapture YuRenderCoreTests Fast ModuleFixture RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FixturePass_SnapshotTracksBoundedCounters YuRenderCoreTests Fast ModuleFixture RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_BindsValuesToRenderFixtureRequest YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsInvalidPipelineWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsInvalidTextureBindingWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsInvalidSamplerBindingWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsOversizedConstantsWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsDuplicateMaterialIdWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_RejectsCapacityOverflowWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_PropagatesRenderFixturePassFailure YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(Material_BindingFixture_SnapshotTracksBoundedCounters YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
add_executable(YuRenderMaterialTests
    Tests/RenderCore/RenderMaterialTests.cpp
)
yuengine_apply_compile_options(YuRenderMaterialTests)
target_link_libraries(YuRenderMaterialTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_MaterialBuildsBindingRequest YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialAcceptsCameraShaderConstants YuRenderMaterialTests Fast ModuleFixture Camera Shader Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialRejectsInvalidProgramIdWithoutOutputMutation YuRenderMaterialTests Fast ModuleFixture Shader Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialRejectsOversizedConstantsWithoutOutputMutation YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialRejectsDuplicateMaterialId YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialRejectsCapacityExceeded YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialSnapshotTracksCounters YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_MaterialHasNarrowDependencyBoundary YuRenderMaterialTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
add_executable(YuRenderDrawPacketTests
    Tests/RenderCore/RenderDrawPacketTests.cpp
)
yuengine_apply_compile_options(YuRenderDrawPacketTests)
target_link_libraries(YuRenderDrawPacketTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_DrawPacketBuildsPassGeometry YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketRejectsMissingVertexWithoutOutputMutation YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketRejectsMissingIndexWithoutOutputMutation YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketRejectsInvalidDrawRangeWithoutOutputMutation YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketRejectsDuplicateDrawId YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketRejectsCapacityExceeded YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketSnapshotTracksCounters YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawPacketHasNarrowDependencyBoundary YuRenderDrawPacketTests Fast ModuleFixture Draw Material RenderCore RHI EvidenceOracle)
add_executable(YuRenderViewPacketTests
    Tests/RenderCore/RenderViewPacketTests.cpp
)
yuengine_apply_compile_options(YuRenderViewPacketTests)
target_link_libraries(YuRenderViewPacketTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_ViewPacketBuildsCameraMaterialDrawPass YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketRejectsPassMismatchWithoutOutputMutation YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketRejectsMaterialFailureWithoutOutputMutation YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketRejectsDrawFailureWithoutOutputMutation YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketRejectsDuplicateViewId YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketRejectsCapacityExceeded YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketSnapshotTracksCounters YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ViewPacketHasNarrowDependencyBoundary YuRenderViewPacketTests Fast ModuleFixture View Camera Shader Material Draw RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_ExecutesMaterialPreparedRequests YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_RejectsEmptyBatchWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_RejectsNullPassWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_RejectsInvalidPassRequestWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_RejectsDuplicatePassIdWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_RejectsBatchCapacityWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_PropagatesRenderFixturePassFailure YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SubmissionBatch_SnapshotTracksBoundedCounters YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_ExecutesPreparedSubmissionBatch YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsZeroFrameIdWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsNullExecutorWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsNullBatchRequestWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsInvalidBatchRequestWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsDuplicateFrameIdWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_RejectsPacketCapacityWithoutMutation YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_PropagatesSubmissionBatchFailure YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_FramePacket_SnapshotTracksBoundedCounters YuRenderCoreTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)

add_executable(YuRenderCameraTests
    Tests/RenderCore/RenderCameraTests.cpp
)
yuengine_apply_compile_options(YuRenderCameraTests)
target_link_libraries(YuRenderCameraTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_CameraBuildsPerspectiveFrame YuRenderCameraTests Fast ModuleFixture Camera RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraBuildsOrthographicFrame YuRenderCameraTests Fast ModuleFixture Camera RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraRejectsInvalidProjectionWithoutOutputMutation YuRenderCameraTests Fast ModuleFixture Camera RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraRejectsDegeneratePoseWithoutOutputMutation YuRenderCameraTests Fast ModuleFixture Camera RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraSnapshotTracksCounters YuRenderCameraTests Fast ModuleFixture Camera RenderCore PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraHasNarrowDependencyBoundary YuRenderCameraTests Fast ModuleFixture Camera RenderCore EvidenceOracle)

add_executable(YuRenderCameraShaderConstantsTests
    Tests/RenderCore/RenderCameraShaderConstantsTests.cpp
)
yuengine_apply_compile_options(YuRenderCameraShaderConstantsTests)
target_link_libraries(YuRenderCameraShaderConstantsTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_CameraShaderConstantsWritesViewProjection YuRenderCameraShaderConstantsTests Fast ModuleFixture Camera Shader Material RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraShaderConstantsRejectsNullOutput YuRenderCameraShaderConstantsTests Fast ModuleFixture Camera Shader Material RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraShaderConstantsRejectsInvalidFrameWithoutOutputMutation YuRenderCameraShaderConstantsTests Fast ModuleFixture Camera Shader Material RenderCore EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraShaderConstantsSnapshotTracksCounters YuRenderCameraShaderConstantsTests Fast ModuleFixture Camera Shader Material RenderCore PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_CameraShaderConstantsHasNarrowDependencyBoundary YuRenderCameraShaderConstantsTests Fast ModuleFixture Camera Shader Material RenderCore EvidenceOracle)

add_executable(YuRenderShaderProgramTests
    Tests/RenderCore/RenderShaderProgramTests.cpp
)
yuengine_apply_compile_options(YuRenderShaderProgramTests)
target_link_libraries(YuRenderShaderProgramTests PRIVATE YuRenderCore)
yuengine_add_labeled_test(RenderCore_ShaderProgramBuildsPipelineDesc YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramRejectsInvalidVertexShaderWithoutOutputMutation YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramRejectsInvalidInputLayoutWithoutOutputMutation YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramRejectsDuplicateProgramId YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramRejectsCapacityExceeded YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramSnapshotTracksCounters YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_ShaderProgramHasNarrowDependencyBoundary YuRenderShaderProgramTests Fast ModuleFixture Shader RenderCore RHI EvidenceOracle)

add_executable(YuRenderDrawableFramePipelineTests
    Tests/RenderCore/RenderDrawableFramePipelineTests.cpp
)
yuengine_apply_compile_options(YuRenderDrawableFramePipelineTests)
target_link_libraries(YuRenderDrawableFramePipelineTests PRIVATE YuRenderCore YuRHI)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_ExecutesMaterialFrameDrawCapture YuRenderDrawableFramePipelineTests Fast ModuleFixture Material RenderCore RHI Swapchain FramePipeline PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_RejectsInvalidSwapchainWithoutMutation YuRenderDrawableFramePipelineTests Fast ModuleFixture Material RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_RejectsInvalidMaterialWithoutRhiMutation YuRenderDrawableFramePipelineTests Fast ModuleFixture Material RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_RejectsInvalidDrawThroughViewPacket YuRenderDrawableFramePipelineTests Fast ModuleFixture View Material Draw RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_RejectsDuplicateFrameThroughFramePacket YuRenderDrawableFramePipelineTests Fast ModuleFixture Material RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_DrawableFramePipeline_PropagatesFixtureCommandCapacityFailure YuRenderDrawableFramePipelineTests Fast ModuleFixture Material RenderCore RHI Swapchain FramePipeline PerformanceSmoke EvidenceOracle)

add_executable(YuRenderSwapchainFramePipelineTests
    Tests/RenderCore/RenderSwapchainFramePipelineTests.cpp
)
yuengine_apply_compile_options(YuRenderSwapchainFramePipelineTests)
target_link_libraries(YuRenderSwapchainFramePipelineTests PRIVATE YuRenderCore YuRHI)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_ExecutesClearSubmitPresentCapture YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain FramePipeline PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_ResizesBeforeSubmit YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain Resize FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_RejectsDuplicateFrameIdWithoutMutation YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_RejectsInvalidSwapchainWithoutMutation YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_TracksRhiFailureWithoutCapture YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain FramePipeline EvidenceOracle)
yuengine_add_labeled_test(RenderCore_SwapchainFramePipeline_RejectsCommandCapacityWithoutRhiMutation YuRenderSwapchainFramePipelineTests Fast ModuleFixture RenderCore RHI Swapchain FramePipeline PerformanceSmoke EvidenceOracle)

add_executable(YuRenderGraphSkeletonTests
    Tests/RenderCore/RenderGraphSkeletonTests.cpp
)
yuengine_apply_compile_options(YuRenderGraphSkeletonTests)
target_link_libraries(YuRenderGraphSkeletonTests PRIVATE YuRenderCore YuRHI)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_PreparesSubmissionBatchInDeclarationOrder YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_QueryReturnsGraphRecord YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_ReleaseClearsGraphRecord YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsZeroGraphIdWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsEmptyGraphWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsDuplicateGraphIdWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsDuplicatePassIdWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsMissingDependencyWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsSelfDependencyWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsDependencyCycleWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsPassCapacityOverflowWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsDependencyCapacityOverflowWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_RejectsInvalidPassRequestWithoutMutation YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphSkeleton_SnapshotTracksBoundedCounters YuRenderGraphSkeletonTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)

add_executable(YuRenderGraphExecutionPlanTests
    Tests/RenderCore/RenderGraphExecutionPlanTests.cpp
)
yuengine_apply_compile_options(YuRenderGraphExecutionPlanTests)
target_link_libraries(YuRenderGraphExecutionPlanTests PRIVATE YuRenderCore YuRHI)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_ExecutesPreparedSkeletonBatch YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsFailedSkeletonResultWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsNullFrameExecutorWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsInvalidFrameIdWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsInvalidPreparedBatchWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsDuplicatePlanIdWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsDuplicateGraphExecutionWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_RejectsPlanCapacityOverflowWithoutMutation YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_PropagatesFramePacketFailure YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_QueryReturnsPlanRecord YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_ReleaseClearsPlanRecord YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_SnapshotTracksBoundedCounters YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderCore_GraphExecutionPlan_ResetClearsPlanRecords YuRenderGraphExecutionPlanTests Fast ModuleFixture Material RenderCore RHI PerformanceSmoke EvidenceOracle)

add_executable(YuRhiD3D11HardwareSmokeTests
    Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuRhiD3D11HardwareSmokeTests)
target_link_libraries(YuRhiD3D11HardwareSmokeTests PRIVATE YuRHI YuPlatform)
yuengine_add_labeled_test(RHI_D3D11Hardware_ClearPresentCaptureBytes YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32)
set_tests_properties(RHI_D3D11Hardware_ClearPresentCaptureBytes PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_PrimitiveResourcePipelineSnapshot YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32)
set_tests_properties(RHI_D3D11Hardware_PrimitiveResourcePipelineSnapshot PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_VisibleTriangleCaptureBytes YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32)
set_tests_properties(RHI_D3D11Hardware_VisibleTriangleCaptureBytes PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_IndexedStaticMeshCaptureBytes YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32 StaticMesh IndexedDraw)
set_tests_properties(RHI_D3D11Hardware_IndexedStaticMeshCaptureBytes PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_TextureSamplingCaptureBytes YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32 TextureSampling Sampler)
set_tests_properties(RHI_D3D11Hardware_TextureSamplingCaptureBytes PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_SwapchainResizeInvalidatesOldBackbufferGeneration YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32 Swapchain Resize EvidenceOracle)
set_tests_properties(RHI_D3D11Hardware_SwapchainResizeInvalidatesOldBackbufferGeneration PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_SwapchainResizeSameExtentNoOpKeepsGeneration YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32 Swapchain Resize)
set_tests_properties(RHI_D3D11Hardware_SwapchainResizeSameExtentNoOpKeepsGeneration PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(RHI_D3D11Hardware_SwapchainResizeRejectsInvalidExtent YuRhiD3D11HardwareSmokeTests HardwareSmoke D3D11 RHI Win32 Swapchain Resize EvidenceOracle)
set_tests_properties(RHI_D3D11Hardware_SwapchainResizeRejectsInvalidExtent PROPERTIES SKIP_RETURN_CODE 77)

add_executable(YuRenderFrameD3D11Smoke
    Tests/RenderCore/RenderSwapchainFramePipelineD3D11HardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuRenderFrameD3D11Smoke)
target_link_libraries(YuRenderFrameD3D11Smoke PRIVATE YuRenderCore YuRHI YuPlatform)
yuengine_add_labeled_test(RenderCore_D3D11Hardware_SwapchainFramePipelineResizeClearCapture YuRenderFrameD3D11Smoke HardwareSmoke D3D11 RenderCore RHI Win32 Swapchain Resize FramePipeline EvidenceOracle)
set_tests_properties(RenderCore_D3D11Hardware_SwapchainFramePipelineResizeClearCapture PROPERTIES SKIP_RETURN_CODE 77)

add_executable(YuRenderDrawableFrameD3D11Smoke
    Tests/RenderCore/RenderDrawableFramePipelineD3D11HardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuRenderDrawableFrameD3D11Smoke)
target_link_libraries(YuRenderDrawableFrameD3D11Smoke PRIVATE YuRenderCore YuRHI YuPlatform)
yuengine_add_labeled_test(RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture YuRenderDrawableFrameD3D11Smoke HardwareSmoke D3D11 Material RenderCore RHI Win32 Swapchain FramePipeline TextureSampling Sampler EvidenceOracle)
set_tests_properties(RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture PROPERTIES SKIP_RETURN_CODE 77)

add_executable(YuAudioTests
    Tests/Audio/AudioTests.cpp
)
yuengine_apply_compile_options(YuAudioTests)
target_include_directories(YuAudioTests
    PRIVATE
        Src/YuEngine/Audio/Src
)
target_link_libraries(YuAudioTests PRIVATE YuAudio)
yuengine_add_labeled_test(Audio_CreateTestDevice_ReturnsCapabilities YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CreateDevice_RejectsUnsupportedBackend YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CreateDevice_RejectsUnsupportedFormat YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_RegisterSyntheticSource_ReturnsStableId YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_SourceCapacityOverflow_DoesNotMutate YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_StartVoice_ReturnsGenerationHandle YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_StartVoice_RejectsMissingSource YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_StartVoice_RejectsInvalidGainWithoutMutation YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_VoiceCapacityOverflow_DoesNotMutate YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_StopVoice_InvalidatesStaleHandle YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_Reinitialize_InvalidatesPriorSourceAndVoiceHandles YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixSingleVoice_WritesDeterministicS16StereoSamples YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_MixUnityGain_PreservesS16EdgeSamples YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixFractionalGain_RoundsTowardZeroDeterministically YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_MixMultipleVoices_UsesStableOrderAndSaturates YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixMaxVoicesFullScaleSaturatesWithoutOverflow YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixStopsVoiceAtEndOfSource YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixEndedVoice_WritesSilentTail YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixOverwritesPrefilledDestination YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_MixRejectsUndersizedBufferWithoutWritingSamples YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_UninitializedDeviceOperationsReturnExplicitStatusWithoutMutation YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_Mix_DoesNotGrowVoiceStorage YuAudioTests Fast ModuleFixture Audio PerformanceSmoke)
yuengine_add_labeled_test(Audio_DisabledDiagnosticsDoesNotChangeResults YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_NoDeviceCodecResourceScriptUiGameAdapterDependency YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackDesc_DefaultValuesAreBounded YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackSnapshot_DefaultValuesAreExplicit YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackCompletion_DefaultValuesAreExplicit YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackPublicContract_UsesExplicitOwnerAndValues YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackDevice_RejectsUnsupportedBackendBeforeHardware YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackDevice_RejectsUnsupportedFormatBeforeHardware YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackDevice_RejectsInvalidBufferShapeBeforeHardware YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackDevice_UninitializedOperationsReturnExplicitStatus YuAudioTests Fast ModuleFixture Audio)
yuengine_add_labeled_test(Audio_CallbackDevice_ControlledBackendSubmitsAndDrains YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackDevice_ControlledBackendSplitsUnavailableAndBackendError YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackDevice_ControlledBackendSubmitErrorRollsBack YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_CallbackDevice_ControlledBackendCallbackErrorReportsFailure YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_CreatesQueriesAndReleasesMetadata YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsDuplicatePacketIdWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsUnsupportedFormatWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsUnsupportedSampleRateWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsUnsupportedChannelCountWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsZeroFrameCountWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsSampleCountMismatchWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsByteCountMismatchWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsCapacityOverflowWithoutMutation YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectsStaleHandleWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_SnapshotTracksCounters YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_RejectionsDoNotMutateSourceVoiceState YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmSamplePacket_PublicContractsArePlainValues YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_QueuesDrainsAndReleasesMetadata YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsInvalidPacketHandleWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsReleasedPacketHandleWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsPacketIdMismatchWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsDuplicateQueueIdWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsFrameRangeOverflowWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsSampleCountMismatchWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsByteCountMismatchWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsInvalidChunkFrameCountWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsCapacityOverflowWithoutMutation YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_DrainRejectsSmallOutputWithoutMutation YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectsStaleHandleWithoutMutation YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_RejectionsDoNotMutatePacketSourceVoiceCallbackState YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_SnapshotTracksCounters YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueue_PublicContractsArePlainValues YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_SubmitsDrainedChunks YuAudioTests Fast ModuleFixture Audio PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_RejectsPartialFinalChunkWithoutDrain YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_RejectsSmallSampleSpanWithoutDrain YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_StopsOnSubmitFailure YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_RejectsUnstartedCallbackWithoutDrain YuAudioTests Fast ModuleFixture Audio EvidenceOracle)
yuengine_add_labeled_test(Audio_PcmStreamQueueCallback_PublicContractsArePlainValues YuAudioTests Fast ModuleFixture Audio EvidenceOracle)

add_executable(YuAudioHardwareSmokeTests
    Tests/Audio/AudioHardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuAudioHardwareSmokeTests)
target_link_libraries(YuAudioHardwareSmokeTests PRIVATE YuAudio)
yuengine_add_labeled_test(Audio_HardwareCallback_CompletesSubmittedBuffer YuAudioHardwareSmokeTests HardwareSmoke Audio Win32 XAudio2)
set_tests_properties(Audio_HardwareCallback_CompletesSubmittedBuffer PROPERTIES SKIP_RETURN_CODE 77)
yuengine_add_labeled_test(Audio_HardwareCallback_SubmitsPcmStreamQueue YuAudioHardwareSmokeTests HardwareSmoke Audio Win32 XAudio2 StreamQueue EvidenceOracle)
set_tests_properties(Audio_HardwareCallback_SubmitsPcmStreamQueue PROPERTIES SKIP_RETURN_CODE 77)

add_executable(YuAudioResourceTests
    Tests/AudioResource/AudioResourceTests.cpp
)
yuengine_apply_compile_options(YuAudioResourceTests)
target_link_libraries(YuAudioResourceTests PRIVATE YuAudioResource)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_MapsResourceDecodeResult YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_RejectsNonAudioDecodeResult YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_RejectsDecodedByteMismatch YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_RejectsInvalidPacketShape YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_RejectsDuplicateImportAndPacketIds YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_RejectsCapacityOverflow YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_QueryAndReleaseInvalidateHandle YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(AudioResource_PcmPacketImportBridge_PublicContractsArePlainValues YuAudioResourceTests Fast ModuleFixture AudioResource Audio Resource EvidenceOracle)

add_executable(YuResourceTests
    Tests/Resource/ResourceTests.cpp
)
yuengine_apply_compile_options(YuResourceTests)
target_link_libraries(YuResourceTests PRIVATE YuResource)
yuengine_add_labeled_test(Resource_RegisterSyntheticDescriptor_ReturnsGenerationHandle YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_RegisterDuplicate_ReturnsExplicitStatus YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_RegistryRejectsCapacityOverflowWithoutMutation YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_TypeCapacityOverflow_DoesNotMutate YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_HandleRejectsWrongGeneration YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_HandleRejectsTypeMismatch YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_AcquireRelease_TracksReferenceCount YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_RepeatedAcquire_IncrementsReferenceCount YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_AcquireRejectsReferenceCountOverflow YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_ValidateAcquireChecksHandleTypeAndOverflowWithoutMutation YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_ValidateAcquireDoesNotRequireWorld YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_RetireRejectsOutstandingAcquire YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_RetireRejectsLiveDependentEdge YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_DependencyValidationRejectsMissingDependency YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_DependencyValidationRejectsCycle YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_NoFileOrPackageDependency_ForHandleRegistry YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DisabledDiagnosticsDoesNotChangeResults YuResourceTests Fast ModuleFixture Resource)
yuengine_add_labeled_test(Resource_NoHiddenAllocation_UsesYuMemorySignal YuResourceTests Fast ModuleFixture Resource PerformanceSmoke)
yuengine_add_labeled_test(Resource_LoadCommit_UploadSuccessSetsTerminalState YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_FailedUploadSetsFailedState YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_RejectsInvalidHandleWithoutSlotMutation YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_RejectsDuplicateCommitId YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_RejectsInvalidTransition YuResourceTests Fast ModuleFixture Resource Upload EvidenceOracle)
yuengine_add_labeled_test(Resource_LoadCommit_SnapshotTracksCounters YuResourceTests Fast ModuleFixture Resource Upload PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_AdmitsUploadedSlotWithinBudget YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_RejectsUnloadedWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_RejectsFailedLoadWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_RejectsDuplicateAdmission YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_RejectsBudgetOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_PinUnpinTracksCounters YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_SelectsEvictionCandidateInSlotOrder YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_ReportsNoEvictionCandidate YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_FailedValidationDoesNotMutateResourceState YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_Residency_EvictsResourceOwnedStateOnly YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_StoresAndReadsResidentBytes YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_ReleaseClearsPayloadOnly YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsNotResidentWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsFailedLoadWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsStaleHandleWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsDuplicatePayloadId YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsCapacityOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_RejectsBudgetOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_ReadRejectsOutputBufferTooSmall YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_FailedValidationDoesNotMutateResourceState YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_CachePayload_ReleaseRejectsPinnedWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_CreatesQueriesAndReleasesMetadata YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsMissingCachePayload YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsInvalidHeader YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsUnsupportedHeaderVersion YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsStaleHandleWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsNotResidentWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsFailedLoadWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsDuplicatePlanId YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsCapacityOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_RejectsBudgetOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodePlan_FailedValidationDoesNotMutateResourceState YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_CommitsQueriesAndReleasesMetadata YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsMissingDecodePlan YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsStaleHandleWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsNotResidentWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsFailedLoadWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsDuplicateResultId YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsCapacityOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsBudgetOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsAssetClassMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsResultClassMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_RejectsDecodedByteCountMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_ReleasingPlanClearsDependentRecords YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_ReleasingPayloadClearsDependentRecords YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodeResult_FailedValidationDoesNotMutateResourceState YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_StoresReadsQueriesAndReleasesBytes YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsMissingCachePayload YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsMissingDecodePlan YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsMissingDecodeResult YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsNullInputBytes YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsEmptyPayload YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_ReadRejectsOutputBufferTooSmall YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsDuplicatePayloadId YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsCapacityOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsBudgetOverflow YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsAssetClassMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsResultClassMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsDecodedByteCountMismatch YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsTypeMismatchWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsNotResidentWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_RejectsFailedLoadWithoutMutation YuResourceTests Fast ModuleFixture Resource EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_ReleasingResultClearsPayload YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_ReleasingPlanClearsPayload YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_ReleasingCachePayloadClearsPayload YuResourceTests Fast ModuleFixture Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Resource_DecodedPayload_FailedValidationDoesNotMutateResourceState YuResourceTests Fast ModuleFixture Resource EvidenceOracle)

add_executable(YuObjectTests
    Tests/Object/ObjectTests.cpp
)
yuengine_apply_compile_options(YuObjectTests)
target_link_libraries(YuObjectTests PRIVATE YuObject)
yuengine_add_labeled_test(Object_CreateSyntheticObject_ReturnsGenerationHandle YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_CreateRejectsInvalidTypeWithoutMutation YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_RegistryCapacityOverflow_DoesNotMutate YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_TypeCapacityOverflow_DoesNotMutate YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_ValidateRejectsInvalidOrStaleHandle YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_InvalidOrStaleHandleOperations_ReturnExplicitStatusWithoutMutation YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_DestroyIncrementsGenerationAndInvalidatesOldHandle YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_ReusesFreedSlotWithNewGeneration YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_AcquireRelease_TracksReferenceCount YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_RepeatedAcquire_IncrementsReferenceCount YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_AcquireRejectsReferenceCountOverflow YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_ReleaseAtZero_DoesNotMutate YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_DestroyRejectsOutstandingReference YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_RegistrySnapshot_ReportsCountsAndLastStatus YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_DisabledDiagnosticsDoesNotChangeResults YuObjectTests Fast ModuleFixture Object)
yuengine_add_labeled_test(Object_NoWorldScriptResourceOrGameAdapterDependency YuObjectTests Fast ModuleFixture Object EvidenceOracle)
yuengine_add_labeled_test(Object_NoHiddenAllocation_UsesYuMemorySignal YuObjectTests Fast ModuleFixture Object PerformanceSmoke)
yuengine_add_labeled_test(Object_HotPathValidateSmoke_DoesNotGrowRuntimeCounts YuObjectTests Fast ModuleFixture Object L1Runtime PerformanceSmoke)

add_executable(YuUiCoreTests
    Tests/UiCore/UiCoreTests.cpp
)
yuengine_apply_compile_options(YuUiCoreTests)
target_link_libraries(YuUiCoreTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_NodeTree_CreateAttachDetachOrder YuUiCoreTests Fast ModuleFixture UiCore UI EvidenceOracle)
yuengine_add_labeled_test(UiCore_NodeTree_DestroyRemovesDescendants YuUiCoreTests Fast ModuleFixture UiCore UI EvidenceOracle)
yuengine_add_labeled_test(UiCore_RectMath_ParentResizePivotMarginPaddingDpi YuUiCoreTests Fast ModuleFixture UiCore UI EvidenceOracle)
yuengine_add_labeled_test(UiCore_NoLifecycleConfigEditorRenderBackendDependency YuUiCoreTests Fast ModuleFixture UiCore UI EvidenceOracle)
yuengine_add_labeled_test(UiCore_LayoutContainers_ResolveExpectedRects YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DirtyTracker_PaintOnlyDoesNotTriggerLayoutRebuild YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_InvalidationModel_SubtreeRulesExposeCacheCounters YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_InvalidationModel_RejectsSmallOutputWithoutMutation YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_HitTest_LayerClipDisabled YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DrawList_DeterministicElements YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_StaticAtlasMetadata_ResolvesSpritePageUvNineSlice YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_StaticAtlasMetadata_MissingSpriteReportsStatus YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_StaticAtlasMetadata_RejectsInvalidSpriteMetadata YuUiCoreTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiDrawBatcherTests
    Tests/UiCore/UiDrawBatcherTests.cpp
)
yuengine_apply_compile_options(YuUiDrawBatcherTests)
target_link_libraries(YuUiDrawBatcherTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_DrawBatcher_DeterministicFixtureBatchCount YuUiDrawBatcherTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DrawBatcher_SplitsTextAndTextureState YuUiDrawBatcherTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DrawBatcher_RejectsInvalidElementWithoutMutation YuUiDrawBatcherTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DrawBatcher_RejectsSmallOutputWithoutMutation YuUiDrawBatcherTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiDynamicAtlasTests
    Tests/UiCore/UiDynamicAtlasTests.cpp
)
yuengine_apply_compile_options(YuUiDynamicAtlasTests)
target_link_libraries(YuUiDynamicAtlasTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_DynamicAtlasPacker_SafePointPacksAndResolves YuUiDynamicAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DynamicAtlasPacker_RejectsPaintPathPacking YuUiDynamicAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DynamicAtlasPacker_ReportsOverflowWithoutMutation YuUiDynamicAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_DynamicAtlasPacker_RejectsSmallOutputAndInvalidRequest YuUiDynamicAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiGridViewSemanticsTests
    Tests/UiCore/UiGridViewSemanticsTests.cpp
)
yuengine_apply_compile_options(YuUiGridViewSemanticsTests)
target_link_libraries(YuUiGridViewSemanticsTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_GridViewSemantics_GroupsCellsAndVisibleBufferRange YuUiGridViewSemanticsTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewSemantics_RejectsFullPoolAsNonVirtualized YuUiGridViewSemanticsTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewSemantics_BtnSelectionTouchesVisibleCellsOnly YuUiGridViewSemanticsTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewSemantics_ResolvesScrollToIndexTarget YuUiGridViewSemanticsTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewSemantics_RejectsInvalidAndSmallOutput YuUiGridViewSemanticsTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiGridViewVirtualizerTests
    Tests/UiCore/UiGridViewVirtualizerTests.cpp
)
yuengine_apply_compile_options(YuUiGridViewVirtualizerTests)
target_link_libraries(YuUiGridViewVirtualizerTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_GridViewVirtualizer_LargeListUsesVisibleBufferPool YuUiGridViewVirtualizerTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewVirtualizer_TailGroupKeepsEmptyPoolSlotHidden YuUiGridViewVirtualizerTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewVirtualizer_RejectsFullPoolAsNonVirtualized YuUiGridViewVirtualizerTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewVirtualizer_RejectsSmallOutputWithoutMutation YuUiGridViewVirtualizerTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_GridViewVirtualizer_ResolvesAutoScrollAndAffectedDirtyCells YuUiGridViewVirtualizerTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiFontGlyphAtlasTests
    Tests/UiCore/UiFontGlyphAtlasTests.cpp
)
yuengine_apply_compile_options(YuUiFontGlyphAtlasTests)
target_link_libraries(YuUiFontGlyphAtlasTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_FontGlyphAtlas_SafePointBuildResolvesPrimaryGlyph YuUiFontGlyphAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_FontGlyphAtlas_TextFixtureResolvesFallbackGlyph YuUiFontGlyphAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_FontGlyphAtlas_TextFixtureReportsMissingGlyph YuUiFontGlyphAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_FontGlyphAtlas_RejectsPaintPathPackAndDuplicateGlyph YuUiFontGlyphAtlasTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiButtonComponentTests
    Tests/UiCore/UiButtonComponentTests.cpp
)
yuengine_apply_compile_options(YuUiButtonComponentTests)
target_link_libraries(YuUiButtonComponentTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_ButtonComponent_ResolvesStateVisuals YuUiButtonComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ButtonComponent_PointerActivationEmitsHooks YuUiButtonComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ButtonComponent_KeyboardGamepadActivationPathsAreExplicit YuUiButtonComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ButtonComponent_StateDirtyDoesNotTriggerLayout YuUiButtonComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ButtonComponent_RejectsInvalidDesc YuUiButtonComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiSliderComponentTests
    Tests/UiCore/UiSliderComponentTests.cpp
)
yuengine_apply_compile_options(YuUiSliderComponentTests)
target_link_libraries(YuUiSliderComponentTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_SliderComponent_MapsValueToFillAndHandle YuUiSliderComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_SliderComponent_PointerCaptureUpdatesValueAndHook YuUiSliderComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_SliderComponent_KeyboardGamepadAdjustmentPathsAreExplicit YuUiSliderComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_SliderComponent_ValueDirtyDoesNotTriggerLayout YuUiSliderComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_SliderComponent_RejectsInvalidDesc YuUiSliderComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiImageComponentTests
    Tests/UiCore/UiImageComponentTests.cpp
)
yuengine_apply_compile_options(YuUiImageComponentTests)
target_link_libraries(YuUiImageComponentTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_ImageComponent_BuildsSpriteDrawRecord YuUiImageComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ImageComponent_BuildsNineSliceRecordsAndBatches YuUiImageComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ImageComponent_ReportsMissingSpriteWithoutMutation YuUiImageComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ImageComponent_RejectsSmallOutputWithoutMutation YuUiImageComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_ImageComponent_PaintInvalidationDoesNotTriggerLayout YuUiImageComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiTextComponentTests
    Tests/UiCore/UiTextComponentTests.cpp
)
yuengine_apply_compile_options(YuUiTextComponentTests)
target_link_libraries(YuUiTextComponentTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiCore_TextComponent_RendersAlignedWrappedPlainText YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_TextComponent_LocalizePlaceholderUsesFallback YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_TextComponent_ClipsOverflowAndWritesEffects YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_TextComponent_TextDirtyDoesNotTriggerLayout YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_TextComponent_ReportsMissingGlyphWithoutMutation YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiCore_TextComponent_RejectsSmallOutputWithoutMutation YuUiTextComponentTests Fast ModuleFixture UiCore UI L1 EvidenceOracle)

add_executable(YuUiRuntimeTests
    Tests/UiRuntime/BaseUiControllerTests.cpp
)
yuengine_apply_compile_options(YuUiRuntimeTests)
target_link_libraries(YuUiRuntimeTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_BaseUiController_InitializeBindsOnce YuUiRuntimeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_BaseUiController_OpenClosePerActivation YuUiRuntimeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_BaseUiController_DestroyClearsOnce YuUiRuntimeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_BaseUiController_CloseSelfThroughManager YuUiRuntimeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_BaseUiController_RejectsInvalidLifecycleCalls YuUiRuntimeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiPanelRegistryTests
    Tests/UiRuntime/UiPanelRegistryTests.cpp
)
yuengine_apply_compile_options(YuUiPanelRegistryTests)
target_link_libraries(YuUiPanelRegistryTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_PanelRegistry_RegistersAndResolvesRefs YuUiPanelRegistryTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelRegistry_RejectsDuplicateAndMissingPanels YuUiPanelRegistryTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelRegistry_LoadsExplicitTestManifestAtomically YuUiPanelRegistryTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelRegistry_ExportsManifestAsReadOnlyCopy YuUiPanelRegistryTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiManagerLayerModelTests
    Tests/UiRuntime/UiManagerLayerModelTests.cpp
)
yuengine_apply_compile_options(YuUiManagerLayerModelTests)
target_link_libraries(YuUiManagerLayerModelTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_ManagerLayerModel_RegistersFiveLayerRoots YuUiManagerLayerModelTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerLayerModel_RejectsDuplicateAndMissingLayers YuUiManagerLayerModelTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerLayerModel_ExportsLayersInStableOrder YuUiManagerLayerModelTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerLayerModel_ResolvesPanelIdsToLayerRoots YuUiManagerLayerModelTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiManagerPanelMapTests
    Tests/UiRuntime/UiManagerPanelMapTests.cpp
)
yuengine_apply_compile_options(YuUiManagerPanelMapTests)
target_link_libraries(YuUiManagerPanelMapTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_OpenCloseReopenRetainsLoadedController YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_RejectsMissingPanelLayerAndController YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_DuplicateOpenCloseAreIdempotent YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_ResolvesLoadedAndActiveLayerRoots YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_ReleaseClearsLoadedControllerAndArgs YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPanelMap_ReleaseActiveAndReopenUsesNewLifecycle YuUiManagerPanelMapTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiManagerPopupStackTests
    Tests/UiRuntime/UiManagerPopupStackTests.cpp
)
yuengine_apply_compile_options(YuUiManagerPopupStackTests)
target_link_libraries(YuUiManagerPopupStackTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_OpenPushesPopupOrder YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_BringExistingPopupToTop YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_CloseRemovesPopupFromStack YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_DuplicateOpenMovesExistingPopup YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_RejectsMissingNonPopupAndUntrackedStatus YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerPopupStack_ReleaseRemovesPopupAndPanelCache YuUiManagerPopupStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiManagerFullscreenStackTests
    Tests/UiRuntime/UiManagerFullscreenStackTests.cpp
)
yuengine_apply_compile_options(YuUiManagerFullscreenStackTests)
target_link_libraries(YuUiManagerFullscreenStackTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_OpenPushesFullscreenOrder YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_NavigateBackRestoresPreviousFullscreen YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_CloseCurrentAndMiddleFullscreen YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_DuplicateOpenMovesExistingFullscreen YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_RejectsMissingNonFullscreenAndBackEmptyStatus YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_ManagerFullscreenStack_ReleaseRestoresPreviousFullscreen YuUiManagerFullscreenStackTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiPanelParameterTests
    Tests/UiRuntime/UiPanelParameterTests.cpp
)
yuengine_apply_compile_options(YuUiPanelParameterTests)
target_link_libraries(YuUiPanelParameterTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_PanelParameter_OpenArgsReachController YuUiPanelParameterTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelParameter_EmptyArgsAndInvalidArgsAreStable YuUiPanelParameterTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelParameter_ReopenLoadedUpdatesArgs YuUiPanelParameterTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_PanelParameter_PopupAndFullscreenPassArgs YuUiPanelParameterTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiSimplePopupToastRepresentativeTests
    Tests/UiRuntime/UiSimplePopupToastRepresentativeTests.cpp
)
yuengine_apply_compile_options(YuUiSimplePopupToastRepresentativeTests)
target_link_libraries(YuUiSimplePopupToastRepresentativeTests PRIVATE YuUiRuntime)
yuengine_add_labeled_test(UiRuntime_SimplePopupToastRepresentative_OpenDisplaysAndCloseClears YuUiSimplePopupToastRepresentativeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_SimplePopupToastRepresentative_ReopenBeforeReleaseUsesCache YuUiSimplePopupToastRepresentativeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_SimplePopupToastRepresentative_ReleaseClearsLoadedCache YuUiSimplePopupToastRepresentativeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_SimplePopupToastRepresentative_ReleaseThenReopenUsesFreshLifecycle YuUiSimplePopupToastRepresentativeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRuntime_SimplePopupToastRepresentative_StatusAndOrderRemainExplicit YuUiSimplePopupToastRepresentativeTests Fast ModuleFixture UiRuntime UI L1 EvidenceOracle)

add_executable(YuUiEditorTests
    Tests/UiEditor/UiEditorTests.cpp
)
yuengine_apply_compile_options(YuUiEditorTests)
target_link_libraries(YuUiEditorTests PRIVATE YuUiEditorShellCore)
yuengine_add_labeled_test(UiEditor_ShellState_DefaultPlaceholdersOpen YuUiEditorTests Fast ModuleFixture UiEditor UI Editor EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ShellState_ClosePanelUpdatesSnapshot YuUiEditorTests Fast ModuleFixture UiEditor UI Editor EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ShellState_RejectsSmallPanelOutput YuUiEditorTests Fast ModuleFixture UiEditor UI Editor EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ShellState_DearImGuiBackendGateIsExplicit YuUiEditorTests Fast ModuleFixture UiEditor UI Editor EvidenceOracle)
yuengine_add_labeled_test(UiEditor_LayoutAsset_LoadsHierarchyAndInspector YuUiEditorTests Fast ModuleFixture UiEditor UI Editor LayoutAsset EvidenceOracle)
yuengine_add_labeled_test(UiEditor_LayoutAsset_SelectNodeUpdatesInspector YuUiEditorTests Fast ModuleFixture UiEditor UI Editor LayoutAsset EvidenceOracle)
yuengine_add_labeled_test(UiEditor_LayoutAsset_RejectsDuplicateNodeId YuUiEditorTests Fast ModuleFixture UiEditor UI Editor LayoutAsset EvidenceOracle)
yuengine_add_labeled_test(UiEditor_LayoutAsset_RejectsMissingParent YuUiEditorTests Fast ModuleFixture UiEditor UI Editor LayoutAsset EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewVariants_ResolutionDpiSafeAreaExport YuUiEditorTests Fast ModuleFixture UiEditor UI Editor PreviewVariant EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewVariants_RejectInvalidSafeAreaWithoutMutation YuUiEditorTests Fast ModuleFixture UiEditor UI Editor PreviewVariant EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewViewport_RendersLoadedLayoutThroughRuntimePath YuUiEditorTests Fast ModuleFixture UiEditor UI Editor RenderCore EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewViewport_RejectsBeforeLayoutLoad YuUiEditorTests Fast ModuleFixture UiEditor UI Editor RenderCore EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewViewport_RejectsInvalidViewport YuUiEditorTests Fast ModuleFixture UiEditor UI Editor RenderCore EvidenceOracle)
yuengine_add_labeled_test(UiEditor_PreviewViewport_WorksWithoutDearImGuiBackend YuUiEditorTests Fast ModuleFixture UiEditor UI Editor RenderCore EvidenceOracle)

add_executable(YuUiEditorResourceReferenceValidatorTests
    Tests/UiEditor/UiEditorResourceReferenceValidatorTests.cpp
)
yuengine_apply_compile_options(YuUiEditorResourceReferenceValidatorTests)
target_link_libraries(YuUiEditorResourceReferenceValidatorTests PRIVATE YuUiEditorShellCore)
yuengine_add_labeled_test(UiEditor_ResourceValidator_ReportsMissingReferences YuUiEditorResourceReferenceValidatorTests Fast ModuleFixture UiEditor UI Editor ResourceValidator EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ResourceValidator_AcceptsExistingReferences YuUiEditorResourceReferenceValidatorTests Fast ModuleFixture UiEditor UI Editor ResourceValidator EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ResourceValidator_RejectsSmallReportOutputWithoutMutation YuUiEditorResourceReferenceValidatorTests Fast ModuleFixture UiEditor UI Editor ResourceValidator EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ResourceValidator_RejectsInvalidReferenceWithoutMutation YuUiEditorResourceReferenceValidatorTests Fast ModuleFixture UiEditor UI Editor ResourceValidator EvidenceOracle)
yuengine_add_labeled_test(UiEditor_ResourceValidator_RejectsInvalidAtlasMetadata YuUiEditorResourceReferenceValidatorTests Fast ModuleFixture UiEditor UI Editor ResourceValidator EvidenceOracle)

add_executable(YuUiRenderCoreBridgeTests
    Tests/UiRenderCoreBridge/UiRenderCoreBridgeTests.cpp
)
yuengine_apply_compile_options(YuUiRenderCoreBridgeTests)
target_link_libraries(YuUiRenderCoreBridgeTests PRIVATE YuUiRenderCoreBridge YuRHI)
yuengine_add_labeled_test(UiRenderCoreBridge_SubmitsDrawElementsThroughRenderCore YuUiRenderCoreBridgeTests Fast ModuleFixture UiCore RenderCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRenderCoreBridge_RejectsInvalidDrawElementWithoutSubmission YuUiRenderCoreBridgeTests Fast ModuleFixture UiCore RenderCore UI L1 EvidenceOracle)
yuengine_add_labeled_test(UiRenderCoreBridge_RejectsSmallOutputStorageWithoutSubmission YuUiRenderCoreBridgeTests Fast ModuleFixture UiCore RenderCore UI L1 EvidenceOracle)

add_executable(YuUiCoreSmokeSampleTests
    Samples/UiCoreSmoke/Source/UiCoreSmokeSample.cpp
    Tests/UiCoreSmoke/UiCoreSmokeSampleTests.cpp
)
yuengine_apply_compile_options(YuUiCoreSmokeSampleTests)
target_include_directories(YuUiCoreSmokeSampleTests PRIVATE Samples/UiCoreSmoke/Source)
target_compile_definitions(YuUiCoreSmokeSampleTests PRIVATE
    YU_UI_CORE_SMOKE_LAYOUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Samples/UiCoreSmoke/Layouts/SimpleWindow.YuUILayout.json"
)
target_link_libraries(YuUiCoreSmokeSampleTests PRIVATE YuUiRenderCoreBridge YuRHI)
yuengine_add_labeled_test(UiCoreSmokeSample_LoadsLayoutRendersWindowReportsPass YuUiCoreSmokeSampleTests Fast ModuleFixture UiCore RenderCore UI Sample L1 EvidenceOracle)
yuengine_add_labeled_test(UiCoreSmokeSample_RejectsInvalidLayoutWithoutRenderSubmission YuUiCoreSmokeSampleTests Fast ModuleFixture UiCore RenderCore UI Sample L1 EvidenceOracle)
yuengine_add_labeled_test(UiCoreSmokeSample_ValidationRouteDocumentsCommands YuUiCoreSmokeSampleTests Fast ModuleFixture UiCore RenderCore UI Sample L1 EvidenceOracle)

add_executable(YuUiComponentSmokeSampleTests
    Samples/UiComponentSmoke/Source/UiComponentSmokeSample.cpp
    Tests/UiComponentSmoke/UiComponentSmokeSampleTests.cpp
)
yuengine_apply_compile_options(YuUiComponentSmokeSampleTests)
target_include_directories(YuUiComponentSmokeSampleTests PRIVATE Samples/UiComponentSmoke/Source)
target_compile_definitions(YuUiComponentSmokeSampleTests PRIVATE
    YU_UI_COMPONENT_SMOKE_LAYOUT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/Samples/UiComponentSmoke/Layouts/ComponentWindow.YuUILayout.json"
)
target_link_libraries(YuUiComponentSmokeSampleTests PRIVATE YuUiCore)
yuengine_add_labeled_test(UiComponentSmokeSample_UsesStage2ComponentsInOneWindow YuUiComponentSmokeSampleTests Fast ModuleFixture UiCore UI Sample L1 EvidenceOracle)
yuengine_add_labeled_test(UiComponentSmokeSample_RejectsInvalidLayoutBeforeComponents YuUiComponentSmokeSampleTests Fast ModuleFixture UiCore UI Sample L1 EvidenceOracle)
yuengine_add_labeled_test(UiComponentSmokeSample_KeepsGridViewListVirtualized YuUiComponentSmokeSampleTests Fast ModuleFixture UiCore UI Sample L1 EvidenceOracle)
yuengine_add_labeled_test(UiComponentSmokeSample_ReportsPerformanceDiagnostics YuUiComponentSmokeSampleTests Fast ModuleFixture UiCore UI Sample L1 EvidenceOracle)

add_executable(YuSerializeTests
    Tests/Serialize/SerializeTests.cpp
)
yuengine_apply_compile_options(YuSerializeTests)
target_link_libraries(YuSerializeTests PRIVATE YuSerialize)
yuengine_add_labeled_test(Serialize_WriteReadPrimitives_RoundTripsDeterministically YuSerializeTests Fast ModuleFixture Serialize EvidenceOracle)
yuengine_add_labeled_test(Serialize_StreamHeader_RejectsInvalidMagicOrVersion YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_StreamHeader_RejectsReservedFlags YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_WriterBufferOverflow_ReturnsStatusWithoutOverrun YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_RecordCapacityOverflow_DoesNotMutate YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_FieldCapacityOverflow_DoesNotMutate YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_FixedBytesPayloadLimit_ReturnsExplicitStatus YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_ReaderRejectsTruncatedStream YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_ReaderRejectsInvalidRecordOrFieldId YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_ReaderRejectsMalformedFieldLength YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_ReaderRejectsUnknownTypeTag YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_ReaderTypeMismatch_ReturnsExplicitStatus YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_DuplicateField_ReturnsExplicitStatus YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_UnknownFieldWithValidLength_CanSkipDeterministically YuSerializeTests Fast ModuleFixture Serialize EvidenceOracle)
yuengine_add_labeled_test(Serialize_DisabledDiagnostics_DoesNotChangeResults YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_NoFilePackageResourceObjectOrGameAdapterDependency YuSerializeTests Fast ModuleFixture Serialize EvidenceOracle)
yuengine_add_labeled_test(Serialize_NoHiddenAllocationInReadWritePath YuSerializeTests Fast ModuleFixture Serialize PerformanceSmoke)
yuengine_add_labeled_test(Serialize_SnapshotReportsCountsAndLastStatus YuSerializeTests Fast ModuleFixture Serialize)
yuengine_add_labeled_test(Serialize_RuntimeConfigStream_RoundTripsCallerOwnedConfigBoundary YuSerializeTests Fast ModuleFixture Serialize L1 EvidenceOracle)
yuengine_add_labeled_test(Serialize_RuntimeConfigStream_RejectsUnsupportedVersionWithoutMutation YuSerializeTests Fast ModuleFixture Serialize L1 EvidenceOracle)
yuengine_add_labeled_test(Serialize_RuntimeConfigStream_KeepsPersistencePolicyOutsideCore YuSerializeTests Fast ModuleFixture Serialize L1 EvidenceOracle)

add_executable(YuPackageTests
    Tests/Package/PackageTests.cpp
)
yuengine_apply_compile_options(YuPackageTests)
target_link_libraries(YuPackageTests PRIVATE YuPackage)
yuengine_add_labeled_test(Package_RegisterSyntheticManifest_ReturnsStableId YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterDuplicateManifest_ReturnsExplicitStatus YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterEntry_ReturnsStableEntryId YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterDuplicateEntry_ReturnsExplicitStatus YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterDuplicateResourceKey_ReturnsExplicitStatus YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterInvalidIdsOrType_ReturnsExplicitStatusWithoutMutation YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_ManifestCapacityOverflow_DoesNotMutate YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_EntryCapacityOverflow_DoesNotMutate YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterEntryRejectsOversizedKeysWithoutMutation YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_RegisterEntryRejectsOversizedByteRangeWithoutMutation YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_ResolveEntryByResourceKey_ReturnsDeterministicLoadPlan YuPackageTests Fast ModuleFixture Package EvidenceOracle)
yuengine_add_labeled_test(Package_ResolveEntryByResourceKey_UsesTypeAndLogicalKeyTuple YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_IndexedLookupsPreserveStatusOrderAndCapacities YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_ResolveRejectsUnknownResourceKey YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_ResolveRejectsTypeMismatchWithoutMutation YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DependencyValidationRejectsMissingEntry YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DependencyValidationRejectsCycle YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DependencyValidationRejectsCycleAfterHighFanout YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DependencyPlanPreservesDeclarationOrder YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DependencyCapacityOverflow_DoesNotMutate YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_LoadPlanCapacityOverflow_DoesNotMutate YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_DisabledDiagnosticsDoesNotChangeResults YuPackageTests Fast ModuleFixture Package)
yuengine_add_labeled_test(Package_NoFileReadOriginalPackageOrGameAdapterDependency YuPackageTests Fast ModuleFixture Package EvidenceOracle)
yuengine_add_labeled_test(Package_NoHiddenAllocation_UsesYuMemorySignal YuPackageTests Fast ModuleFixture Package PerformanceSmoke)

add_executable(YuStreamingTests
    Tests/Streaming/StreamingTests.cpp
)
yuengine_apply_compile_options(YuStreamingTests)
target_link_libraries(YuStreamingTests PRIVATE YuStreaming)
target_compile_definitions(YuStreamingTests PRIVATE YUENGINE_FILE_FIXTURE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/Tests/Fixtures/File")
yuengine_add_labeled_test(Streaming_PackageResourceStaging_SubmitsAndCompletesFixtureRead YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_RejectsInvalidResourceHandleWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_RejectsResourceTypeMismatchWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_RejectsByteRangeOverflowWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_ReportsMissingFileCompletion YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_RejectsQueueOverflowWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO PerformanceSmoke)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_ReportsCompletionOverflowWithoutDroppingPending YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_RejectsDuplicateRequestId YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_SnapshotReportsBoundedCounters YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_PackageResourceStaging_NoUpperRuntimeDependency YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_CreateBufferFromStagingCompletion YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_UpdateBufferSignalsFence YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_CreateTextureFromStagingCompletion YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_UpdateTextureSignalsFence YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsInvalidResourceHandleWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsResourceTypeMismatchWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsFailedStagingCompletion YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsEmptyUploadBytes YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsByteRangeOverflow YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsNullRhiDevice YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsNullOutputStorage YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsUnsupportedUploadKind YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsQueueOverflowWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_ReportsCompletionOverflowWithoutProcessingPending YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_RejectsDuplicateUploadId YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_SnapshotReportsBoundedCounters YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUpload_ReportsRhiFailureWithoutWritingOutput YuStreamingTests Fast ModuleFixture Streaming Upload Resource RHI EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_CommitsSuccessfulUpload YuStreamingTests Fast ModuleFixture Streaming Upload Resource EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_CommitsFailedUpload YuStreamingTests Fast ModuleFixture Streaming Upload Resource EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_RejectsInvalidResourceHandleWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_RejectsTypeMismatchWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_RejectsDuplicateCommitId YuStreamingTests Fast ModuleFixture Streaming Upload Resource EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_RejectsQueueOverflowWithoutMutation YuStreamingTests Fast ModuleFixture Streaming Upload Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_ReportsCompletionOverflowWithoutProcessingPending YuStreamingTests Fast ModuleFixture Streaming Upload Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_PreservesOldestOrderAfterSlotReuse YuStreamingTests Fast ModuleFixture Streaming Upload Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceUploadCommit_SnapshotReportsBoundedCounters YuStreamingTests Fast ModuleFixture Streaming Upload Resource PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceStreamingPipeline_FixtureBufferReadUploadCommit YuStreamingTests Fast ModuleFixture Streaming Upload Resource Package File AsyncIO RHI PerformanceSmoke EvidenceOracle)

add_executable(YuResourceDecodedTextureBridgeTests
    Tests/Streaming/ResourceDecodedTextureBridgeTests.cpp
)
yuengine_apply_compile_options(YuResourceDecodedTextureBridgeTests)
target_link_libraries(YuResourceDecodedTextureBridgeTests PRIVATE YuStreaming YuRenderCore)
yuengine_add_labeled_test(Streaming_ResourceDecodedTextureBridge_UploadsDecodedPayloadAsTextureBinding YuResourceDecodedTextureBridgeTests Fast ModuleFixture Streaming Resource RHI RenderCore TextureSampling EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceDecodedTextureBridge_RejectsTextureByteMismatchWithoutRhiMutation YuResourceDecodedTextureBridgeTests Fast ModuleFixture Streaming Resource RHI RenderCore TextureSampling EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceDecodedTextureBridge_RejectsSmallScratchWithoutRhiMutation YuResourceDecodedTextureBridgeTests Fast ModuleFixture Streaming Resource RHI RenderCore TextureSampling EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceDecodedTextureBridge_RejectsSampledTextureSlotOutOfRangeWithoutRhiMutation YuResourceDecodedTextureBridgeTests Fast ModuleFixture Streaming Resource RHI RenderCore TextureSampling EvidenceOracle)
yuengine_add_labeled_test(Streaming_ResourceDecodedTextureBridge_ReportsRhiTextureCapacityWithoutWritingOutput YuResourceDecodedTextureBridgeTests Fast ModuleFixture Streaming Resource RHI RenderCore TextureSampling PerformanceSmoke EvidenceOracle)

add_executable(YuAssetTests
    Tests/Asset/AssetManagerTests.cpp
)
yuengine_apply_compile_options(YuAssetTests)
target_link_libraries(YuAssetTests PRIVATE YuAsset)
yuengine_add_labeled_test(Asset_RegisterRuntimeAsset_ReturnsStableHandleAndState YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_DependenciesTraverseBoundedAndRejectCycle YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_TextureReadyRecordUsesStreamingResultWithoutOwningDevice YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 TextureSampling EvidenceOracle)
yuengine_add_labeled_test(Asset_AudioReadyRecordUsesImportRecordWithoutOwningDevice YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_RefreshStateFromResourceMapsUploadedResidentAndFailed YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_ReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_RuntimeFixtureClosesResidentTexturePathAndDependencies YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_RuntimeFixtureRejectsSmallDependencyOutputBeforeMutation YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(Asset_NoWorldGameAdapterUiDependency YuAssetTests Fast ModuleFixture Asset Resource Streaming AudioResource L1 EvidenceOracle)

add_executable(YuRenderSceneTests
    Tests/RenderScene/RenderSceneContractQueueTests.cpp
)
yuengine_apply_compile_options(YuRenderSceneTests)
target_link_libraries(YuRenderSceneTests PRIVATE YuRenderScene)
yuengine_add_labeled_test(RenderScene_BuildsRenderCoreViewPacket YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)
yuengine_add_labeled_test(RenderScene_FullListVisibilityOutputsOnlyVisibleEntities YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)
yuengine_add_labeled_test(RenderScene_MissingMeshAssetDoesNotMutateOutput YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)
yuengine_add_labeled_test(RenderScene_MissingMaterialAssetDoesNotMutateOutput YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)
yuengine_add_labeled_test(RenderScene_MissingCameraDoesNotMutateOutput YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)
yuengine_add_labeled_test(RenderScene_OutputCapacityFailureDoesNotMutateOutput YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 PerformanceSmoke EvidenceOracle)
yuengine_add_labeled_test(RenderScene_NoNativeOrUpperDependency YuRenderSceneTests Fast ModuleFixture RenderScene RenderCore Asset World L1 EvidenceOracle)

add_executable(YuAudioSceneTests
    Tests/AudioScene/AudioSceneContractQueueTests.cpp
)
yuengine_apply_compile_options(YuAudioSceneTests)
target_link_libraries(YuAudioSceneTests PRIVATE YuAudioScene)
yuengine_add_labeled_test(AudioScene_PlayingSourceBuildsPcmQueueRequest YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_BusRoutingMapsFixedBusIdsToQueueIds YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_MissingBackendReturnsExplicitStatus YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_PausedSourceDoesNotSubmitQueue YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_MissingSoundAssetDoesNotMutateOutput YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_InvalidBusDoesNotMutateOutput YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)
yuengine_add_labeled_test(AudioScene_NoNativeOrUpperDependency YuAudioSceneTests Fast ModuleFixture AudioScene Audio Asset AudioResource L1 EvidenceOracle)

add_executable(YuSampleTests
    Samples/AssetSmokeDemo/Source/L1VerticalSamplePrep.cpp
    Tests/Sample/L1VerticalSamplePrepTests.cpp
)
yuengine_apply_compile_options(YuSampleTests)
target_include_directories(YuSampleTests PRIVATE Samples/AssetSmokeDemo/Source)
target_link_libraries(YuSampleTests PRIVATE YuAsset YuAudioScene YuInput YuKernel YuRenderScene YuSerialize YuWorld)
yuengine_add_labeled_test(Sample_L1VerticalPrep_BuildsManifestAndSubmitPrep YuSampleTests Fast ModuleFixture Sample L1 RuntimeApp Asset Input RenderScene AudioScene EvidenceOracle)
yuengine_add_labeled_test(Sample_L1VerticalPrep_UsesValueContractsWithoutHardware YuSampleTests Fast ModuleFixture Sample L1 EvidenceOracle)
yuengine_add_labeled_test(Sample_L1VerticalPrep_DeterministicObjectGraphHasStableSlots YuSampleTests Fast ModuleFixture Sample L1 World Object EvidenceOracle)
yuengine_add_labeled_test(Sample_L1VerticalPrep_RoutesAssetRenderAudioAndLifecycle YuSampleTests Fast ModuleFixture Sample L1 Asset RenderScene AudioScene EvidenceOracle)
yuengine_add_labeled_test(Sample_L1VerticalPrep_ExposesDebugReleaseFastValidationRoute YuSampleTests Fast ModuleFixture Sample L1 Diagnostics EvidenceOracle)
yuengine_add_labeled_test(Sample_L1VerticalPrep_RoundTripsStateAndCleansActiveRecords YuSampleTests Fast ModuleFixture Sample L1 Serialize World Resource Asset EvidenceOracle)

add_executable(YuInputTests
    Tests/Input/InputTests.cpp
)
yuengine_apply_compile_options(YuInputTests)
target_include_directories(YuInputTests PRIVATE Src/YuEngine/Input/Src)
target_link_libraries(YuInputTests PRIVATE YuInput)
yuengine_add_labeled_test(Input_RegisterActionBinding_ReturnsStableActionId YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_RegisterControlAlreadyBound_ReturnsDuplicateStatus YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_MultipleControlsForOneAction_UsesInsertionOrder YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BindingCapacityOverflow_DoesNotMutate YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_ReplayFrame_AppliesButtonPressAndRelease YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_ReplayFrame_EventOrderIsDeterministic YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_ReplayFrame_EventOrderLastValidValueWins YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_ReplayFrame_PressReleaseSameFrame_SetsChangedFlag YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_ReplayFrame_AppliesFixedPointAxisValue YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_InvalidAxisValue_ReturnsExplicitStatusWithoutMutation YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_InvalidEvent_DoesNotMutateReplayOrSnapshot YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_UnknownDeviceControlOrAction_ReturnsExplicitStatus YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_EventCapacityOverflow_DoesNotMutateReplay YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_FrameSnapshot_IsDeterministicAcrossReplay YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_ResetClearsChangedStateWithoutClearingPressedState YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_DisabledDiagnosticsDoesNotChangeResults YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_FrameApply_DoesNotGrowReplayStorage YuInputTests Fast ModuleFixture Input PerformanceSmoke)
yuengine_add_labeled_test(Input_NoPlatformUiOrGameAdapterDependency YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeDesc_DefaultValuesAreBounded YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_BridgePublicContract_UsesValueTypes YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeInitialize_RejectsUnsupportedBackend YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BridgeSubmitAndDrain_RecordsKeyboardMouseWheel YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BridgeFocusLost_RejectsInputAndTracksCounters YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BridgeCapacityOverflow_DoesNotGrow YuInputTests Fast ModuleFixture Input PerformanceSmoke)
yuengine_add_labeled_test(Input_BridgeDrain_RejectsSmallOutputWithoutMutation YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BridgeGamepadUnavailableState_IsExplicit YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeGamepadConnectedState_QueuesButtonAndAxisEvents YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeGamepadRepeatedPacket_DoesNotQueueDuplicateEvents YuInputTests Fast ModuleFixture Input)
yuengine_add_labeled_test(Input_BridgeGamepadCapacityOverflow_DoesNotQueuePartialEvents YuInputTests Fast ModuleFixture Input PerformanceSmoke)
yuengine_add_labeled_test(Input_BridgeXInputPollUnavailable_ReturnsDeviceUnavailable YuInputTests Fast ModuleFixture Input XInput EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeXInputPollConnected_QueuesButtonAxisAndPacket YuInputTests Fast ModuleFixture Input XInput EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeXInputPollBackendError_ReturnsExplicitStatus YuInputTests Fast ModuleFixture Input XInput EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeXInputPollInvalidUserIndex_ReturnsExplicitStatus YuInputTests Fast ModuleFixture Input XInput EvidenceOracle)
yuengine_add_labeled_test(Input_BridgeNoUiGameOrReportDispatch YuInputTests Fast ModuleFixture Input EvidenceOracle)
yuengine_add_labeled_test(Input_CommandMapper_KeyboardBuildsFrameCommandSnapshot YuInputTests Fast ModuleFixture Input L1 CommandSnapshot EvidenceOracle)
yuengine_add_labeled_test(Input_CommandMapper_XInputStyleAxisBuildsCommandSnapshot YuInputTests Fast ModuleFixture Input L1 CommandSnapshot XInput EvidenceOracle)
yuengine_add_labeled_test(Input_CommandMapper_KeyboardAndXInputFixturesStayRuntimeOnly YuInputTests Fast ModuleFixture Input L1 CommandSnapshot XInput EvidenceOracle)
yuengine_add_labeled_test(Input_CommandMapper_FocusRejectsInputWithoutMutation YuInputTests Fast ModuleFixture Input L1 CommandSnapshot EvidenceOracle)
yuengine_add_labeled_test(Input_CommandMapper_InvalidEventDoesNotMutateHeldState YuInputTests Fast ModuleFixture Input L1 CommandSnapshot EvidenceOracle)

add_executable(YuInputHardwareSmokeTests
    Tests/Input/InputHardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuInputHardwareSmokeTests)
target_link_libraries(YuInputHardwareSmokeTests PRIVATE YuInput)
yuengine_add_labeled_test(Input_HardwareBridge_TranslatesSourceMessages YuInputHardwareSmokeTests HardwareSmoke Input Win32 Win32Message)
yuengine_add_labeled_test(Input_HardwareBridge_PollsXInputGamepad YuInputHardwareSmokeTests HardwareSmoke Input Win32 XInput)
set_tests_properties(Input_HardwareBridge_PollsXInputGamepad PROPERTIES SKIP_RETURN_CODE 77)

add_executable(YuHardwareFrameHostTests
    Tests/Hardware/HardwareFrameHostTests.cpp
)
yuengine_apply_compile_options(YuHardwareFrameHostTests)
target_link_libraries(YuHardwareFrameHostTests PRIVATE YuHardware)
yuengine_add_labeled_test(HardwareFrameHost_TranslatesInjectedPlatformInput YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input EvidenceOracle)
yuengine_add_labeled_test(HardwareFrameHost_RejectsInvalidDescriptor YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input EvidenceOracle)
yuengine_add_labeled_test(HardwareFrameHost_RejectsTickBeforeInitialize YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input)
yuengine_add_labeled_test(HardwareFrameHost_GamepadPollRejectsInvalidIndex YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input XInput EvidenceOracle)
yuengine_add_labeled_test(HardwareFrameHost_ShutdownAfterSuccessIsIdempotent YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input EvidenceOracle)
yuengine_add_labeled_test(HardwareFrameHost_InvalidTickPreservesCallerOutput YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input EvidenceOracle)
yuengine_add_labeled_test(HardwareFrameHost_ResizeEventWithoutRenderCompletesTick YuHardwareFrameHostTests Fast ModuleFixture Hardware Platform Input Resize EvidenceOracle)

add_executable(YuHardwareFrameHostD3D11Smoke
    Tests/Hardware/HardwareFrameHostD3D11HardwareSmokeTests.cpp
)
yuengine_apply_compile_options(YuHardwareFrameHostD3D11Smoke)
target_link_libraries(YuHardwareFrameHostD3D11Smoke PRIVATE YuHardware)
yuengine_add_labeled_test(HardwareFrameHost_D3D11IntegratedFrameRuns YuHardwareFrameHostD3D11Smoke HardwareSmoke Hardware Platform Input RenderCore RHI D3D11 Win32 Audio XAudio2 FramePipeline EvidenceOracle)
