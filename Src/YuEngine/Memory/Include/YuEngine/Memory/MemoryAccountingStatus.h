// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAccountingStatus.h

#pragma once

namespace yuengine::memory {
enum class MemoryAccountingStatus {
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
