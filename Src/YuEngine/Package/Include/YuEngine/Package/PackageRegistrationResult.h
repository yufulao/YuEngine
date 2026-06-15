// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/PackageRegistrationResult.h

#pragma once

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageRegistrationResult final {
    PackageStatus status;
    PackageId package;
    PackageEntryId entry;

    /**
     * @comment Creates a successful manifest registration result.
     * @param package Input package.
     * @return Explicit operation result.
     */
    static PackageRegistrationResult ManifestSuccess(PackageId package);
    /**
     * @comment Creates a successful entry registration result.
     * @param package Input package.
     * @param entry Input entry.
     * @return Explicit operation result.
     */
    static PackageRegistrationResult EntrySuccess(PackageId package, PackageEntryId entry);
    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static PackageRegistrationResult Failure(PackageStatus status);
    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const;
};
}
