#pragma once

#include "YuEngine/Package/PackageEntryDescriptor.h"

namespace yuengine::package {
struct EntrySlot final {
    PackageEntryDescriptor descriptor{};
    bool is_active = false;
};
}
