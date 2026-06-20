// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/IBaseUiCloseRequestSink.h

#pragma once

#include "YuEngine/UiRuntime/BaseUiLifecycleStatus.h"

namespace yuengine::uiruntime {
class BaseUiController;

class IBaseUiCloseRequestSink {
public:
    virtual ~IBaseUiCloseRequestSink() = default;

    /**
     * @comment 通过上层 manager/sink 执行 controller 自关闭请求。
     * @param controller 请求关闭自身的 BaseUiController。
     * @return 成功时返回 Success，否则返回明确生命周期状态。
     */
    virtual BaseUiLifecycleStatus RequestCloseSelf(BaseUiController *controller) = 0;
};
}
