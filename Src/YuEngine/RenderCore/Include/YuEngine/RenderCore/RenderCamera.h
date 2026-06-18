// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCamera.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderCore/RenderCameraProjectionDesc.h"
#include "YuEngine/RenderCore/RenderCameraSnapshot.h"
#include "YuEngine/RenderCore/RenderCameraStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Builds backend-neutral camera matrices for RenderCore frame contracts.
 */
class RenderCamera final {
public:
    /**
     * @comment Builds view, projection, and view-projection matrices.
     * @param pose Input camera pose.
     * @param projection Input projection descriptor.
     * @param out_frame Caller-owned output frame.
     * @return Explicit build status.
     */
    RenderCameraStatus BuildFrame(
        const RenderCameraPose &pose,
        const RenderCameraProjectionDesc &projection,
        RenderCameraFrame *out_frame);

    /**
     * @comment Returns bounded camera build counters.
     * @return Snapshot value.
     */
    RenderCameraSnapshot Snapshot() const;

    /**
     * @comment Resets bounded camera build counters.
     */
    void Reset();

private:
    void RecordStatus(RenderCameraStatus status);

    RenderCameraSnapshot snapshot_{};
};
}
