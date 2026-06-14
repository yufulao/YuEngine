#pragma once

#include "yuengine/package/package_id.h"

namespace yuengine::package {
struct ManifestSlot final {
    PackageId id{};
    bool is_active = false;
};
}
