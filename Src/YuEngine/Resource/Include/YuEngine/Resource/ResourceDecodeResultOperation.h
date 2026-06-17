/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultOperation.h
 * @brief Resource module decode result operation value contract.
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief Identifies the last operation applied to decode-result metadata.
 */
enum class ResourceDecodeResultOperation {
    None,
    ConfigureBudget,
    Commit,
    Query,
    Release
};

} // namespace yuengine::resource
