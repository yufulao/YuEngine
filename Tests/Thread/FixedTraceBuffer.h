// 模块：Tests Thread
// 文件：Tests/Thread/FixedTraceBuffer.h

#pragma once

#include <array>
#include <cstddef>

namespace yuengine::thread::Tests {
constexpr std::size_t THREAD_TEST_TRACE_CAPACITY = 8U;

struct FixedTraceBuffer {
    std::array<int, THREAD_TEST_TRACE_CAPACITY> values{};
    std::size_t count = 0U;

    // 当跟踪存储仍有容量时追加一个值。
    bool Append(int value) {
        if (count >= values.size()) {
            return false;
        }

        values[count] = value;
        ++count;
        return true;
    }

    // 报告是否没有记录任何跟踪值。
    bool IsEmpty() const {
        return count == 0U;
    }
};
}
