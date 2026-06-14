#pragma once

#include <array>
#include <cstddef>

namespace yuengine::thread::tests
{
constexpr std::size_t THREAD_TEST_TRACE_CAPACITY = 8U;

struct FixedTraceBuffer
{
    std::array<int, THREAD_TEST_TRACE_CAPACITY> Values{};
    std::size_t Count = 0U;

    // Appends a value when trace storage still has capacity.
    bool Append(int value)
    {
        if (Count >= Values.size())
        {
            return false;
        }

        Values[Count] = value;
        ++Count;
        return true;
    }

    // Reports whether no trace values were recorded.
    bool IsEmpty() const
    {
        return Count == 0U;
    }
};
}
