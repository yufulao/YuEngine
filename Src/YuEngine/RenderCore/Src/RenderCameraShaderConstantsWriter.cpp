// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Src/RenderCameraShaderConstantsWriter.cpp

#include "YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h"

#include <cmath>

namespace yuengine::rendercore {
namespace {
bool IsFrameFinite(const RenderCameraFrame &frame) {
    for (float value : frame.view_projection.values) {
        if (!std::isfinite(value)) {
            return false;
        }
    }

    return true;
}
}

RenderCameraShaderConstantsStatus RenderCameraShaderConstantsWriter::WriteViewProjection(
    const RenderCameraFrame &frame,
    RenderCameraShaderConstants *out_constants) {
    if (out_constants == nullptr) {
        RecordStatus(RenderCameraShaderConstantsStatus::InvalidArgument);
        return RenderCameraShaderConstantsStatus::InvalidArgument;
    }

    if (!IsFrameFinite(frame)) {
        RecordStatus(RenderCameraShaderConstantsStatus::InvalidFrame);
        return RenderCameraShaderConstantsStatus::InvalidFrame;
    }

    out_constants->view_projection_values = frame.view_projection.values;
    RecordStatus(RenderCameraShaderConstantsStatus::Success);
    return RenderCameraShaderConstantsStatus::Success;
}

RenderCameraShaderConstantsSnapshot RenderCameraShaderConstantsWriter::Snapshot() const {
    return snapshot_;
}

void RenderCameraShaderConstantsWriter::Reset() {
    snapshot_ = {};
}

void RenderCameraShaderConstantsWriter::RecordStatus(RenderCameraShaderConstantsStatus status) {
    snapshot_.last_status = status;
    if (status == RenderCameraShaderConstantsStatus::Success) {
        ++snapshot_.accepted_write_count;
        return;
    }

    ++snapshot_.rejected_write_count;
}
}
