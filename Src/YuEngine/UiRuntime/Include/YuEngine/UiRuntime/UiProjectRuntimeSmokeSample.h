// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiProjectRuntimeSmokeSample.h

#pragma once

#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleResult.h"
#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleStatus.h"

namespace yuengine::uiruntime {
class UiProjectRuntimeSmokeSample final {
public:
    /**
     * @comment 运行 Project UI Runtime smoke sample，依次验证 popup、fullscreen 与 GridView 代表窗口。
     * @param out_result 输出聚合 smoke 结果。
     * @return 成功或明确失败状态。
     */
    UiProjectRuntimeSmokeSampleStatus Run(UiProjectRuntimeSmokeSampleResult *out_result);
};
}
