// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryBudgetClass.h

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
 * @comment Returns the memory budget class index.
 * @param budget_class Input budget class.
 * @return Memory budget class index value.
 */
inline std::size_t MemoryBudgetClassIndex(MemoryBudgetClass budget_class) {
    return static_cast<std::size_t>(budget_class);
}

/**
 * @comment Checks whether the memory budget class is valid.
 * @param budget_class Input budget class.
 * @return True when the condition is satisfied; false otherwise.
 */
inline bool IsValidMemoryBudgetClass(MemoryBudgetClass budget_class) {
    return MemoryBudgetClassIndex(budget_class) < MEMORY_BUDGET_CLASS_COUNT;
}

/**
 * @comment Checks whether the memory budget class is hot-path.
 * @param budget_class Input budget class.
 * @return True when the condition is satisfied; false otherwise.
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
