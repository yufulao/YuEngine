#pragma once

#include "fixed_trace_buffer.h"

namespace yuengine::thread::tests {
struct ThreadTestContext {
    FixedTraceBuffer *Trace;
    int Value;
    bool ShouldFail;
};
}
