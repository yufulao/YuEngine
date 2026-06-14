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

inline std::size_t MemoryBudgetClassIndex(MemoryBudgetClass budgetClass) {
    return static_cast<std::size_t>(budgetClass);
}

inline bool IsValidMemoryBudgetClass(MemoryBudgetClass budgetClass) {
    return MemoryBudgetClassIndex(budgetClass) < MemoryBudgetClassCount;
}

inline bool IsHotMemoryBudgetClass(MemoryBudgetClass budgetClass) {
    if (budgetClass == MemoryBudgetClass::Frame) {
        return true;
    }

    if (budgetClass == MemoryBudgetClass::Callback) {
        return true;
    }

    if (budgetClass == MemoryBudgetClass::Job) {
        return true;
    }

    return false;
}
}
