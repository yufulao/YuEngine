// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetConstants.h

#pragma once

#include <cstdint>

namespace yuengine::asset {
constexpr std::uint32_t MAX_ASSET_COUNT = 64U;
constexpr std::uint32_t MAX_ASSET_TYPE_COUNT = 16U;
constexpr std::uint32_t MAX_ASSET_DEPENDENCY_EDGE_COUNT = 128U;
constexpr std::uint32_t INVALID_ASSET_SLOT = 0xFFFFFFFFU;
constexpr std::uint32_t INVALID_ASSET_GENERATION = 0U;
}
