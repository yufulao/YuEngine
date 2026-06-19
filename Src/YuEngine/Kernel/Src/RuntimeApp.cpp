// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Src/RuntimeApp.cpp

#include "YuEngine/Kernel/RuntimeApp.h"

#include "YuEngine/Kernel/KernelResult.h"

namespace yuengine::kernel {
bool RuntimeApp::Initialize(EngineKernel* kernel, const RuntimeAppDesc& desc) {
    if (kernel == nullptr) {
        return false;
    }

    if (desc.frame_count == 0U) {
        return false;
    }

    if (desc.fixed_delta_time_nanoseconds == 0U) {
        return false;
    }

    if (running_) {
        return false;
    }

    kernel_ = kernel;
    desc_ = desc;
    snapshot_ = RuntimeAppSnapshot();
    frame_context_ = MakeFrameContext(0U, RuntimeFramePhase::BeginFrame);
    initialized_ = true;
    return true;
}

RuntimeAppRunResult RuntimeApp::RunFixedFrames(std::vector<std::string>* lifecycle_trace, std::vector<RuntimeFramePhase>* phase_trace) {
    if (!initialized_ || kernel_ == nullptr) {
        return MakeRunResult(RuntimeAppStatus::InvalidDescriptor, KernelStatus::InvalidLifecycle, KernelStatus::Success, 0U);
    }

    if (lifecycle_trace == nullptr) {
        return MakeRunResult(RuntimeAppStatus::InvalidDescriptor, KernelStatus::InvalidLifecycle, KernelStatus::Success, 0U);
    }

    if (phase_trace == nullptr) {
        return MakeRunResult(RuntimeAppStatus::InvalidDescriptor, KernelStatus::InvalidLifecycle, KernelStatus::Success, 0U);
    }

    if (running_) {
        return MakeRunResult(RuntimeAppStatus::AlreadyRunning, KernelStatus::InvalidLifecycle, KernelStatus::Success, 0U);
    }

    running_ = true;
    snapshot_ = RuntimeAppSnapshot();
    snapshot_.running = true;

    const KernelResult start_result = kernel_->Start(*lifecycle_trace);
    if (!start_result.succeeded) {
        running_ = false;
        snapshot_.running = false;
        return MakeRunResult(RuntimeAppStatus::KernelStartupFailure, start_result.status, KernelStatus::Success, 0U);
    }

    for (std::uint32_t frame_index = 0U; frame_index < desc_.frame_count; ++frame_index) {
        RecordPhase(frame_index, RuntimeFramePhase::BeginFrame, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::PollPlatform, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::PollInput, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::LoadOrCommitResources, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::UpdateWorld, phase_trace);

        const KernelResult update_result = kernel_->Update(frame_index, desc_.fixed_delta_time_nanoseconds, *lifecycle_trace);
        if (!update_result.succeeded) {
            const KernelResult shutdown_result = kernel_->Shutdown(*lifecycle_trace);
            running_ = false;
            snapshot_.running = false;
            return MakeRunResult(RuntimeAppStatus::KernelUpdateFailure, update_result.status, shutdown_result.status, frame_index);
        }

        RecordPhase(frame_index, RuntimeFramePhase::PrepareRender, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::SubmitAudio, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::SubmitRender, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::Present, phase_trace);
        RecordPhase(frame_index, RuntimeFramePhase::EndFrame, phase_trace);
        snapshot_.completed_frame_count = frame_index + 1U;
    }

    const KernelResult shutdown_result = kernel_->Shutdown(*lifecycle_trace);
    running_ = false;
    snapshot_.running = false;
    if (!shutdown_result.succeeded) {
        return MakeRunResult(
            RuntimeAppStatus::KernelShutdownFailure,
            KernelStatus::Success,
            shutdown_result.status,
            snapshot_.completed_frame_count);
    }

    return MakeRunResult(
        RuntimeAppStatus::Success,
        KernelStatus::Success,
        KernelStatus::Success,
        snapshot_.completed_frame_count);
}

RuntimeAppSnapshot RuntimeApp::Snapshot() const {
    return snapshot_;
}

RuntimeFrameContext RuntimeApp::FrameContext() const {
    return frame_context_;
}

RuntimeFrameContext RuntimeApp::MakeFrameContext(std::uint32_t frame_index, RuntimeFramePhase phase) const {
    RuntimeFrameContext context;
    context.frame_index = frame_index;
    context.delta_time_nanoseconds = desc_.fixed_delta_time_nanoseconds;
    context.fixed_time_nanoseconds = desc_.fixed_delta_time_nanoseconds;
    context.frame_mode = desc_.frame_mode;
    context.input_snapshot = desc_.input_snapshot;
    context.diagnostics_sink = desc_.diagnostics_sink;
    context.phase = phase;
    return context;
}

void RuntimeApp::RecordPhase(std::uint32_t frame_index, RuntimeFramePhase phase, std::vector<RuntimeFramePhase>* phase_trace) {
    frame_context_ = MakeFrameContext(frame_index, phase);
    snapshot_.current_phase = phase;
    phase_trace->push_back(phase);
}

RuntimeAppRunResult RuntimeApp::MakeRunResult(
    RuntimeAppStatus status,
    KernelStatus kernel_status,
    KernelStatus shutdown_kernel_status,
    std::uint32_t completed_frame_count) {
    RuntimeAppRunResult result;
    result.succeeded = status == RuntimeAppStatus::Success;
    result.status = status;
    result.kernel_status = kernel_status;
    result.shutdown_kernel_status = shutdown_kernel_status;
    result.completed_frame_count = completed_frame_count;
    result.last_frame_context = frame_context_;

    snapshot_.status = status;
    snapshot_.kernel_status = kernel_status;
    snapshot_.shutdown_kernel_status = shutdown_kernel_status;
    snapshot_.completed_frame_count = completed_frame_count;
    return result;
}
}
