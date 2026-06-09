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

    bool IsEmpty() const
    {
        return Count == 0U;
    }
};

struct ThreadTestContext
{
    FixedTraceBuffer* Trace;
    int Value;
    bool ShouldFail;
};
}
