// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiHitTestResolver.h

#pragma once

#include "YuEngine/UiCore/UiHitTestRequest.h"
#include "YuEngine/UiCore/UiHitTestResult.h"
#include "YuEngine/UiCore/UiNodeTree.h"

namespace yuengine::uicore {
class UiHitTestResolver final {
public:
    /**
     * @comment 解析 UI hit-test path。
     * @param tree 输入 node tree。
     * @param request 输入 hit-test request。
     * @return hit-test result。
     */
    static UiHitTestResult Resolve(const UiNodeTree &tree, const UiHitTestRequest &request);
};
}
