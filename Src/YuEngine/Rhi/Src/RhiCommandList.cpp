// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Src/RhiCommandList.cpp

#include "YuEngine/Rhi/RhiCommandList.h"

namespace yuengine::rhi {
RhiCommandList::RhiCommandList(std::size_t capacity)
    : records_(capacity),
      target_handle_{},
      command_count_(0U),
      required_command_count_(1U),
      failed_command_index_(0U),
      failed_command_type_(RhiCommandType::BeginFrame),
      last_command_capacity_entry_type_(RhiCommandType::BeginFrame),
      last_command_capacity_entry_capacity_(0U),
      last_command_capacity_entry_command_count_(0U),
      last_command_capacity_entry_required_count_(0U),
      draw_command_count_(0U),
      indexed_draw_command_count_(0U),
      sampled_texture_bind_command_count_(0U),
      sampler_bind_command_count_(0U),
      constant_buffer_bind_command_count_(0U),
      blend_state_bind_command_count_(0U),
      last_status_(RhiStatus::Success),
      is_recording_(false),
      is_complete_(false) {
}

RhiStatus RhiCommandList::Reset() {
    target_handle_ = RhiTextureHandle{};
    command_count_ = 0U;
    required_command_count_ = 1U;
    draw_command_count_ = 0U;
    indexed_draw_command_count_ = 0U;
    sampled_texture_bind_command_count_ = 0U;
    sampler_bind_command_count_ = 0U;
    constant_buffer_bind_command_count_ = 0U;
    blend_state_bind_command_count_ = 0U;
    is_recording_ = false;
    is_complete_ = false;
    return RecordSuccess();
}

RhiStatus RhiCommandList::BeginFrame(RhiTextureHandle target) {
    if (is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    const RhiStatus append_status = Append(RhiCommandRecord{RhiCommandType::BeginFrame, target, RhiColor{}});
    if (append_status != RhiStatus::Success) {
        return append_status;
    }

    target_handle_ = target;
    is_recording_ = true;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordClear(RhiTextureHandle target, RhiColor color) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    return Append(RhiCommandRecord{RhiCommandType::ClearColor, target, color});
}

RhiStatus RhiCommandList::RecordBindPipeline(RhiPipelineHandle pipeline) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindPipeline;
    record.target = target_handle_;
    record.pipeline = pipeline;
    return Append(record);
}

RhiStatus RhiCommandList::RecordBindVertexBuffer(const RhiVertexBufferView &vertex_buffer) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindVertexBuffer;
    record.target = target_handle_;
    record.vertex_buffer = vertex_buffer;
    return Append(record);
}

RhiStatus RhiCommandList::RecordBindIndexBuffer(const RhiIndexBufferView &index_buffer) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindIndexBuffer;
    record.target = target_handle_;
    record.index_buffer = index_buffer;
    return Append(record);
}

RhiStatus RhiCommandList::RecordBindSampledTexture(const RhiSampledTextureBinding &binding) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindSampledTexture;
    record.target = target_handle_;
    record.sampled_texture = binding;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++sampled_texture_bind_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordBindSampler(const RhiSamplerBinding &binding) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindSampler;
    record.target = target_handle_;
    record.sampler = binding;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++sampler_bind_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordBindConstantBuffer(const RhiConstantBufferBinding &binding) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindConstantBuffer;
    record.target = target_handle_;
    record.constant_buffer = binding;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++constant_buffer_bind_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordBindBlendState(const RhiBlendStateDesc &desc) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindBlendState;
    record.target = target_handle_;
    record.blend_state = desc;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++blend_state_bind_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordDraw(const RhiDrawDesc &desc) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::Draw;
    record.target = target_handle_;
    record.draw = desc;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++draw_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::RecordDrawIndexed(const RhiDrawIndexedDesc &desc) {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::DrawIndexed;
    record.target = target_handle_;
    record.draw_indexed = desc;
    const RhiStatus status = Append(record);
    if (status != RhiStatus::Success) {
        return status;
    }

    ++indexed_draw_command_count_;
    return RecordSuccess();
}

RhiStatus RhiCommandList::EndFrame() {
    if (!is_recording_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (is_complete_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    const RhiStatus append_status = Append(RhiCommandRecord{RhiCommandType::EndFrame, target_handle_, RhiColor{}});
    if (append_status != RhiStatus::Success) {
        return append_status;
    }

    is_recording_ = false;
    is_complete_ = true;
    return RecordSuccess();
}

RhiCommandListSnapshot RhiCommandList::Snapshot() const {
    return RhiCommandListSnapshot{
        records_.size(),
        command_count_,
        required_command_count_,
        failed_command_index_,
        failed_command_type_,
        last_command_capacity_entry_type_,
        last_command_capacity_entry_capacity_,
        last_command_capacity_entry_command_count_,
        last_command_capacity_entry_required_count_,
        draw_command_count_,
        indexed_draw_command_count_,
        sampled_texture_bind_command_count_,
        sampler_bind_command_count_,
        constant_buffer_bind_command_count_,
        blend_state_bind_command_count_,
        is_recording_,
        is_complete_,
        last_status_};
}

const RhiCommandRecord& RhiCommandList::CommandAt(std::size_t index) const {
    return records_[index];
}

RhiTextureHandle RhiCommandList::TargetHandle() const {
    return target_handle_;
}

std::size_t RhiCommandList::Capacity() const {
    return records_.size();
}

std::size_t RhiCommandList::CommandCount() const {
    return command_count_;
}

bool RhiCommandList::IsComplete() const {
    return is_complete_;
}

RhiStatus RhiCommandList::Append(RhiCommandRecord record) {
    if (command_count_ >= records_.size()) {
        required_command_count_ = RequiredCommandCount();
        failed_command_index_ = command_count_;
        failed_command_type_ = record.type;
        last_command_capacity_entry_type_ = record.type;
        last_command_capacity_entry_capacity_ = records_.size();
        last_command_capacity_entry_command_count_ = command_count_;
        last_command_capacity_entry_required_count_ = required_command_count_;
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    records_[command_count_] = record;
    ++command_count_;
    required_command_count_ = command_count_;
    return RecordSuccess();
}

std::size_t RhiCommandList::RequiredCommandCount() const {
    return command_count_ + 1U;
}

RhiStatus RhiCommandList::RecordSuccess() {
    ClearCapacityFailure();
    last_status_ = RhiStatus::Success;
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::RecordFailure(RhiStatus status) {
    if (status != RhiStatus::CapacityExceeded) {
        ClearCapacityFailure();
    }

    last_status_ = status;
    return status;
}

void RhiCommandList::ClearCapacityFailure() {
    failed_command_index_ = 0U;
    failed_command_type_ = RhiCommandType::BeginFrame;
    last_command_capacity_entry_type_ = RhiCommandType::BeginFrame;
    last_command_capacity_entry_capacity_ = 0U;
    last_command_capacity_entry_command_count_ = 0U;
    last_command_capacity_entry_required_count_ = 0U;
}
}
