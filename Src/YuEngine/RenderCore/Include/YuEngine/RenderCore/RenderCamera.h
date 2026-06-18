// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCamera.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderCore/RenderCameraProjectionDesc.h"
#include "YuEngine/RenderCore/RenderCameraSnapshot.h"
#include "YuEngine/RenderCore/RenderCameraStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 为 RenderCore 帧契约构建后端无关的相机矩阵。
 */
class RenderCamera final {
public:
    /**
     * @comment 构建 视图，projection，和 视图-projection matrices.
     * @param pose 输入 camera pose。
     * @param projection 输入 projection 描述。
     * @param out_frame 调用方持有的 output frame。
     * @return 显式 build 状态。
     */
    RenderCameraStatus BuildFrame(
        const RenderCameraPose &pose,
        const RenderCameraProjectionDesc &projection,
        RenderCameraFrame *out_frame);

    /**
     * @comment 返回 固定容量 camera build 计数器。
     * @return 快照值。
     */
    RenderCameraSnapshot Snapshot() const;

    /**
     * @comment 重置固定容量 camera build 计数。
     */
    void Reset();

private:
    void RecordStatus(RenderCameraStatus status);

    RenderCameraSnapshot snapshot_{};
};
}
