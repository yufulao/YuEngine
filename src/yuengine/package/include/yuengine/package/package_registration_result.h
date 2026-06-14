#pragma once

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"
#include "yuengine/package/package_status.h"

namespace yuengine::package {
struct package_registration_result_t final {
    PackageStatus Status;
    package_id_t Package;
    package_entry_id_t Entry;

    static package_registration_result_t ManifestSuccess(package_id_t package);
    static package_registration_result_t EntrySuccess(package_id_t package, package_entry_id_t entry);
    static package_registration_result_t Failure(PackageStatus status);
    bool Succeeded() const;
};
}
