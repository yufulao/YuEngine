/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultClass.h
 * @brief Resource 模块 decode 结果 class 值契约。
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief 声明一个已提交 decode 结果使用的 import-ready class。
 */
enum class ResourceDecodeResultClass {
    Unknown,
    Texture,
    Audio,
    Mesh,
    Material
};

} // namespace yuengine::resource
