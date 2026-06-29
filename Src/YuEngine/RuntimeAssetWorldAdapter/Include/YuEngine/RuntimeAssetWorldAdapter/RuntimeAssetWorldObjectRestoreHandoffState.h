// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffState.h

#pragma once

#include <cstdint>

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectRestoreHandoffState final {
    std::uint32_t input_mapping_count = 0U;
    std::uint32_t output_identity_count = 0U;
    std::uint32_t output_transform_count = 0U;
    std::uint32_t proof_record_count = 0U;
    std::uint32_t slice_record_count = 0U;
    std::uint32_t gate_record_count = 0U;
    std::uint32_t restored_identity_count = 0U;
    std::uint32_t restored_transform_count = 0U;
};
}
