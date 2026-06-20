// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiFullscreenRepresentativeController.cpp

#include "YuEngine/UiRuntime/UiFullscreenRepresentativeController.h"

#include "YuEngine/UiRuntime/UiPanelOpenArgsConstants.h"

namespace yuengine::uiruntime {
namespace {
std::uint32_t ClampOpenArgValueCount(std::uint32_t value_count) {
    if (value_count > MAX_UI_PANEL_OPEN_ARG_VALUE_COUNT) {
        return MAX_UI_PANEL_OPEN_ARG_VALUE_COUNT;
    }

    return value_count;
}

UiPanelOpenArgsSnapshot CopyOpenArgs(const UiPanelOpenArgs &open_args) {
    UiPanelOpenArgsSnapshot snapshot{};
    snapshot.request_key = open_args.request_key;
    snapshot.value_count = ClampOpenArgValueCount(open_args.value_count);
    snapshot.has_args = snapshot.request_key != 0U || snapshot.value_count > 0U;
    if (snapshot.value_count == 0U) {
        return snapshot;
    }

    if (open_args.values == nullptr) {
        snapshot.value_count = 0U;
        snapshot.has_args = snapshot.request_key != 0U;
        return snapshot;
    }

    for (std::uint32_t index = 0U; index < snapshot.value_count; ++index) {
        snapshot.values[index] = open_args.values[index];
    }

    return snapshot;
}

std::uint32_t ReadDisplayModeKey(const UiPanelOpenArgsSnapshot &snapshot) {
    if (snapshot.value_count == 0U) {
        return 0U;
    }

    return snapshot.values[0U];
}

std::uint32_t ReadFocusKey(const UiPanelOpenArgsSnapshot &snapshot) {
    if (snapshot.value_count < 2U) {
        return 0U;
    }

    return snapshot.values[1U];
}
}

const UiFullscreenRepresentativeSnapshot &
UiFullscreenRepresentativeController::GetRepresentativeSnapshot() const {
    return representative_snapshot_;
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnInitEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnBindEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnOpenEvent() {
    UiPanelOpenArgs open_args{};
    return OnOpenWithArgsEvent(open_args);
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnOpenWithArgsEvent(
    const UiPanelOpenArgs &open_args) {
    representative_snapshot_.open_args = CopyOpenArgs(open_args);
    representative_snapshot_.screen_key = representative_snapshot_.open_args.request_key;
    representative_snapshot_.display_mode_key = ReadDisplayModeKey(representative_snapshot_.open_args);
    representative_snapshot_.focus_key = ReadFocusKey(representative_snapshot_.open_args);
    representative_snapshot_.visible = true;
    representative_snapshot_.displayed = true;
    representative_snapshot_.top_active = true;
    representative_snapshot_.cleared = false;
    ++representative_snapshot_.open_display_count;
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnCloseEvent() {
    representative_snapshot_.visible = false;
    representative_snapshot_.displayed = false;
    representative_snapshot_.top_active = false;
    ++representative_snapshot_.close_display_count;
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiFullscreenRepresentativeController::OnClearEvent() {
    const std::uint32_t open_display_count = representative_snapshot_.open_display_count;
    const std::uint32_t close_display_count = representative_snapshot_.close_display_count;
    const std::uint32_t clear_display_count = representative_snapshot_.clear_display_count + 1U;
    representative_snapshot_ = UiFullscreenRepresentativeSnapshot{};
    representative_snapshot_.open_display_count = open_display_count;
    representative_snapshot_.close_display_count = close_display_count;
    representative_snapshot_.clear_display_count = clear_display_count;
    representative_snapshot_.cleared = true;
    return BaseUiLifecycleStatus::Success;
}
}
