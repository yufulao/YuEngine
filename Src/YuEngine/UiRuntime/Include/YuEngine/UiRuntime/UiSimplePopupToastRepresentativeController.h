// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiSimplePopupToastRepresentativeController.h

#pragma once

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"
#include "YuEngine/UiRuntime/UiSimplePopupToastRepresentativeSnapshot.h"

namespace yuengine::uiruntime {
class UiSimplePopupToastRepresentativeController final : public BaseUiController {
public:
    /**
     * @comment 获取 simple popup/toast 代表窗口的显示快照。
     * @return 当前代表窗口显示状态。
     */
    const UiSimplePopupToastRepresentativeSnapshot &GetRepresentativeSnapshot() const;

protected:
    BaseUiLifecycleStatus OnInitEvent() override;
    BaseUiLifecycleStatus OnBindEvent() override;
    BaseUiLifecycleStatus OnOpenEvent() override;
    BaseUiLifecycleStatus OnOpenWithArgsEvent(const UiPanelOpenArgs &open_args) override;
    BaseUiLifecycleStatus OnCloseEvent() override;
    BaseUiLifecycleStatus OnClearEvent() override;

private:
    UiSimplePopupToastRepresentativeSnapshot representative_snapshot_;
};
}
