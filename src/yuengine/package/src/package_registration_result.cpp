#include "yuengine/package/package_registration_result.h"

namespace yuengine::package {
package_registration_result_t package_registration_result_t::ManifestSuccess(package_id_t package) {
    return package_registration_result_t{PACKAGE_STATUS::Success, package, package_entry_id_t{}};
}

package_registration_result_t package_registration_result_t::EntrySuccess(package_id_t package, package_entry_id_t entry) {
    return package_registration_result_t{PACKAGE_STATUS::Success, package, entry};
}

package_registration_result_t package_registration_result_t::Failure(PACKAGE_STATUS status) {
    return package_registration_result_t{status, package_id_t{}, package_entry_id_t{}};
}

bool package_registration_result_t::Succeeded() const {
    return Status == PACKAGE_STATUS::Success;
}
}
