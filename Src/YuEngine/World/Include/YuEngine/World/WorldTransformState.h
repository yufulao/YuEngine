// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldTransformState.h

#pragma once

namespace yuengine::world {
struct WorldTransformState final {
    float translation_x = 0.0F;
    float translation_y = 0.0F;
    float translation_z = 0.0F;
    float rotation_x = 0.0F;
    float rotation_y = 0.0F;
    float rotation_z = 0.0F;
    float rotation_w = 1.0F;
    float scale_x = 1.0F;
    float scale_y = 1.0F;
    float scale_z = 1.0F;
};
}
