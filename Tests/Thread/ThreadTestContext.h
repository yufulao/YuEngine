#pragma once

#include "FixedTraceBuffer.h"

namespace yuengine::thread::Tests {
struct ThreadTestContext {
    FixedTraceBuffer *trace;
    int value;
    bool should_fail;
};
}
