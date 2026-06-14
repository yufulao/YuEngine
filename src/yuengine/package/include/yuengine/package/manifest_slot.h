#pragma once

#include "yuengine/package/package_id.h"

namespace yuengine::package {
struct manifest_slot_t final {
    package_id_t id{};
    bool is_active = false;
};
}
