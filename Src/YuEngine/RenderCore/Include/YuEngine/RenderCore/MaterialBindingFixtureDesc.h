// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/MaterialBindingFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Configures bounded material binding fixture storage.
 */
struct MaterialBindingFixtureDesc final {
    std::size_t binding_record_capacity = MAX_MATERIAL_BINDING_FIXTURE_RECORDS;
};
}
