#pragma once

#include <cstddef>

namespace yuengine::memory {
enum class MEMORY_BUDGET_CLASS {
    Setup = 0,
    Load = 1,
    Frame = 2,
    Callback = 3,
    Job = 4,
    Tool = 5
};

inline constexpr std::size_t MemoryBudgetClassCount = 6U;

inline std::size_t MemoryBudgetClassIndex(MEMORY_BUDGET_CLASS budgetClass) {
    return static_cast<std::size_t>(budgetClass);
}

inline bool IsValidMemoryBudgetClass(MEMORY_BUDGET_CLASS budgetClass) {
    return MemoryBudgetClassIndex(budgetClass) < MemoryBudgetClassCount;
}

inline bool IsHotMemoryBudgetClass(MEMORY_BUDGET_CLASS budgetClass) {
    if (budgetClass == MEMORY_BUDGET_CLASS::Frame) {
        return true;
    }

    if (budgetClass == MEMORY_BUDGET_CLASS::Callback) {
        return true;
    }

    if (budgetClass == MEMORY_BUDGET_CLASS::Job) {
        return true;
    }

    return false;
}
}
