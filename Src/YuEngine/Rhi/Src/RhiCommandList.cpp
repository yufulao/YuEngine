// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Src/RhiCommandList.cpp

#include "YuEngine/Rhi/RhiCommandList.h"

namespace yuengine::rhi {
RhiCommandList::RhiCommandList(std::size_t capacity)
    : records_(capacity),
      target_handle_{},
      command_count_(0U),
      draw_command_count_(0U),
      is_recording_(false),
      is_complete_(false) {
}

RhiStatus RhiCommandList::Reset() {
    target_handle_ = RhiTextureHandle{};
    command_count_ = 0U;
    draw_command_count_ = 0U;
    is_recording_ = false;
    is_complete_ = false;
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::BeginFrame(RhiTextureHandle target) {
    if (is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
    }

    const RhiStatus append_status = Append(RhiCommandRecord{RhiCommandType::BeginFrame, target, RhiColor{}});
    if (append_status != RhiStatus::Success) {
        return append_status;
    }

    target_handle_ = target;
    is_recording_ = true;
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::RecordClear(RhiTextureHandle target, RhiColor color) {
    if (!is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
    }

    return Append(RhiCommandRecord{RhiCommandType::ClearColor, target, color});
}

RhiStatus RhiCommandList::RecordBindPipeline(RhiPipelineHandle pipeline) {
    if (!is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindPipeline;
    record.target = target_handle_;
    record.pipeline = pipeline;
    return Append(record);
}

RhiStatus RhiCommandList::RecordBindVertexBuffer(const RhiVertexBufferView &vertex_buffer) {
    if (!is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
    }

    RhiCommandRecord record{};
    record.type = RhiCommandType::BindVertexBuffer;
    record.target = target_handle_;
    record.vertex_buffer = vertex_buffer;
    return Append(record);
}

RhiStatus RhiCommandList::RecordDraw(const RhiDrawDesc &desc) {
    if (!is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
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
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::EndFrame() {
    if (!is_recording_) {
        return RhiStatus::InvalidLifecycle;
    }

    if (is_complete_) {
        return RhiStatus::InvalidLifecycle;
    }

    const RhiStatus append_status = Append(RhiCommandRecord{RhiCommandType::EndFrame, target_handle_, RhiColor{}});
    if (append_status != RhiStatus::Success) {
        return append_status;
    }

    is_recording_ = false;
    is_complete_ = true;
    return RhiStatus::Success;
}

RhiCommandListSnapshot RhiCommandList::Snapshot() const {
    return RhiCommandListSnapshot{records_.size(), command_count_, draw_command_count_, is_recording_, is_complete_};
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
        return RhiStatus::CapacityExceeded;
    }

    records_[command_count_] = record;
    ++command_count_;
    return RhiStatus::Success;
}
}
