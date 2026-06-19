// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneContractQueue.h

#pragma once

#include <cstddef>
#include <span>

#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneEntityRecord.h"
#include "YuEngine/RenderScene/RenderSceneSnapshot.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/RenderScene/RenderSceneSubmitRequest.h"
#include "YuEngine/RenderScene/RenderSceneSubmitResult.h"

namespace yuengine::renderscene {
class RenderSceneContractQueue final {
public:
    /**
     * @comment 将 L1 render scene 值记录转换为 RenderCore view packet 请求。
     * @param request 输入 scene submit 请求。
     * @param out_packets 调用方持有的输出 packet storage。
     * @param out_result 输出 submit 结果。
     * @return 显式操作状态。
     */
    RenderSceneStatus BuildRenderCorePackets(
        const RenderSceneSubmitRequest &request,
        std::span<yuengine::rendercore::RenderViewPacketRequest> out_packets,
        RenderSceneSubmitResult *out_result);

    /**
     * @comment 返回当前 contract queue 快照。
     * @return 快照值。
     */
    RenderSceneSnapshot Snapshot() const;

private:
    RenderSceneStatus ValidateRequest(
        const RenderSceneSubmitRequest &request,
        std::span<yuengine::rendercore::RenderViewPacketRequest> out_packets,
        const RenderSceneCameraRecord **out_camera,
        std::size_t *out_visible_entity_count) const;
    RenderSceneStatus ValidateEntity(const RenderSceneEntityRecord &entity) const;
    const RenderSceneCameraRecord *FindCamera(const RenderSceneSubmitRequest &request) const;
    bool IsActiveEntity(const RenderSceneEntityRecord &entity) const;
    void FillPacket(
        const RenderSceneSubmitRequest &request,
        const RenderSceneCameraRecord &camera,
        const RenderSceneEntityRecord &entity,
        yuengine::rendercore::RenderViewPacketRequest *out_packet) const;
    RenderSceneStatus RecordSuccess(const RenderSceneSubmitResult &result);
    RenderSceneStatus RecordFailure(RenderSceneStatus status, const RenderSceneSubmitResult &result);

    RenderSceneSnapshot snapshot_{};
};
}
