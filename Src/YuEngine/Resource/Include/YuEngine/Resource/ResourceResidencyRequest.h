// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyRequest.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceResidencyRequest final {
    ResourceHandle resource;
    ResourceTypeId expected_type;
};
}
