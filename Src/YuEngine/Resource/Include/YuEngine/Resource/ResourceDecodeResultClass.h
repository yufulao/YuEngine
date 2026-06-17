/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultClass.h
 * @brief Resource module decode result class value contract.
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief Declares the import-ready class for a committed decode result.
 */
enum class ResourceDecodeResultClass {
    Unknown,
    Texture,
    Audio,
    Mesh,
    Material
};

} // namespace yuengine::resource
