// 模块: YuEngine Diagnostics
// 文件: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHook.h

#pragma once

#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookProposal.h"
#include "YuEngine/Diagnostics/RuntimeDiagnosticsOverlayHookResult.h"

namespace yuengine::diagnostics {
class RuntimeDiagnosticsOverlayHook final {
public:
    /**
     * @comment 验证 debug overlay hook proposal，只允许 optional tooling plane。
     * @param proposal 输入 hook proposal。
     * @return 显式验证结果。
     */
    RuntimeDiagnosticsOverlayHookResult ValidateProposal(
        const RuntimeDiagnosticsOverlayHookProposal &proposal) const;
};
}
