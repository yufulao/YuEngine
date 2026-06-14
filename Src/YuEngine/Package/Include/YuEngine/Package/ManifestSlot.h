#pragma once

#include "YuEngine/Package/PackageId.h"

namespace yuengine::package {
struct ManifestSlot final {
    PackageId id{};
    bool is_active = false;
};
}
