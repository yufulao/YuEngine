// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h

#pragma once

#include "YuEngine/RenderCore/RenderCameraFrame.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsSnapshot.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Writes compact shader constants from a validated camera frame.
 */
class RenderCameraShaderConstantsWriter final {
public:
    /**
     * @comment Writes the camera view-projection matrix into a compact shader constant block.
     * @param frame Input camera frame.
     * @param out_constants Caller-owned output constant block.
     * @return Explicit operation status.
     */
    RenderCameraShaderConstantsStatus WriteViewProjection(
        const RenderCameraFrame &frame,
        RenderCameraShaderConstants *out_constants);

    /**
     * @comment Returns the current camera shader constant writer snapshot.
     * @return Snapshot value.
     */
    RenderCameraShaderConstantsSnapshot Snapshot() const;

    /**
     * @comment Resets writer counters.
     */
    void Reset();

private:
    void RecordStatus(RenderCameraShaderConstantsStatus status);

    RenderCameraShaderConstantsSnapshot snapshot_;
};
}
