#include "yuengine/rhi/rhi_command_list.h"

namespace yuengine::rhi {
RhiCommandList::RhiCommandList(std::size_t capacity)
    : _records(capacity),
      _targetHandle{},
      _commandCount(0U),
      _isRecording(false),
      _isComplete(false) {
}

RHI_STATUS RhiCommandList::Reset() {
    _targetHandle = RhiTextureHandle{};
    _commandCount = 0U;
    _isRecording = false;
    _isComplete = false;
    return RHI_STATUS::Success;
}

RHI_STATUS RhiCommandList::BeginFrame(RhiTextureHandle target) {
    if (_isRecording) {
        return RHI_STATUS::InvalidLifecycle;
    }

    if (_isComplete) {
        return RHI_STATUS::InvalidLifecycle;
    }

    const RHI_STATUS appendStatus = Append(RhiCommandRecord{RHI_COMMAND_TYPE::BeginFrame, target, RhiColor{}});
    if (appendStatus != RHI_STATUS::Success) {
        return appendStatus;
    }

    _targetHandle = target;
    _isRecording = true;
    return RHI_STATUS::Success;
}

RHI_STATUS RhiCommandList::RecordClear(RhiTextureHandle target, RhiColor color) {
    if (!_isRecording) {
        return RHI_STATUS::InvalidLifecycle;
    }

    if (_isComplete) {
        return RHI_STATUS::InvalidLifecycle;
    }

    return Append(RhiCommandRecord{RHI_COMMAND_TYPE::ClearColor, target, color});
}

RHI_STATUS RhiCommandList::EndFrame() {
    if (!_isRecording) {
        return RHI_STATUS::InvalidLifecycle;
    }

    if (_isComplete) {
        return RHI_STATUS::InvalidLifecycle;
    }

    const RHI_STATUS appendStatus = Append(RhiCommandRecord{RHI_COMMAND_TYPE::EndFrame, _targetHandle, RhiColor{}});
    if (appendStatus != RHI_STATUS::Success) {
        return appendStatus;
    }

    _isRecording = false;
    _isComplete = true;
    return RHI_STATUS::Success;
}

RhiCommandListSnapshot RhiCommandList::Snapshot() const {
    return RhiCommandListSnapshot{_records.size(), _commandCount, _isRecording, _isComplete};
}

const RhiCommandRecord& RhiCommandList::CommandAt(std::size_t index) const {
    return _records[index];
}

RhiTextureHandle RhiCommandList::TargetHandle() const {
    return _targetHandle;
}

std::size_t RhiCommandList::Capacity() const {
    return _records.size();
}

std::size_t RhiCommandList::CommandCount() const {
    return _commandCount;
}

bool RhiCommandList::IsComplete() const {
    return _isComplete;
}

RHI_STATUS RhiCommandList::Append(RhiCommandRecord record) {
    if (_commandCount >= _records.size()) {
        return RHI_STATUS::CapacityExceeded;
    }

    _records[_commandCount] = record;
    ++_commandCount;
    return RHI_STATUS::Success;
}
}
