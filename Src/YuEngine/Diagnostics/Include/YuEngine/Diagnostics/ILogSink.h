// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/ILogSink.h

#pragma once

#include <string_view>

#include "YuEngine/Diagnostics/LogLevel.h"

namespace yuengine::diagnostics {
class ILogSink {
public:
    virtual ~ILogSink() = default;

    /**
     * @comment 写入 log message。
     * @param module_name 输入 log module name。
     * @param level 输入 level。
     * @param message 输入 message text。
     */
    virtual void Write(std::string_view module_name, LogLevel level, std::string_view message) = 0;
    /**
     * @comment 设置全局 logging switch。
     * @param enabled 输入 enabled state。
     */
    virtual void SetEnabled(bool enabled) = 0;
    /**
     * @comment 检查 component 是否启用。
     * @return 条件满足时返回 true，否则返回 false。
     */
    virtual bool IsEnabled() const = 0;
    /**
     * @comment 设置单个 module 的 logging switch。
     * @param module_name 输入 log module name。
     * @param enabled 输入 enabled state。
     * @return module switch 已存储时返回 true，否则返回 false。
     */
    virtual bool SetModuleEnabled(std::string_view module_name, bool enabled) = 0;
    /**
     * @comment 检查 one log module is enabled。
     * @param module_name 输入 log module name。
     * @return module 启用时返回 true，否则返回 false。
     */
    virtual bool IsModuleEnabled(std::string_view module_name) const = 0;
};
}
