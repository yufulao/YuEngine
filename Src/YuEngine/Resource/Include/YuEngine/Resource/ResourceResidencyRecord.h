// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyRecord.h

#pragma once

#include "YuEngine/Resource/ResourceResidencyOperation.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"

namespace yuengine::resource {
struct ResourceResidencyRecord final {
    ResourceResidencyOperation operation = ResourceResidencyOperation::None;
    ResourceResidencyRequest request;
    ResourceResidencyStatus status = ResourceResidencyStatus::Success;
    ResourceResidencyState state = ResourceResidencyState::Unloaded;
    bool is_active = false;
};
}
