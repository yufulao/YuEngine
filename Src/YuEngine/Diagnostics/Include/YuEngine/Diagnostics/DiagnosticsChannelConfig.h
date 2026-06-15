// Module: YuEngine Diagnostics
// File: Src/YuEngine/Diagnostics/Include/YuEngine/Diagnostics/DiagnosticsChannelConfig.h

#pragma once

#include <cstddef>

namespace yuengine::diagnostics {
struct DiagnosticsChannelConfig {
    std::size_t event_capacity;
    std::size_t counter_capacity;
    std::size_t accepted_event_id_capacity;
    std::size_t accepted_counter_id_capacity;
    bool validate_ids;
};
}
