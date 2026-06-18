// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsSnapshot.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 写入 紧凑 shader constants 从 一个 已验证 camera frame。
 */
class RenderCameraShaderConstantsWriter final {
public:
    /**
     * @comment 写入 camera 视图-projection matrix 写入 一个 紧凑 shader constant block。
     * @param frame 输入 camera frame。
     * @param out_constants 调用方持有的 output constant block。
     * @return 显式操作状态。
     */
    RenderCameraShaderConstantsStatus WriteViewProjection(
        const RenderCameraFrame &frame,
        RenderCameraShaderConstants *out_constants);

    /**
     * @comment 返回当前 camera shader constant writer 快照。
     * @return 快照值。
     */
    RenderCameraShaderConstantsSnapshot Snapshot() const;

    /**
     * @comment 重置 writer 计数器。
     */
    void Reset();

private:
    void RecordStatus(RenderCameraShaderConstantsStatus status);

    RenderCameraShaderConstantsSnapshot snapshot_;
};
}
