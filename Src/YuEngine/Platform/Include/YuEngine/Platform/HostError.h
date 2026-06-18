// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/HostError.h

#pragma once

#include <string>

namespace yuengine::platform {
struct HostError {
    bool succeeded;
    std::string message;

    /**
     * @comment 创建成功结果。
     * @return 成功状态值。
     */
    static HostError Success();
    /**
     * @comment 创建失败结果。
     * @param message 输入 消息文本。
     * @return Failure 值。
     */
    static HostError Failure(std::string message);
};
}
