// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/BaseUiController.h

#pragma once

#include "YuEngine/UiRuntime/BaseUiLifecycleSnapshot.h"
#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"
#include "YuEngine/UiRuntime/UiPanelOpenArgs.h"

namespace yuengine::uiruntime {
class IBaseUiCloseRequestSink;

class BaseUiController {
public:
    virtual ~BaseUiController() = default;

    /**
     * @comment 初始化 controller，并且只执行一次 BindEvent 语义。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus Initialize();

    /**
     * @comment 激活 controller；首次激活会先执行一次 Initialize。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus Open();

    /**
     * @comment 携带 open 参数激活 controller；首次激活会先执行一次 Initialize。
     * @param open_args 调用方提供的 open 参数视图，仅在本次调用期间有效。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus OpenWithArgs(const UiPanelOpenArgs &open_args);

    /**
     * @comment 反激活 controller。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus Close();

    /**
     * @comment 销毁 controller，并且只执行一次 Clear 语义。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus Destroy();

    /**
     * @comment 通过外部 close request sink 请求关闭自身。
     * @param close_request_sink 负责执行关闭请求的上层对象。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    BaseUiLifecycleStatus RequestCloseSelf(IBaseUiCloseRequestSink *close_request_sink);

    /**
     * @comment 获取 controller 生命周期快照。
     * @return 当前生命周期快照。
     */
    BaseUiLifecycleSnapshot Snapshot() const;

protected:
    virtual BaseUiLifecycleStatus OnInitEvent();
    virtual BaseUiLifecycleStatus OnBindEvent();
    virtual BaseUiLifecycleStatus OnOpenEvent();
    virtual BaseUiLifecycleStatus OnOpenWithArgsEvent(const UiPanelOpenArgs &open_args);
    virtual BaseUiLifecycleStatus OnCloseEvent();
    virtual BaseUiLifecycleStatus OnClearEvent();

private:
    BaseUiLifecycleStatus SetLastStatus(BaseUiLifecycleStatus status);

    BaseUiLifecycleSnapshot snapshot_;
};
}
