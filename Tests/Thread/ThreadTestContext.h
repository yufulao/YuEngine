#pragma once

#include "FixedTraceBuffer.h"

namespace yuengine::thread::Tests {
struct ThreadTestContext {
    FixedTraceBuffer *Trace;
    int Value;
    bool ShouldFail;
};
}
