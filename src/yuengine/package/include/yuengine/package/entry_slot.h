#pragma once

#include "yuengine/package/package_entry_descriptor.h"

namespace yuengine::package {
struct entry_slot_t final {
    package_entry_descriptor_t descriptor{};
    bool is_active = false;
};
}
