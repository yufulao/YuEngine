// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreResult.h

#pragma once

#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreState.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"

namespace yuengine::world {
struct WorldComponentResourceBindingRestoreResult final {
    WorldComponentResourceBindingRestoreStatus status =
        WorldComponentResourceBindingRestoreStatus::Success;
    WorldComponentResourceBindingStatus binding_status =
        WorldComponentResourceBindingStatus::Success;
    yuengine::resource::ResourceStatus resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldComponentResourceBindingRestoreState state{};

    /**
     * @comment 创建成功 restore result。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldComponentResourceBindingRestoreResult Success(
        const WorldComponentResourceBindingRestoreState &state) {
        return WorldComponentResourceBindingRestoreResult{
            WorldComponentResourceBindingRestoreStatus::Success,
            WorldComponentResourceBindingStatus::Success,
            yuengine::resource::ResourceStatus::Success,
            state};
    }

    /**
     * @comment 创建失败 restore result。
     * @param status 输入 restore status。
     * @param binding_status 输入 binding bridge status。
     * @param resource_status 输入 resource registry status。
     * @return 显式操作结果。
     */
    static WorldComponentResourceBindingRestoreResult Failure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status=WorldComponentResourceBindingStatus::Success,
        yuengine::resource::ResourceStatus resource_status=yuengine::resource::ResourceStatus::Success) {
        return Failure(
            status,
            binding_status,
            resource_status,
            WorldComponentResourceBindingRestoreState{});
    }

    /**
     * @comment 创建带 operation state 的失败 restore result。
     * @param status 输入 restore status。
     * @param binding_status 输入 binding bridge status。
     * @param resource_status 输入 resource registry status。
     * @param state 输入 operation state。
     * @return 显式操作结果。
     */
    static WorldComponentResourceBindingRestoreResult Failure(
        WorldComponentResourceBindingRestoreStatus status,
        WorldComponentResourceBindingStatus binding_status,
        yuengine::resource::ResourceStatus resource_status,
        const WorldComponentResourceBindingRestoreState &state) {
        return WorldComponentResourceBindingRestoreResult{
            status,
            binding_status,
            resource_status,
            state};
    }

    /**
     * @comment 检查 result 是否成功。
     * @return result 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentResourceBindingRestoreStatus::Success;
    }
};
}
