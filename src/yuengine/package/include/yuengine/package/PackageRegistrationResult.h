#pragma once

#include "yuengine/package/PackageEntryId.h"
#include "yuengine/package/PackageId.h"
#include "yuengine/package/PackageStatus.h"

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
