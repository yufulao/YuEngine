#pragma once

namespace yuengine::memory {
enum class MEMORY_ACCOUNTING_STATUS {
    Success,
    ExplicitlyTrackedOnly,
    InvalidOwner,
    InvalidTag,
    InvalidSize,
    InvalidAlignment,
    InvalidBudgetClass,
    BudgetExceeded,
    CapacityExceeded,
    UnmatchedFree,
    OwnerTagMismatch
};
}
