// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 配置 固定容量 存储 用于 一个 RenderCore submission batch fixture.
 */
struct RenderSubmissionBatchFixtureDesc final {
    std::size_t submission_record_capacity = MAX_RENDER_SUBMISSION_BATCH_FIXTURE_RECORDS;
};
}
