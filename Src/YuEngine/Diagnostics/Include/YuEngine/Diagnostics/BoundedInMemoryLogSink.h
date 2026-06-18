// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/BoundedInMemoryLogSink.h

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Diagnostics/ILogSink.h"
#include "YuEngine/Diagnostics/LogEvent.h"

namespace yuengine::diagnostics {
class BoundedInMemoryLogSink final : public ILogSink {
public:
    /**
     * @comment 构造 BoundedInMemoryLogSink 实例。
     * @param capacity 输入容量。
     */
    explicit BoundedInMemoryLogSink(std::size_t capacity);

    /**
     * @comment 写入 log message。
     * @param module_name 输入 log module name。
     * @param level 输入 level。
     * @param message 输入 message text。
     */
    void Write(std::string_view module_name, LogLevel level, std::string_view message) override;
    /**
     * @comment 设置全局 logging switch。
     * @param enabled 输入 enabled state。
     */
    void SetEnabled(bool enabled) override;
    /**
     * @comment 检查 component 是否启用。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsEnabled() const override;
    /**
     * @comment 设置单个 module 的 logging switch。
     * @param module_name 输入 log module name。
     * @param enabled 输入 enabled state。
     * @return module switch 已存储时返回 true，否则返回 false。
     */
    bool SetModuleEnabled(std::string_view module_name, bool enabled) override;
    /**
     * @comment 检查 one log module is enabled。
     * @param module_name 输入 log module name。
     * @return module 启用时返回 true，否则返回 false。
     */
    bool IsModuleEnabled(std::string_view module_name) const override;

    /**
     * @comment 返回已记录的 log events。
     * @return 请求对象的引用。
     */
    const std::vector<LogEvent>& Events() const;
    /**
     * @comment 返回 dropped records 数量。
     * @return Dropped count 值。
     */
    std::size_t DroppedCount() const;
    /**
     * @comment 清空 stored state。
     */
    void Clear();

private:
    bool ContainsDisabledModule(std::string_view module_name) const;
    void RemoveDisabledModule(std::string_view module_name);

    std::size_t capacity_;
    std::vector<LogEvent> events_;
    std::vector<std::string> disabled_modules_;
    std::size_t dropped_count_;
    bool is_enabled_;
};
}
