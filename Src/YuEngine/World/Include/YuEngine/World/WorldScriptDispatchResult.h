// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldScriptDispatchResult.h

#pragma once

#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"

namespace yuengine::world {
struct WorldScriptDispatchResult final {
    WorldScriptDispatchStatus status = WorldScriptDispatchStatus::Success;
    WorldUpdatePhase phase = WorldUpdatePhase::BeginFrame;
    yuengine::script::ScriptCallId call_id{};

    /**
     * @comment 创建成功 result。
     * @param phase 输入 world update phase。
     * @param call_id 输入 script call id。
     * @return 显式操作结果。
     */
    static WorldScriptDispatchResult Success(WorldUpdatePhase phase,
        yuengine::script::ScriptCallId call_id) {
        return WorldScriptDispatchResult{WorldScriptDispatchStatus::Success, phase, call_id};
    }

    /**
     * @comment 创建失败 result。
     * @param status 输入 dispatch status。
     * @return 显式操作结果。
     */
    static WorldScriptDispatchResult Failure(WorldScriptDispatchStatus status) {
        return WorldScriptDispatchResult{status, WorldUpdatePhase::BeginFrame, yuengine::script::ScriptCallId{}};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldScriptDispatchStatus::Success;
    }
};
}
