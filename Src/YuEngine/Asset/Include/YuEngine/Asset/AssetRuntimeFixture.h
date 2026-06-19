// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRuntimeFixture.h

#pragma once

#include "YuEngine/Asset/AssetRuntimeFixtureRequest.h"
#include "YuEngine/Asset/AssetRuntimeFixtureResult.h"
#include "YuEngine/Asset/AssetRuntimeFixtureSnapshot.h"
#include "YuEngine/Asset/AssetRuntimeFixtureStatus.h"

namespace yuengine::asset {
class AssetRuntimeFixture final {
public:
    /**
     * @comment 构造 Asset runtime fixture 执行器。
     */
    AssetRuntimeFixture();

    /**
     * @comment 执行 caller-owned synthetic asset fixture 闭环。
     * @param request 输入 fixture 请求。
     * @return 显式 fixture 结果。
     */
    AssetRuntimeFixtureResult Execute(const AssetRuntimeFixtureRequest &request);
    /**
     * @comment 返回 fixture 执行快照。
     * @return 快照值。
     */
    AssetRuntimeFixtureSnapshot Snapshot() const;

private:
    AssetRuntimeFixtureStatus ValidateRequest(const AssetRuntimeFixtureRequest &request) const;
    AssetRuntimeFixtureResult RecordRejected(AssetRuntimeFixtureResult result);
    AssetRuntimeFixtureResult RecordCompleted(AssetRuntimeFixtureResult result);

    AssetRuntimeFixtureSnapshot snapshot_;
};
}
