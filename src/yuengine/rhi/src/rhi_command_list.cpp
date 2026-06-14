#include "yuengine/rhi/rhi_command_list.h"

namespace yuengine::rhi {
RhiCommandList::RhiCommandList(std::size_t capacity)
    : _records(capacity),
      _targetHandle{},
      _commandCount(0U),
      _isRecording(false),
      _isComplete(false) {
}

RhiStatus RhiCommandList::Reset() {
    _targetHandle = RhiTextureHandle{};
    _commandCount = 0U;
    _isRecording = false;
    _isComplete = false;
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::BeginFrame(RhiTextureHandle target) {
    if (_isRecording) {
        return RhiStatus::InvalidLifecycle;
    }

    if (_isComplete) {
        return RhiStatus::InvalidLifecycle;
    }

    const RhiStatus appendStatus = Append(RhiCommandRecord{RhiCommandType::BeginFrame, target, RhiColor{}});
    if (appendStatus != RhiStatus::Success) {
        return appendStatus;
    }

    _targetHandle = target;
    _isRecording = true;
    return RhiStatus::Success;
}

RhiStatus RhiCommandList::RecordClear(RhiTextureHandle target, RhiColor color) {
    if (!_isRecording) {
        return RhiStatus::InvalidLifecycle;
    }

    if (_isComplete) {
        return RhiStatus::InvalidLifecycle;
    }

    return Append(RhiCommandRecord{RhiCommandType::ClearColor, target, color});
}

RhiStatus RhiCommandList::EndFrame() {
    if (!_isRecording) {
        return RhiStatus::InvalidLifecycle;
    }

    if (_isComplete) {
        return RhiStatus::InvalidLifecycle;
    }

    const RhiStatus appendStatus = Append(RhiCommandRecord{RhiCommandType::EndFrame, _targetHandle, RhiColor{}});
    if (appendStatus != RhiStatus::Success) {
        return appendStatus;
    }

    _isRecording = false;
    _isComplete = true;
    return RhiStatus::Success;
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

RhiStatus RhiCommandList::Append(RhiCommandRecord record) {
    if (_commandCount >= _records.size()) {
        return RhiStatus::CapacityExceeded;
    }

    _records[_commandCount] = record;
    ++_commandCount;
    return RhiStatus::Success;
}
}
