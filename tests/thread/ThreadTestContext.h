#pragma once

#include <vector>

namespace yuengine::thread::tests
{
struct ThreadTestContext
{
    std::vector<int>* Trace;
    int Value;
    bool ShouldFail;
};
}
