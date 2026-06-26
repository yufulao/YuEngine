// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandList.h

#pragma once

#include <cstddef>
#include <vector>

#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandListSnapshot.h"
#include "YuEngine/Rhi/RhiCommandRecord.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
class RhiCommandList final {
public:
    /**
     * @comment 构造 RhiCommandList 实例。
     * @param capacity 输入 容量。
     */
    explicit RhiCommandList(std::size_t capacity);

    /**
     * @comment 重置 操作。
     * @return 显式操作状态。
     */
    RhiStatus Reset();
    /**
     * @comment 开始 command 记录ing 用于 一个 frame.
     * @param target 输入 target。
     * @return 显式操作状态。
     */
    RhiStatus BeginFrame(RhiTextureHandle target);
    /**
     * @comment 记录清屏。
     * @param target 输入 target。
     * @param color 输入颜色。
     * @return 显式操作状态。
     */
    RhiStatus RecordClear(RhiTextureHandle target, RhiColor color);
    /**
     * @comment 记录流水线绑定。
     * @param pipeline 输入 pipeline。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindPipeline(RhiPipelineHandle pipeline);
    /**
     * @comment 记录顶点缓冲绑定。
     * @param vertex_buffer 输入 vertex buffer 视图。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindVertexBuffer(const RhiVertexBufferView &vertex_buffer);
    /**
     * @comment 记录索引缓冲绑定。
     * @param index_buffer 输入 index buffer 视图。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindIndexBuffer(const RhiIndexBufferView &index_buffer);
    /**
     * @comment 记录采样纹理绑定。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindSampledTexture(const RhiSampledTextureBinding &binding);
    /**
     * @comment 记录采样器绑定。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindSampler(const RhiSamplerBinding &binding);
    /**
     * @comment 记录 constant buffer 绑定。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindConstantBuffer(const RhiConstantBufferBinding &binding);
    /**
     * @comment 记录 blend state 绑定。
     * @param desc 输入 blend state 描述。
     * @return 显式操作状态。
     */
    RhiStatus RecordBindBlendState(const RhiBlendStateDesc &desc);
    /**
     * @comment 记录绘制。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    RhiStatus RecordDraw(const RhiDrawDesc &desc);
    /**
     * @comment 记录索引绘制。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    RhiStatus RecordDrawIndexed(const RhiDrawIndexedDesc &desc);
    /**
     * @comment 结束 command 记录ing 用于 一个 frame.
     * @return 显式操作状态。
     */
    RhiStatus EndFrame();
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    RhiCommandListSnapshot Snapshot() const;
    /**
     * @comment 返回 command at 请求的 index。
     * @param index 输入 index。
     * @return 请求对象的引用。
     */
    const RhiCommandRecord& CommandAt(std::size_t index) const;
    /**
     * @comment 返回 记录ed target 句柄。
     * @return Target 句柄 值。
     */
    RhiTextureHandle TargetHandle() const;
    /**
     * @comment 返回 存储容量。
     * @return 容量 值。
     */
    std::size_t Capacity() const;
    /**
     * @comment 返回 记录ed command 计数。
     * @return Command 计数 值。
     */
    std::size_t CommandCount() const;
    /**
     * @comment 检查 command 记录ing is 完成.
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool IsComplete() const;

private:
    RhiStatus Append(RhiCommandRecord record);

    std::vector<RhiCommandRecord> records_;
    RhiTextureHandle target_handle_;
    std::size_t command_count_;
    std::size_t draw_command_count_;
    std::size_t indexed_draw_command_count_;
    std::size_t sampled_texture_bind_command_count_;
    std::size_t sampler_bind_command_count_;
    std::size_t constant_buffer_bind_command_count_;
    std::size_t blend_state_bind_command_count_;
    bool is_recording_;
    bool is_complete_;
};
}
