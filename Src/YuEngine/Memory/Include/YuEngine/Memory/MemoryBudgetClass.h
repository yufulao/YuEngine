// 模块: YuEngine Memory
// 文件: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryBudgetClass.h

#pragma once

#include <cstddef>

namespace yuengine::memory {
enum class MemoryBudgetClass {
    Setup = 0,
    Load = 1,
    Frame = 2,
    Callback = 3,
    Job = 4,
    Tool = 5
};

inline constexpr std::size_t MEMORY_BUDGET_CLASS_COUNT = 6U;

/**
 * @comment 返回 memory 预算类别 index。
 * @param budget_class 输入 预算类别。
 * @return Memory 预算类别 index 值。
 */
inline std::size_t MemoryBudgetClassIndex(MemoryBudgetClass budget_class) {
    return static_cast<std::size_t>(budget_class);
}

/**
 * @comment 检查 memory 预算类别是否有效。
 * @param budget_class 输入 预算类别。
 * @return 条件满足时返回 true，否则返回 false。
 */
inline bool IsValidMemoryBudgetClass(MemoryBudgetClass budget_class) {
    return MemoryBudgetClassIndex(budget_class) < MEMORY_BUDGET_CLASS_COUNT;
}

/**
 * @comment 检查 memory 预算类别是否为 hot-path。
 * @param budget_class 输入 预算类别。
 * @return 条件满足时返回 true，否则返回 false。
 */
inline bool IsHotMemoryBudgetClass(MemoryBudgetClass budget_class) {
    if (budget_class == MemoryBudgetClass::Frame) {
        return true;
    }

    if (budget_class == MemoryBudgetClass::Callback) {
        return true;
    }

    if (budget_class == MemoryBudgetClass::Job) {
        return true;
    }

    return false;
}
}
