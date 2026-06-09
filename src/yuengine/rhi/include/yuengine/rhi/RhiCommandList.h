#pragma once

#include <cstddef>
#include <vector>

#include "yuengine/rhi/RhiColor.h"
#include "yuengine/rhi/RhiCommandListSnapshot.h"
#include "yuengine/rhi/RhiCommandRecord.h"
#include "yuengine/rhi/RhiStatus.h"
#include "yuengine/rhi/RhiTextureHandle.h"

namespace yuengine::rhi
{
class RhiCommandList final
{
public:
    explicit RhiCommandList(std::size_t capacity);

    RhiStatus Reset();
    RhiStatus BeginFrame(RhiTextureHandle target);
    RhiStatus RecordClear(RhiTextureHandle target, RhiColor color);
    RhiStatus EndFrame();
    RhiCommandListSnapshot Snapshot() const;
    const RhiCommandRecord& CommandAt(std::size_t index) const;
    RhiTextureHandle TargetHandle() const;
    std::size_t Capacity() const;
    std::size_t CommandCount() const;
    bool IsComplete() const;

private:
    RhiStatus Append(RhiCommandRecord record);

    std::vector<RhiCommandRecord> _records;
    RhiTextureHandle _targetHandle;
    std::size_t _commandCount;
    bool _isRecording;
    bool _isComplete;
};
}
