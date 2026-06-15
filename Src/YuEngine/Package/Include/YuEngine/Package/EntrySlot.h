// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/EntrySlot.h

#pragma once

#include "YuEngine/Package/PackageEntryDescriptor.h"

namespace yuengine::package {
struct EntrySlot final {
    PackageEntryDescriptor descriptor{};
    bool is_active = false;
};
}
