// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/KernelResult.h

#pragma once

#include <string>

#include "YuEngine/Kernel/KernelStatus.h"

namespace yuengine::kernel {
struct KernelResult {
    bool succeeded;
    KernelStatus status;
    std::string message;

    /**
     * @comment 创建成功结果。
     * @return 显式操作结果。
     */
    static KernelResult Success();
    /**
     * @comment 创建失败结果。
     * @param status 输入 状态。
     * @param message 输入 消息文本。
     * @return 显式操作结果。
     */
    static KernelResult Failure(KernelStatus status, std::string message);
};
}
