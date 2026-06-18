/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadOperation.h
 * @brief Resource module decoded payload operation value contract.
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief Identifies the last operation applied to decoded payload storage.
 */
enum class ResourceDecodedPayloadOperation {
    None,
    ConfigureBudget,
    Store,
    Query,
    Read,
    Release
};

} // namespace yuengine::resource
