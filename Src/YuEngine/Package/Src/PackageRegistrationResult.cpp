// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Src/PackageRegistrationResult.cpp

#include "YuEngine/Package/PackageRegistrationResult.h"

namespace yuengine::package {
PackageRegistrationResult PackageRegistrationResult::ManifestSuccess(PackageId package) {
    return PackageRegistrationResult{PackageStatus::Success, package, PackageEntryId{}};
}

PackageRegistrationResult PackageRegistrationResult::EntrySuccess(PackageId package, PackageEntryId entry) {
    return PackageRegistrationResult{PackageStatus::Success, package, entry};
}

PackageRegistrationResult PackageRegistrationResult::Failure(PackageStatus status) {
    return PackageRegistrationResult{status, PackageId{}, PackageEntryId{}};
}

bool PackageRegistrationResult::Succeeded() const {
    return status == PackageStatus::Success;
}
}
