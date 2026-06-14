#pragma once

#include "FixedTraceBuffer.h"

namespace yuengine::thread::tests
{
struct ThreadTestContext
{
    FixedTraceBuffer* Trace;
    int Value;
    bool ShouldFail;
};
}
