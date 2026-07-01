// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h

#pragma once

#include <cstdint>

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterState.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterResult final {
    RuntimeAssetWorldObjectAdapterStatus status = RuntimeAssetWorldObjectAdapterStatus::Success;
    RuntimeAssetWorldObjectAdapterState state{};
    std::uint32_t failed_mapping_index = 0U;
    std::uint64_t failed_target_id = 0U;
    std::uint32_t required_identity_output_count = 0U;
    std::uint32_t required_transform_output_count = 0U;

    /**
     * @comment 创建成功结果。
     * @param state 输出记录状态。
     * @return 成功结果。
     */
    static RuntimeAssetWorldObjectAdapterResult Success(RuntimeAssetWorldObjectAdapterState state);

    /**
     * @comment 创建失败结果。
     * @param status 显式失败状态。
     * @return 失败结果。
     */
    static RuntimeAssetWorldObjectAdapterResult Failure(RuntimeAssetWorldObjectAdapterStatus status);

    /**
     * @comment 创建带定位信息的失败结果。
     * @param status 显式失败状态。
     * @param failed_mapping_index 失败 mapping 索引。
     * @param failed_target_id 失败 target id。
     * @return 失败结果。
     */
    static RuntimeAssetWorldObjectAdapterResult Failure(
        RuntimeAssetWorldObjectAdapterStatus status,
        std::uint32_t failed_mapping_index,
        std::uint64_t failed_target_id);

    /**
     * @comment 检查结果是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const;
};
}
