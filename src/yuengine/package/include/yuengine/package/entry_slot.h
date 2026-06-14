#pragma once

#include "yuengine/package/package_entry_descriptor.h"

namespace yuengine::package {
struct EntrySlot final {
    PackageEntryDescriptor descriptor{};
    bool is_active = false;
};
}
