// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/RuntimeApp.h

#pragma once

#include <string>
#include <vector>

#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Kernel/RuntimeAppDesc.h"
#include "YuEngine/Kernel/RuntimeAppRunResult.h"
#include "YuEngine/Kernel/RuntimeAppSnapshot.h"
#include "YuEngine/Kernel/RuntimeAppStatus.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"

namespace yuengine::kernel {
class RuntimeApp final {
public:
    /**
     * @comment 初始化 RuntimeApp，绑定 caller-owned EngineKernel。
     * @param kernel 输入 caller-owned EngineKernel。
     * @param desc 输入 runtime descriptor。
     * @return descriptor 合法时返回 true，否则返回 false。
     */
    bool Initialize(EngineKernel* kernel, const RuntimeAppDesc& desc);

    /**
     * @comment 运行固定帧 zero-world capable loop。
     * @param lifecycle_trace 函数写入的 Kernel lifecycle trace。
     * @param phase_trace 函数写入的 frame phase trace。
     * @return runtime run result。
     */
    RuntimeAppRunResult RunFixedFrames(std::vector<std::string>* lifecycle_trace, std::vector<RuntimeFramePhase>* phase_trace);

    /**
     * @comment 返回 runtime 快照。
     * @return 当前 runtime snapshot。
     */
    RuntimeAppSnapshot Snapshot() const;

    /**
     * @comment 返回最近一帧的 FrameContext。
     * @return 当前 frame context。
     */
    RuntimeFrameContext FrameContext() const;

private:
    RuntimeFrameContext MakeFrameContext(std::uint32_t frame_index, RuntimeFramePhase phase) const;
    void RecordPhase(std::uint32_t frame_index, RuntimeFramePhase phase, std::vector<RuntimeFramePhase>* phase_trace);
    RuntimeAppRunResult MakeRunResult(
        RuntimeAppStatus status,
        KernelStatus kernel_status,
        KernelStatus shutdown_kernel_status,
        std::uint32_t completed_frame_count);

    EngineKernel* kernel_ = nullptr;
    RuntimeAppDesc desc_;
    RuntimeAppSnapshot snapshot_;
    RuntimeFrameContext frame_context_;
    bool initialized_ = false;
    bool running_ = false;
};
}
