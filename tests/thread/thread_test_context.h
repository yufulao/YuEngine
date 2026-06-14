#pragma once

#include "fixed_trace_buffer.h"

namespace yuengine::thread::tests {
struct thread_test_context_t {
    fixed_trace_buffer_t *Trace;
    int Value;
    bool ShouldFail;
};
}
