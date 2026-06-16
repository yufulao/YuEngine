// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandList.h

#pragma once

#include <cstddef>
#include <vector>

#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandListSnapshot.h"
#include "YuEngine/Rhi/RhiCommandRecord.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
class RhiCommandList final {
public:
    /**
     * @comment Constructs a RhiCommandList instance.
     * @param capacity Input capacity.
     */
    explicit RhiCommandList(std::size_t capacity);

    /**
     * @comment Resets the operation.
     * @return Explicit operation status.
     */
    RhiStatus Reset();
    /**
     * @comment Begins command recording for a frame.
     * @param target Input target.
     * @return Explicit operation status.
     */
    RhiStatus BeginFrame(RhiTextureHandle target);
    /**
     * @comment Records clear.
     * @param target Input target.
     * @param color Input color.
     * @return Explicit operation status.
     */
    RhiStatus RecordClear(RhiTextureHandle target, RhiColor color);
    /**
     * @comment Records pipeline binding.
     * @param pipeline Input pipeline.
     * @return Explicit operation status.
     */
    RhiStatus RecordBindPipeline(RhiPipelineHandle pipeline);
    /**
     * @comment Records vertex buffer binding.
     * @param vertex_buffer Input vertex buffer view.
     * @return Explicit operation status.
     */
    RhiStatus RecordBindVertexBuffer(const RhiVertexBufferView &vertex_buffer);
    /**
     * @comment Records draw.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    RhiStatus RecordDraw(const RhiDrawDesc &desc);
    /**
     * @comment Ends command recording for a frame.
     * @return Explicit operation status.
     */
    RhiStatus EndFrame();
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    RhiCommandListSnapshot Snapshot() const;
    /**
     * @comment Returns the command at the requested index.
     * @param index Input index.
     * @return Reference to the requested object.
     */
    const RhiCommandRecord& CommandAt(std::size_t index) const;
    /**
     * @comment Returns the recorded target handle.
     * @return Target handle value.
     */
    RhiTextureHandle TargetHandle() const;
    /**
     * @comment Returns the storage capacity.
     * @return Capacity value.
     */
    std::size_t Capacity() const;
    /**
     * @comment Returns the recorded command count.
     * @return Command count value.
     */
    std::size_t CommandCount() const;
    /**
     * @comment Checks whether command recording is complete.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsComplete() const;

private:
    RhiStatus Append(RhiCommandRecord record);

    std::vector<RhiCommandRecord> records_;
    RhiTextureHandle target_handle_;
    std::size_t command_count_;
    std::size_t draw_command_count_;
    bool is_recording_;
    bool is_complete_;
};
}
