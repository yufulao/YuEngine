#pragma once

#include "yuengine/package/PackageEntryDescriptor.h"

namespace yuengine::package {
struct EntrySlot final {
    PackageEntryDescriptor descriptor{};
    bool is_active = false;
};
}
