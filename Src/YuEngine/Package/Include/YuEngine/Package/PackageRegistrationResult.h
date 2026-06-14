#pragma once

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageStatus.h"

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
