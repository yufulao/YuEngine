// 模块: YuEngine ResourceBrowser
// 文件: Src/YuEngine/ResourceBrowser/Src/ResourceBrowserDiagnostics.cpp

#include "YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h"

#include <cstddef>
#include <span>

#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Resource/ResourceRegistry.h"

namespace yuengine::resourcebrowser {
namespace {
using yuengine::runtimeasset::RuntimeAssetDataStatus;

ResourceBrowserDiagnosticCode MapRuntimeStatusCode(RuntimeAssetDataStatus status) {
    switch (status) {
        case RuntimeAssetDataStatus::Success:
            return ResourceBrowserDiagnosticCode::None;
        case RuntimeAssetDataStatus::MissingDependency:
            return ResourceBrowserDiagnosticCode::MissingDependency;
        case RuntimeAssetDataStatus::DuplicateDependency:
            return ResourceBrowserDiagnosticCode::DuplicateDependency;
        case RuntimeAssetDataStatus::InvalidKind:
        case RuntimeAssetDataStatus::TypeMismatch:
            return ResourceBrowserDiagnosticCode::TypeMismatch;
        case RuntimeAssetDataStatus::HashMismatch:
            return ResourceBrowserDiagnosticCode::StaleHash;
        case RuntimeAssetDataStatus::InvalidSchema:
            return ResourceBrowserDiagnosticCode::StaleSchema;
        case RuntimeAssetDataStatus::UnsupportedVersion:
        case RuntimeAssetDataStatus::UnsupportedFieldValue:
            return ResourceBrowserDiagnosticCode::Unsupported;
        case RuntimeAssetDataStatus::CapacityExceeded:
            return ResourceBrowserDiagnosticCode::CapacityExceeded;
        case RuntimeAssetDataStatus::BudgetExceeded:
            return ResourceBrowserDiagnosticCode::BudgetExceeded;
        default:
            return ResourceBrowserDiagnosticCode::ValidateFailed;
    }
}

ResourceBrowserDependencyState MapDependencyState(RuntimeAssetDataStatus status) {
    switch (MapRuntimeStatusCode(status)) {
        case ResourceBrowserDiagnosticCode::None:
            return ResourceBrowserDependencyState::Ready;
        case ResourceBrowserDiagnosticCode::MissingDependency:
            return ResourceBrowserDependencyState::Missing;
        case ResourceBrowserDiagnosticCode::DuplicateDependency:
            return ResourceBrowserDependencyState::Duplicate;
        case ResourceBrowserDiagnosticCode::TypeMismatch:
            return ResourceBrowserDependencyState::TypeMismatch;
        case ResourceBrowserDiagnosticCode::StaleHash:
            return ResourceBrowserDependencyState::StaleHash;
        case ResourceBrowserDiagnosticCode::StaleSchema:
            return ResourceBrowserDependencyState::StaleSchema;
        case ResourceBrowserDiagnosticCode::Unsupported:
            return ResourceBrowserDependencyState::Unsupported;
        case ResourceBrowserDiagnosticCode::CapacityExceeded:
            return ResourceBrowserDependencyState::CapacityExceeded;
        case ResourceBrowserDiagnosticCode::BudgetExceeded:
            return ResourceBrowserDependencyState::BudgetExceeded;
        default:
            break;
    }

    return ResourceBrowserDependencyState::Unknown;
}

const char *MessageForCode(ResourceBrowserDiagnosticCode code) {
    switch (code) {
        case ResourceBrowserDiagnosticCode::FileReadFailed:
            return "file read failed through mounted VFS path";
        case ResourceBrowserDiagnosticCode::ValidateFailed:
            return "runtime asset validation failed";
        case ResourceBrowserDiagnosticCode::LoadFailed:
            return "runtime asset load failed";
        case ResourceBrowserDiagnosticCode::MissingDependency:
            return "required dependency is missing";
        case ResourceBrowserDiagnosticCode::DuplicateDependency:
            return "dependency is duplicated";
        case ResourceBrowserDiagnosticCode::TypeMismatch:
            return "runtime asset type or dependency type mismatch";
        case ResourceBrowserDiagnosticCode::StaleHash:
            return "runtime asset hash is stale or mismatched";
        case ResourceBrowserDiagnosticCode::StaleSchema:
            return "runtime asset schema is stale or invalid";
        case ResourceBrowserDiagnosticCode::Unsupported:
            return "runtime asset field or version is unsupported";
        case ResourceBrowserDiagnosticCode::CapacityExceeded:
            return "diagnostic or runtime output capacity exceeded";
        case ResourceBrowserDiagnosticCode::BudgetExceeded:
            return "runtime asset budget exceeded";
        case ResourceBrowserDiagnosticCode::ResourceQueryFailed:
            return "resource registry query failed";
        case ResourceBrowserDiagnosticCode::AssetQueryFailed:
            return "asset query failed";
        case ResourceBrowserDiagnosticCode::None:
            break;
        default:
            break;
    }

    return "no diagnostic";
}

ResourceBrowserDiagnosticPhase PhaseForRuntimeStatus(RuntimeAssetDataStatus status) {
    switch (status) {
        case RuntimeAssetDataStatus::FileReadFailed:
            return ResourceBrowserDiagnosticPhase::FileRead;
        case RuntimeAssetDataStatus::ResourceRegistrationFailed:
        case RuntimeAssetDataStatus::ResourceLoadCommitFailed:
        case RuntimeAssetDataStatus::ResourceResidencyFailed:
        case RuntimeAssetDataStatus::CachePayloadStoreFailed:
        case RuntimeAssetDataStatus::DecodePlanFailed:
        case RuntimeAssetDataStatus::DecodeResultFailed:
        case RuntimeAssetDataStatus::DecodedPayloadStoreFailed:
        case RuntimeAssetDataStatus::ResourceDependencyFailed:
            return ResourceBrowserDiagnosticPhase::Resource;
        case RuntimeAssetDataStatus::AssetRegistrationFailed:
        case RuntimeAssetDataStatus::AssetDependencyFailed:
            return ResourceBrowserDiagnosticPhase::Asset;
        case RuntimeAssetDataStatus::MissingDependency:
        case RuntimeAssetDataStatus::DuplicateDependency:
        case RuntimeAssetDataStatus::InvalidDependency:
        case RuntimeAssetDataStatus::TypeMismatch:
            return ResourceBrowserDiagnosticPhase::Dependency;
        default:
            break;
    }

    return ResourceBrowserDiagnosticPhase::Validate;
}

bool PushDiagnostic(
    ResourceBrowserDiagnosticsResult *result,
    const ResourceBrowserDiagnosticsRequest &request,
    const ResourceBrowserDiagnosticRecord &record) {
    if (result == nullptr) {
        return false;
    }

    if (result->diagnostic_count >= request.diagnostic_capacity) {
        result->status = ResourceBrowserDiagnosticsStatus::OutputCapacityExceeded;
        result->diagnostic_overflow = true;
        return false;
    }

    request.diagnostics[result->diagnostic_count] = record;
    ++result->diagnostic_count;
    return true;
}

bool PushCapacityDiagnostic(
    ResourceBrowserDiagnosticsResult *result,
    const ResourceBrowserDiagnosticsRequest &request) {
    ResourceBrowserDiagnosticRecord record{};
    record.code = ResourceBrowserDiagnosticCode::CapacityExceeded;
    record.severity = ResourceBrowserDiagnosticSeverity::Blocker;
    record.phase = ResourceBrowserDiagnosticPhase::ImportSettings;
    record.runtime_status = RuntimeAssetDataStatus::CapacityExceeded;
    record.message = MessageForCode(record.code);
    return PushDiagnostic(result, request, record);
}

const yuengine::runtimeasset::RuntimeAssetLoadedFile *FindLoadedFile(
    const ResourceBrowserDiagnosticsRequest &request,
    std::uint32_t file_index,
    std::uint64_t stable_id) {
    if (request.loaded_files == nullptr || request.loaded_file_count == 0U) {
        return nullptr;
    }

    if (file_index < request.loaded_file_count &&
        request.loaded_files[file_index].stable_id == stable_id) {
        return &request.loaded_files[file_index];
    }

    for (std::uint32_t index = 0U; index < request.loaded_file_count; ++index) {
        if (request.loaded_files[index].stable_id == stable_id) {
            return &request.loaded_files[index];
        }
    }

    return nullptr;
}

ResourceBrowserImportSettings ImportSettingsFor(
    const yuengine::runtimeasset::RuntimeAssetFileDesc &file) {
    ResourceBrowserImportSettings settings{};
    settings.source_path = file.path;
    settings.target_kind = file.kind;
    settings.resource_type = file.resource_type;
    settings.asset_type = file.asset_type;
    settings.stable_id = file.stable_id;
    settings.importer_version = 1U;
    settings.expected_schema_version = 1U;
    return settings;
}

bool IsValidRequest(const ResourceBrowserDiagnosticsRequest &request) {
    if (request.mount_table == nullptr) {
        return false;
    }

    if (request.file_count > 0U && request.files == nullptr) {
        return false;
    }

    if (request.entry_capacity > 0U && request.entries == nullptr) {
        return false;
    }

    if (request.diagnostic_capacity > 0U && request.diagnostics == nullptr) {
        return false;
    }

    if (request.file_count > 0U && request.entry_capacity == 0U) {
        return false;
    }

    return true;
}

}

ResourceBrowserDiagnosticsStatus BuildResourceBrowserRuntimeAssetDiagnostics(
    const ResourceBrowserDiagnosticsRequest &request,
    ResourceBrowserDiagnosticsResult *out_result) {
    if (out_result == nullptr) {
        return ResourceBrowserDiagnosticsStatus::InvalidArgument;
    }

    ResourceBrowserDiagnosticsResult result{};
    result.status = ResourceBrowserDiagnosticsStatus::InvalidArgument;
    if (!IsValidRequest(request)) {
        *out_result = result;
        return result.status;
    }

    result.status = ResourceBrowserDiagnosticsStatus::Success;
    if (request.resource_registry != nullptr) {
        result.resource_snapshot = request.resource_registry->Snapshot();
        result.observed_resource_registry = true;
    }

    if (request.asset_manager != nullptr) {
        result.asset_snapshot = request.asset_manager->Snapshot();
        result.observed_asset_manager = true;
    }

    if (request.file_count > request.entry_capacity) {
        PushCapacityDiagnostic(&result, request);
        result.status = ResourceBrowserDiagnosticsStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    for (std::uint32_t index = 0U; index < request.file_count; ++index) {
        const yuengine::runtimeasset::RuntimeAssetFileDesc &file = request.files[index];
        ResourceBrowserResourceEntry entry{};
        entry.import_settings = ImportSettingsFor(file);

        yuengine::file::FileReadRequest read_request{};
        read_request.mount = request.mount;
        read_request.path = yuengine::file::VirtualPath(file.path);
        const yuengine::file::FileReadResult read_result = request.mount_table->Read(read_request);
        entry.file_status = read_result.status;
        entry.from_file_vfs = true;
        result.used_file_vfs = true;
        ++result.file_read_count;
        if (!read_result.Succeeded()) {
            ResourceBrowserDiagnosticRecord record{};
            record.code = ResourceBrowserDiagnosticCode::FileReadFailed;
            record.severity = ResourceBrowserDiagnosticSeverity::Blocker;
            record.phase = ResourceBrowserDiagnosticPhase::FileRead;
            record.runtime_status = RuntimeAssetDataStatus::FileReadFailed;
            record.file_status = read_result.status;
            record.source_path = file.path;
            record.expected_kind = file.kind;
            record.file_index = index;
            record.message = MessageForCode(record.code);
            if (!PushDiagnostic(&result, request, record)) {
                *out_result = result;
                return result.status;
            }

            request.entries[result.entry_count] = entry;
            ++result.entry_count;
            continue;
        }

        entry.validation = yuengine::runtimeasset::RuntimeAssetValidationResult{};
        const RuntimeAssetDataStatus validation_status =
            yuengine::runtimeasset::ValidateRuntimeAssetDataBytes(
                std::span<const std::uint8_t>(read_result.bytes.data(), read_result.bytes.size()),
                file.kind,
                &entry.validation);
        entry.from_runtime_asset_validation = true;
        entry.dependency_state = MapDependencyState(validation_status);
        ++result.validation_count;
        if (validation_status != RuntimeAssetDataStatus::Success) {
            ++result.validation_failure_count;
            const ResourceBrowserDiagnosticCode code = MapRuntimeStatusCode(validation_status);
            ResourceBrowserDiagnosticRecord record{};
            record.code = code;
            record.severity = ResourceBrowserDiagnosticSeverity::Error;
            record.phase = PhaseForRuntimeStatus(validation_status);
            record.runtime_status = validation_status;
            record.file_status = read_result.status;
            record.source_path = file.path;
            record.expected_kind = file.kind;
            record.artifact_class = entry.validation.artifact_class;
            record.file_index = index;
            record.expected_schema_version = entry.import_settings.expected_schema_version;
            record.actual_schema_version = entry.validation.schema_version;
            record.expected_hash = entry.import_settings.expected_source_hash;
            record.actual_hash = entry.validation.hash;
            record.message = MessageForCode(code);
            if (!PushDiagnostic(&result, request, record)) {
                *out_result = result;
                return result.status;
            }
        }

        if (const yuengine::runtimeasset::RuntimeAssetLoadedFile *loaded =
                FindLoadedFile(request, index, file.stable_id)) {
            entry.resource = loaded->resource;
            entry.asset = loaded->asset;
            entry.cache_payload_id = loaded->cache_payload_id;
            entry.decode_plan_id = loaded->decode_plan_id;
            entry.decode_result_id = loaded->decode_result_id;
            entry.decoded_payload_id = loaded->decoded_payload_id;
            entry.decoded_byte_count = loaded->decoded_byte_count;
            entry.from_runtime_asset_load = true;
            entry.from_asset_record = loaded->asset.IsValid();
            ++result.loaded_record_count;
            if (request.resource_registry != nullptr && loaded->resource.IsValid()) {
                yuengine::resource::ResourceLoadState load_state =
                    yuengine::resource::ResourceLoadState::Unloaded;
                entry.resource_load_status =
                    request.resource_registry->GetLoadState(loaded->resource, loaded->resource_type, &load_state);
                entry.resource_load_state = load_state;
                entry.from_resource_registry = entry.resource_load_status ==
                    yuengine::resource::ResourceLoadCommitStatus::Success;
                ++result.resource_query_count;
                if (!entry.from_resource_registry) {
                    ResourceBrowserDiagnosticRecord record{};
                    record.code = ResourceBrowserDiagnosticCode::ResourceQueryFailed;
                    record.severity = ResourceBrowserDiagnosticSeverity::Error;
                    record.phase = ResourceBrowserDiagnosticPhase::Resource;
                    record.runtime_status = RuntimeAssetDataStatus::ResourceLoadCommitFailed;
                    record.resource_load_status = entry.resource_load_status;
                    record.source_path = file.path;
                    record.expected_kind = file.kind;
                    record.file_index = index;
                    record.message = MessageForCode(record.code);
                    if (!PushDiagnostic(&result, request, record)) {
                        *out_result = result;
                        return result.status;
                    }
                }
            }
        }

        request.entries[result.entry_count] = entry;
        ++result.entry_count;
    }

    result.file_snapshot = request.mount_table->Snapshot();
    if (request.resource_registry != nullptr) {
        result.resource_snapshot = request.resource_registry->Snapshot();
    }

    if (request.asset_manager != nullptr) {
        result.asset_snapshot = request.asset_manager->Snapshot();
    }

    *out_result = result;
    return result.status;
}

}
