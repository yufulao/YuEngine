#pragma once

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"

namespace yuengine::package {
struct dependency_edge_t final {
    package_id_t package{};
    package_entry_id_t dependent{};
    package_entry_id_t dependency{};
    bool is_active = false;
};
}
