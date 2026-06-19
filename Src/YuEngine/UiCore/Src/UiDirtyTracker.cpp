// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiDirtyTracker.cpp

#include "YuEngine/UiCore/UiDirtyTracker.h"

namespace yuengine::uicore {
UiDirtyState UiDirtyTracker::ApplyChange(UiDirtyChangeType change_type) {
    switch (change_type) {
        case UiDirtyChangeType::Layout:
        {
            Mark(UI_DIRTY_LAYOUT | UI_DIRTY_TRANSFORM | UI_DIRTY_HIT_TEST | UI_DIRTY_PAINT);
            ++state_.layout_rebuild_count;
            ++state_.hit_test_rebuild_count;
            break;
        }
        case UiDirtyChangeType::PaintOnly:
        {
            Mark(UI_DIRTY_PAINT);
            ++state_.paint_change_count;
            break;
        }
        case UiDirtyChangeType::Transform:
        {
            Mark(UI_DIRTY_TRANSFORM | UI_DIRTY_HIT_TEST | UI_DIRTY_PAINT);
            ++state_.hit_test_rebuild_count;
            break;
        }
        case UiDirtyChangeType::HitTest:
        {
            Mark(UI_DIRTY_HIT_TEST);
            ++state_.hit_test_rebuild_count;
            break;
        }
        case UiDirtyChangeType::Text:
        {
            Mark(UI_DIRTY_TEXT | UI_DIRTY_PAINT);
            ++state_.paint_change_count;
            break;
        }
        case UiDirtyChangeType::HoverState:
        {
            Mark(UI_DIRTY_PAINT);
            ++state_.paint_change_count;
            break;
        }
        case UiDirtyChangeType::ScrollOffset:
        {
            Mark(UI_DIRTY_TRANSFORM | UI_DIRTY_HIT_TEST | UI_DIRTY_PAINT);
            ++state_.hit_test_rebuild_count;
            ++state_.paint_change_count;
            break;
        }
        case UiDirtyChangeType::AtlasPage:
        {
            Mark(UI_DIRTY_PAINT);
            ++state_.paint_change_count;
            break;
        }
        default:
        {
            break;
        }
    }

    return state_;
}

void UiDirtyTracker::Clear() {
    state_ = UiDirtyState{};
}

UiDirtyState UiDirtyTracker::Snapshot() const {
    return state_;
}

void UiDirtyTracker::Mark(std::uint32_t domains) {
    state_.domains |= domains;
}
}
