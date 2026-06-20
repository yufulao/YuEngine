// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/UiSimplePopupToastRepresentativeController.cpp

#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeController.h"

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

std::uint32_t ReadStyleKey(const UiPanelOpenArgsSnapshot &snapshot) {
    if (snapshot.value_count == 0U) {
        return 0U;
    }

    return snapshot.values[0U];
}
}

const UiSimplePopupToastRepresentativeSnapshot &
UiSimplePopupToastRepresentativeController::GetRepresentativeSnapshot() const {
    return representative_snapshot_;
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnInitEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnBindEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnOpenEvent() {
    UiPanelOpenArgs open_args{};
    return OnOpenWithArgsEvent(open_args);
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnOpenWithArgsEvent(
    const UiPanelOpenArgs &open_args) {
    representative_snapshot_.open_args = CopyOpenArgs(open_args);
    representative_snapshot_.message_key = representative_snapshot_.open_args.request_key;
    representative_snapshot_.style_key = ReadStyleKey(representative_snapshot_.open_args);
    representative_snapshot_.visible = true;
    representative_snapshot_.displayed = true;
    representative_snapshot_.cleared = false;
    ++representative_snapshot_.open_display_count;
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnCloseEvent() {
    representative_snapshot_.visible = false;
    representative_snapshot_.displayed = false;
    ++representative_snapshot_.close_display_count;
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus UiSimplePopupToastRepresentativeController::OnClearEvent() {
    const std::uint32_t open_display_count = representative_snapshot_.open_display_count;
    const std::uint32_t close_display_count = representative_snapshot_.close_display_count;
    const std::uint32_t clear_display_count = representative_snapshot_.clear_display_count + 1U;
    representative_snapshot_ = UiSimplePopupToastRepresentativeSnapshot{};
    representative_snapshot_.open_display_count = open_display_count;
    representative_snapshot_.close_display_count = close_display_count;
    representative_snapshot_.clear_display_count = clear_display_count;
    representative_snapshot_.cleared = true;
    return BaseUiLifecycleStatus::Success;
}
}
