// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Include/YuEngine/UiRenderCoreBridge/UiRenderCoreBridge.h

#pragma once

#include <cstddef>

#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeRequest.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeResult.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeSnapshot.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

namespace yuengine::uirendercorebridge {
/**
 * @comment 将 UiCore draw-element IR 通过 RenderCore fixture 提交流程验证。
 */
class UiRenderCoreBridge final {
public:
    /**
     * @comment 提交 draw elements 到调用方持有的 RenderCore fixture batch。
     * @param request 输入 bridge 请求。
     * @return bridge 提交结果。
     */
    UiRenderCoreBridgeResult Submit(const UiRenderCoreBridgeRequest &request);

    /**
     * @comment 返回 bridge 运行快照。
     * @return 快照值。
     */
    UiRenderCoreBridgeSnapshot Snapshot() const;

    /**
     * @comment 清空 bridge 计数器。
     */
    void Reset();

private:
    UiRenderCoreBridgeStatus ValidateRequest(
        const UiRenderCoreBridgeRequest &request,
        UiRenderCoreBridgeResult *result) const;
    UiRenderCoreBridgeStatus ValidateDrawElement(
        const yuengine::uicore::UiDrawElement &element,
        std::size_t index,
        UiRenderCoreBridgeResult *result) const;
    UiRenderCoreBridgeStatus FillPassRequests(
        const UiRenderCoreBridgeRequest &request,
        UiRenderCoreBridgeResult *result) const;
    void FillPassRequest(
        const UiRenderCoreBridgeRequest &request,
        const yuengine::uicore::UiDrawElement &element,
        std::size_t index) const;
    void RecordRejectedResult(const UiRenderCoreBridgeResult &result);
    void RecordSubmittedResult(const UiRenderCoreBridgeResult &result);

    UiRenderCoreBridgeSnapshot snapshot_;
};
}
