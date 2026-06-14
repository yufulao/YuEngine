#pragma once

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"
#include "yuengine/package/package_status.h"

namespace yuengine::package {
struct PackageRegistrationResult final {
    PackageStatus Status;
    PackageId Package;
    PackageEntryId Entry;

    static PackageRegistrationResult ManifestSuccess(PackageId package);
    static PackageRegistrationResult EntrySuccess(PackageId package, PackageEntryId entry);
    static PackageRegistrationResult Failure(PackageStatus status);
    bool Succeeded() const;
};
}
