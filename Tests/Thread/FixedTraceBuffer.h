// Module: Tests Thread
// File: Tests/Thread/FixedTraceBuffer.h

#pragma once

#include <array>
#include <cstddef>

namespace yuengine::thread::Tests {
constexpr std::size_t THREAD_TEST_TRACE_CAPACITY = 8U;

struct FixedTraceBuffer {
    std::array<int, THREAD_TEST_TRACE_CAPACITY> values{};
    std::size_t count = 0U;

    // Appends a value when trace storage still has capacity.
    bool Append(int value) {
        if (count >= values.size()) {
            return false;
        }

        values[count] = value;
        ++count;
        return true;
    }

    // Reports whether no trace values were recorded.
    bool IsEmpty() const {
        return count == 0U;
    }
};
}
