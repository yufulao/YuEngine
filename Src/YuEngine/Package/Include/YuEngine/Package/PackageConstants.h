#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::package {
constexpr std::uint32_t MAX_PACKAGE_MANIFEST_COUNT = 4U;
constexpr std::uint32_t MAX_PACKAGE_ENTRY_COUNT = 32U;
constexpr std::uint32_t MAX_PACKAGE_DEPENDENCY_EDGE_COUNT = 64U;
constexpr std::uint32_t MAX_LOAD_PLAN_RECORD_COUNT = 32U;
constexpr std::size_t MAX_PACKAGE_SOURCE_KEY_BYTES = 128U;
constexpr std::uint32_t MAX_DECLARED_ENTRY_SIZE = 4096U;
}
