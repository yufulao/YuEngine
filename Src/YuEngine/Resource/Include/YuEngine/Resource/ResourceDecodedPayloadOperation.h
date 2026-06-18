/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadOperation.h
 * @brief Resource 模块解码载荷操作值契约。
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief 标识应用到 decoded payload 存储的最后操作。
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
