#include "yuengine/package/package_registration_result.h"

namespace yuengine::package {
PackageRegistrationResult PackageRegistrationResult::ManifestSuccess(PackageId package) {
    return PackageRegistrationResult{PACKAGE_STATUS::Success, package, PackageEntryId{}};
}

PackageRegistrationResult PackageRegistrationResult::EntrySuccess(PackageId package, PackageEntryId entry) {
    return PackageRegistrationResult{PACKAGE_STATUS::Success, package, entry};
}

PackageRegistrationResult PackageRegistrationResult::Failure(PACKAGE_STATUS status) {
    return PackageRegistrationResult{status, PackageId{}, PackageEntryId{}};
}

bool PackageRegistrationResult::Succeeded() const {
    return Status == PACKAGE_STATUS::Success;
}
}
