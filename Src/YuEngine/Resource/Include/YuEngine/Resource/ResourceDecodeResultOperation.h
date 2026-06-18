/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultOperation.h
 * @brief Resource 模块 decode 结果 操作 值契约。
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief 标识应用到 decode 结果 metadata 的最后操作。
 */
enum class ResourceDecodeResultOperation {
    None,
    ConfigureBudget,
    Commit,
    Query,
    Release
};

} // namespace yuengine::resource
