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

inline constexpr std::size_t MemoryBudgetClassCount = 6U;

inline std::size_t MemoryBudgetClassIndex(MemoryBudgetClass budget_class) {
    return static_cast<std::size_t>(budget_class);
}

inline bool IsValidMemoryBudgetClass(MemoryBudgetClass budget_class) {
    return MemoryBudgetClassIndex(budget_class) < MemoryBudgetClassCount;
}

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
