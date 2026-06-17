// Module: Tests Resource
// File: Tests/Resource/ResourceTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadOperation.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadSnapshot.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanRecord.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanSnapshot.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencySnapshot.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceCachePayloadBudgetDesc;
using yuengine::resource::ResourceCachePayloadOperation;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadSnapshot;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodePlanBudgetDesc;
using yuengine::resource::ResourceDecodePlanOperation;
using yuengine::resource::ResourceDecodePlanRecord;
using yuengine::resource::ResourceDecodePlanRequest;
using yuengine::resource::ResourceDecodePlanSnapshot;
using yuengine::resource::ResourceDecodePlanStatus;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencySnapshot;
using yuengine::resource::ResourceResidencyState;
using yuengine::resource::ResourceResidencyStatus;
using ResourceLogicalKey = yuengine::resource::ResourceLogicalKey;
using ResourceRegistry = yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistryDesc;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::resource::INVALID_RESOURCE_GENERATION;
using yuengine::resource::MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT;
using yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_VERSION;

namespace {
constexpr const char* TEST_REGISTER = "Resource_RegisterSyntheticDescriptor_ReturnsGenerationHandle";
constexpr const char* TEST_DUPLICATE = "Resource_RegisterDuplicate_ReturnsExplicitStatus";
constexpr const char* TEST_CAPACITY = "Resource_RegistryRejectsCapacityOverflowWithoutMutation";
constexpr const char* TEST_TYPE_CAPACITY = "Resource_TypeCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_WRONG_GENERATION = "Resource_HandleRejectsWrongGeneration";
constexpr const char* TEST_TYPE_MISMATCH = "Resource_HandleRejectsTypeMismatch";
constexpr const char* TEST_ACQUIRE_RELEASE = "Resource_AcquireRelease_TracksReferenceCount";
constexpr const char* TEST_REPEATED_ACQUIRE = "Resource_RepeatedAcquire_IncrementsReferenceCount";
constexpr const char* TEST_REFERENCE_OVERFLOW = "Resource_AcquireRejectsReferenceCountOverflow";
constexpr const char *TEST_VALIDATE_ACQUIRE =
    "Resource_ValidateAcquireChecksHandleTypeAndOverflowWithoutMutation";
constexpr const char *TEST_VALIDATE_ACQUIRE_WORLD_FREE =
    "Resource_ValidateAcquireDoesNotRequireWorld";
constexpr const char* TEST_RETIRE_REFERENCED = "Resource_RetireRejectsOutstandingAcquire";
constexpr const char* TEST_RETIRE_DEPENDED_ON = "Resource_RetireRejectsLiveDependentEdge";
constexpr const char* TEST_MISSING_DEPENDENCY = "Resource_DependencyValidationRejectsMissingDependency";
constexpr const char* TEST_DEPENDENCY_CYCLE = "Resource_DependencyValidationRejectsCycle";
constexpr const char* TEST_NO_FILE_PACKAGE = "Resource_NoFileOrPackageDependency_ForHandleRegistry";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Resource_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Resource_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char *TEST_LOAD_COMMIT_SUCCESS = "Resource_LoadCommit_UploadSuccessSetsTerminalState";
constexpr const char *TEST_LOAD_COMMIT_FAILED_UPLOAD = "Resource_LoadCommit_FailedUploadSetsFailedState";
constexpr const char *TEST_LOAD_COMMIT_INVALID_HANDLE =
    "Resource_LoadCommit_RejectsInvalidHandleWithoutSlotMutation";
constexpr const char *TEST_LOAD_COMMIT_TYPE_MISMATCH =
    "Resource_LoadCommit_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_LOAD_COMMIT_DUPLICATE = "Resource_LoadCommit_RejectsDuplicateCommitId";
constexpr const char *TEST_LOAD_COMMIT_INVALID_TRANSITION =
    "Resource_LoadCommit_RejectsInvalidTransition";
constexpr const char *TEST_LOAD_COMMIT_SNAPSHOT = "Resource_LoadCommit_SnapshotTracksCounters";
constexpr const char *TEST_RESIDENCY_ADMIT = "Resource_Residency_AdmitsUploadedSlotWithinBudget";
constexpr const char *TEST_RESIDENCY_UNLOADED =
    "Resource_Residency_RejectsUnloadedWithoutMutation";
constexpr const char *TEST_RESIDENCY_FAILED_LOAD =
    "Resource_Residency_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_RESIDENCY_TYPE_MISMATCH =
    "Resource_Residency_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_RESIDENCY_DUPLICATE = "Resource_Residency_RejectsDuplicateAdmission";
constexpr const char *TEST_RESIDENCY_BUDGET = "Resource_Residency_RejectsBudgetOverflow";
constexpr const char *TEST_RESIDENCY_PIN_UNPIN = "Resource_Residency_PinUnpinTracksCounters";
constexpr const char *TEST_RESIDENCY_CANDIDATE =
    "Resource_Residency_SelectsEvictionCandidateInSlotOrder";
constexpr const char *TEST_RESIDENCY_NO_CANDIDATE = "Resource_Residency_ReportsNoEvictionCandidate";
constexpr const char *TEST_RESIDENCY_FAILED_VALIDATION =
    "Resource_Residency_FailedValidationDoesNotMutateResourceState";
constexpr const char *TEST_RESIDENCY_EVICT = "Resource_Residency_EvictsResourceOwnedStateOnly";
constexpr const char *TEST_CACHE_PAYLOAD_STORE_READ =
    "Resource_CachePayload_StoresAndReadsResidentBytes";
constexpr const char *TEST_CACHE_PAYLOAD_RELEASE = "Resource_CachePayload_ReleaseClearsPayloadOnly";
constexpr const char *TEST_CACHE_PAYLOAD_NOT_RESIDENT =
    "Resource_CachePayload_RejectsNotResidentWithoutMutation";
constexpr const char *TEST_CACHE_PAYLOAD_FAILED_LOAD =
    "Resource_CachePayload_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_CACHE_PAYLOAD_STALE_HANDLE =
    "Resource_CachePayload_RejectsStaleHandleWithoutMutation";
constexpr const char *TEST_CACHE_PAYLOAD_TYPE_MISMATCH =
    "Resource_CachePayload_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_CACHE_PAYLOAD_DUPLICATE =
    "Resource_CachePayload_RejectsDuplicatePayloadId";
constexpr const char *TEST_CACHE_PAYLOAD_CAPACITY =
    "Resource_CachePayload_RejectsCapacityOverflow";
constexpr const char *TEST_CACHE_PAYLOAD_BUDGET =
    "Resource_CachePayload_RejectsBudgetOverflow";
constexpr const char *TEST_CACHE_PAYLOAD_OUTPUT_SMALL =
    "Resource_CachePayload_ReadRejectsOutputBufferTooSmall";
constexpr const char *TEST_CACHE_PAYLOAD_FAILED_VALIDATION =
    "Resource_CachePayload_FailedValidationDoesNotMutateResourceState";
constexpr const char *TEST_CACHE_PAYLOAD_PINNED_RELEASE =
    "Resource_CachePayload_ReleaseRejectsPinnedWithoutMutation";
constexpr const char *TEST_DECODE_PLAN_CREATE_QUERY_RELEASE =
    "Resource_DecodePlan_CreatesQueriesAndReleasesMetadata";
constexpr const char *TEST_DECODE_PLAN_MISSING_PAYLOAD =
    "Resource_DecodePlan_RejectsMissingCachePayload";
constexpr const char *TEST_DECODE_PLAN_INVALID_HEADER =
    "Resource_DecodePlan_RejectsInvalidHeader";
constexpr const char *TEST_DECODE_PLAN_UNSUPPORTED_VERSION =
    "Resource_DecodePlan_RejectsUnsupportedHeaderVersion";
constexpr const char *TEST_DECODE_PLAN_STALE_HANDLE =
    "Resource_DecodePlan_RejectsStaleHandleWithoutMutation";
constexpr const char *TEST_DECODE_PLAN_TYPE_MISMATCH =
    "Resource_DecodePlan_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_DECODE_PLAN_NOT_RESIDENT =
    "Resource_DecodePlan_RejectsNotResidentWithoutMutation";
constexpr const char *TEST_DECODE_PLAN_FAILED_LOAD =
    "Resource_DecodePlan_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_DECODE_PLAN_DUPLICATE =
    "Resource_DecodePlan_RejectsDuplicatePlanId";
constexpr const char *TEST_DECODE_PLAN_CAPACITY =
    "Resource_DecodePlan_RejectsCapacityOverflow";
constexpr const char *TEST_DECODE_PLAN_BUDGET =
    "Resource_DecodePlan_RejectsBudgetOverflow";
constexpr const char *TEST_DECODE_PLAN_FAILED_VALIDATION =
    "Resource_DecodePlan_FailedValidationDoesNotMutateResourceState";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr ResourceTypeId TYPE_MATERIAL{2U};
constexpr ResourceTypeId TYPE_AUDIO{3U};
constexpr ResourceTypeId TYPE_EFFECT{4U};
constexpr std::uint32_t INVALID_GENERATION = INVALID_RESOURCE_GENERATION;
constexpr const char* TYPE_CAPACITY_TEXTURE_KEY = "texture_a";
constexpr const char* TYPE_CAPACITY_MATERIAL_KEY = "material_a";
constexpr const char* TYPE_CAPACITY_RETRY_TEXTURE_KEY = "texture_b";
constexpr const char* TYPE_CAPACITY_FIRST_REGISTRATION_FAILED = "first registration failed before type capacity";
constexpr const char* TYPE_CAPACITY_STATUS_FAILED = "type capacity overflow did not return explicit status";
constexpr const char* TYPE_CAPACITY_VALID_HANDLE_FAILED = "type capacity overflow returned a valid handle";
constexpr const char* TYPE_CAPACITY_TYPE_COUNT_FAILED = "type capacity overflow changed type count";
constexpr const char* TYPE_CAPACITY_REGISTERED_COUNT_FAILED = "type capacity overflow changed registered count";
constexpr const char* TYPE_CAPACITY_ACQUIRED_COUNT_FAILED = "type capacity overflow changed acquired count";
constexpr const char* TYPE_CAPACITY_RETRY_REGISTRATION_FAILED = "retry after type capacity failure did not register existing type";
constexpr const char* TYPE_CAPACITY_RETRY_SLOT_FAILED = "retry after type capacity failure used wrong slot";
constexpr const char* TYPE_CAPACITY_RETRY_GENERATION_FAILED = "type capacity failure advanced retry slot generation";
constexpr const char* TYPE_CAPACITY_RETRY_TYPE_COUNT_FAILED = "retry with existing type changed type count";
constexpr std::uint64_t COMMIT_ONE = 2001U;
constexpr std::uint64_t COMMIT_TWO = 2002U;
constexpr std::uint64_t COMMIT_THREE = 2003U;
constexpr std::uint64_t UPLOAD_ONE = 3001U;
constexpr std::uint64_t UPLOAD_TWO = 3002U;
constexpr std::uint64_t UPLOAD_THREE = 3003U;
constexpr std::uint64_t STAGING_ONE = 4001U;
constexpr std::uint32_t UPLOAD_BYTE_COUNT = 64U;
constexpr std::uint64_t PAYLOAD_ONE = 5001U;
constexpr std::uint64_t PAYLOAD_TWO = 5002U;
constexpr std::uint64_t PAYLOAD_THREE = 5003U;
constexpr std::uint64_t DECODE_PLAN_ONE = 6001U;
constexpr std::uint64_t DECODE_PLAN_TWO = 6002U;
constexpr std::uint32_t DECODE_PLAN_DECODED_BYTE_COUNT = 128U;
constexpr std::uint32_t DECODE_PLAN_PAYLOAD_BYTE_COUNT = RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT + 4U;
using TestFunction = int (*)();
using TestRegistry = std::unordered_map<std::string_view, TestFunction>;

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ResourceDescriptor Descriptor(ResourceTypeId type, const char* key) {
    return ResourceDescriptor{type, ResourceLogicalKey(key), 0U};
}

ResourceDescriptor DescriptorWithReferenceCount(ResourceTypeId type, const char* key, std::uint32_t reference_count) {
    return ResourceDescriptor{type, ResourceLogicalKey(key), reference_count};
}

ResourceRegistrationResult Register(ResourceRegistry& registry, ResourceTypeId type, const char* key) {
    return registry.RegisterSyntheticDescriptor(Descriptor(type, key));
}

ResourceLoadCommitRequest LoadCommitRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState load_state,
    std::uint64_t commit_id,
    std::uint64_t upload_id) {
    ResourceLoadCommitRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.load_state = load_state;
    request.commit_id = commit_id;
    request.upload_id = upload_id;
    request.staging_request_id = STAGING_ONE;
    request.upload_byte_count = UPLOAD_BYTE_COUNT;
    return request;
}

ResourceResidencyRequest ResidencyRequest(ResourceHandle resource, ResourceTypeId expected_type) {
    ResourceResidencyRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    return request;
}

bool LoadStateMatches(
    const ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState expected_state) {
    ResourceLoadState load_state = ResourceLoadState::Unloaded;
    const ResourceLoadCommitStatus status = registry.GetLoadState(resource, expected_type, &load_state);
    if (status != ResourceLoadCommitStatus::Success) {
        return false;
    }

    return load_state == expected_state;
}

bool ResidencyStateMatches(
    const ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceResidencyState expected_state) {
    ResourceResidencyState residency_state = ResourceResidencyState::Unloaded;
    const ResourceResidencyStatus status = registry.GetResidencyState(
        resource,
        expected_type,
        &residency_state);
    if (status != ResourceResidencyStatus::Success) {
        return false;
    }

    return residency_state == expected_state;
}

ResourceCachePayloadRequest CachePayloadRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    const std::uint8_t *payload_bytes,
    std::uint32_t payload_byte_count) {
    ResourceCachePayloadRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.payload_id = payload_id;
    request.payload_bytes = payload_bytes;
    request.payload_byte_count = payload_byte_count;
    return request;
}

void WriteU32LittleEndian(
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> &bytes,
    std::uint32_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> DecodePlanPayload(
    ResourceDecodePlanAssetClass asset_class,
    std::uint32_t header_version,
    std::uint32_t expected_decoded_byte_count) {
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> bytes{};
    bytes[0U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
    bytes[1U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
    bytes[2U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
    bytes[3U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
    WriteU32LittleEndian(bytes, 4U, header_version);
    WriteU32LittleEndian(bytes, 8U, static_cast<std::uint32_t>(asset_class));
    WriteU32LittleEndian(bytes, 12U, DECODE_PLAN_PAYLOAD_BYTE_COUNT);
    WriteU32LittleEndian(bytes, 16U, expected_decoded_byte_count);
    bytes[RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT] = 0x7AU;
    return bytes;
}

ResourceDecodePlanRequest DecodePlanRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    ResourceDecodePlanAssetClass asset_class,
    std::uint32_t expected_decoded_byte_count) {
    ResourceDecodePlanRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.payload_id = payload_id;
    request.decode_plan_id = decode_plan_id;
    request.asset_class = asset_class;
    request.source_byte_count = DECODE_PLAN_PAYLOAD_BYTE_COUNT;
    request.expected_decoded_byte_count = expected_decoded_byte_count;
    return request;
}

bool StoreDecodePlanPayload(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> &payload) {
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        resource,
        expected_type,
        payload_id,
        payload.data(),
        static_cast<std::uint32_t>(payload.size()));
    return registry.StoreCachePayload(request) == ResourceCachePayloadStatus::Success;
}

bool CommitLoad(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState load_state,
    std::uint64_t commit_id,
    std::uint64_t upload_id) {
    const ResourceLoadCommitRequest request = LoadCommitRequest(
        resource,
        expected_type,
        load_state,
        commit_id,
        upload_id);
    return registry.CommitUploadCompletion(request) == ResourceLoadCommitStatus::Success;
}

bool ConfigureResidencyBudget(ResourceRegistry &registry, std::uint32_t byte_capacity) {
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = byte_capacity;
    return registry.SetResidencyBudget(budget) == ResourceResidencyStatus::Success;
}

bool ConfigureCachePayloadBudget(ResourceRegistry &registry, std::uint32_t byte_capacity) {
    ResourceCachePayloadBudgetDesc budget;
    budget.byte_capacity = byte_capacity;
    return registry.SetCachePayloadBudget(budget) == ResourceCachePayloadStatus::Success;
}

bool AdmitUploadedResident(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t commit_id,
    std::uint64_t upload_id) {
    if (!ConfigureResidencyBudget(registry, 256U)) {
        return false;
    }

    if (!CommitLoad(registry, resource, expected_type, ResourceLoadState::Uploaded, commit_id, upload_id)) {
        return false;
    }

    const ResourceResidencyRequest request = ResidencyRequest(resource, expected_type);
    return registry.AdmitResident(request) == ResourceResidencyStatus::Success;
}

bool BytesMatch(
    const std::uint8_t *left,
    const std::uint8_t *right,
    std::uint32_t byte_count) {
    std::uint32_t byte_index = 0U;
    while (byte_index < byte_count) {
        if (left[byte_index] != right[byte_index]) {
            return false;
        }

        ++byte_index;
    }

    return true;
}

bool SnapshotsMatch(const ResourceSnapshot& left, const ResourceSnapshot& right) {
    if (left.registered_resource_count != right.registered_resource_count) {
        return false;
    }

    if (left.acquired_handle_count != right.acquired_handle_count) {
        return false;
    }

    if (left.released_handle_count != right.released_handle_count) {
        return false;
    }

    if (left.dependency_edge_count != right.dependency_edge_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

int ResourceRegisterSyntheticDescriptorReturnsGenerationHandle() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("synthetic descriptor did not register");
    }

    if (result.handle.slot != 0U) {
        return Fail("first resource used unexpected slot");
    }

    if (result.handle.generation == INVALID_GENERATION) {
        return Fail("resource handle generation was invalid");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_resource_count != 1U) {
        return Fail("registered resource count was not recorded");
    }

    if (snapshot.type_count != 1U) {
        return Fail("resource type count was not recorded");
    }

    if (snapshot.last_status != ResourceStatus::Success) {
        return Fail("successful registration did not record success status");
    }

    return 0;
}

int ResourceRegisterDuplicateReturnsExplicitStatus() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!first_result.Succeeded()) {
        return Fail("first registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult duplicate_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (duplicate_result.status != ResourceStatus::DuplicateResource) {
        return Fail("duplicate resource did not return explicit status");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("duplicate resource changed registered count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("duplicate resource changed type count");
    }

    return 0;
}

int ResourceRegistryRejectsCapacityOverflowWithoutMutation() {
    ResourceRegistry registry(ResourceRegistryDesc{1U, 2U, 2U});
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!first_result.Succeeded()) {
        return Fail("first registration failed before capacity");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult overflow_result = Register(registry, TYPE_MATERIAL, "material_a");
    if (overflow_result.status != ResourceStatus::CapacityExceeded) {
        return Fail("resource capacity overflow did not return explicit status");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("resource capacity overflow changed registered count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("resource capacity overflow changed type count");
    }

    return 0;
}

int ResourceTypeCapacityOverflowDoesNotMutate() {
    ResourceRegistry registry(ResourceRegistryDesc{4U, 1U, 2U});
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_TEXTURE_KEY);
    if (!first_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_FIRST_REGISTRATION_FAILED);
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult overflow_result = Register(registry, TYPE_MATERIAL, TYPE_CAPACITY_MATERIAL_KEY);
    if (overflow_result.status != ResourceStatus::CapacityExceeded) {
        return Fail(TYPE_CAPACITY_STATUS_FAILED);
    }

    if (overflow_result.handle.IsValid()) {
        return Fail(TYPE_CAPACITY_VALID_HANDLE_FAILED);
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail(TYPE_CAPACITY_TYPE_COUNT_FAILED);
    }

    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail(TYPE_CAPACITY_REGISTERED_COUNT_FAILED);
    }

    if (after_snapshot.acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail(TYPE_CAPACITY_ACQUIRED_COUNT_FAILED);
    }

    const ResourceRegistrationResult retry_result = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_RETRY_TEXTURE_KEY);
    if (!retry_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_REGISTRATION_FAILED);
    }

    if (retry_result.handle.slot != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_FAILED);
    }

    if (retry_result.handle.generation != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_FAILED);
    }

    if (registry.Snapshot().type_count != before_snapshot.type_count) {
        return Fail(TYPE_CAPACITY_RETRY_TYPE_COUNT_FAILED);
    }

    return 0;
}

int ResourceHandleRejectsWrongGeneration() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const ResourceStatus retire_status = registry.Retire(result.handle);
    if (retire_status != ResourceStatus::Success) {
        return Fail("retire failed before stale handle check");
    }

    const ResourceStatus acquire_status = registry.Acquire(result.handle, TYPE_TEXTURE);
    if (acquire_status != ResourceStatus::GenerationMismatch) {
        return Fail("stale generation handle did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("stale handle acquire changed reference count");
    }

    return 0;
}

int ResourceHandleRejectsTypeMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.handle, TYPE_AUDIO);
    if (status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail("type mismatch changed reference count");
    }

    return 0;
}

int ResourceAcquireReleaseTracksReferenceCount() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("acquire did not increment reference count");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const ResourceSnapshot after_release_snapshot = registry.Snapshot();
    if (after_release_snapshot.acquired_handle_count != 0U) {
        return Fail("release did not decrement reference count");
    }

    if (after_release_snapshot.released_handle_count != 1U) {
        return Fail("release count was not recorded");
    }

    if (registry.Release(result.handle) != ResourceStatus::NotAcquired) {
        return Fail("release at zero did not return not-acquired status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("release at zero mutated reference count");
    }

    return 0;
}

int ResourceRepeatedAcquireIncrementsReferenceCount() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("first acquire failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("second acquire failed");
    }

    if (registry.Snapshot().acquired_handle_count != 2U) {
        return Fail("repeated acquire did not increment reference count");
    }

    return 0;
}

int ResourceAcquireRejectsReferenceCountOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = registry.RegisterSyntheticDescriptor(
        DescriptorWithReferenceCount(TYPE_TEXTURE, "texture_a", std::numeric_limits<std::uint32_t>::max()));
    if (!result.Succeeded()) {
        return Fail("overflow fixture registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.handle, TYPE_TEXTURE);
    if (status != ResourceStatus::ReferenceCountOverflow) {
        return Fail("reference count overflow did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail("reference count overflow changed acquired count");
    }

    return 0;
}

int ResourceValidateAcquireChecksHandleTypeAndOverflowWithoutMutation() {
    ResourceRegistry registry;
    const std::uint32_t max_reference_count = std::numeric_limits<std::uint32_t>::max();
    const std::uint32_t initial_reference_count = max_reference_count - 1U;
    const ResourceRegistrationResult result = registry.RegisterSyntheticDescriptor(
        DescriptorWithReferenceCount(TYPE_TEXTURE, "texture_a", initial_reference_count));
    if (!result.Succeeded()) {
        return Fail("validate acquire fixture registration failed");
    }

    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceStatus success_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 1U);
    if (success_status != ResourceStatus::Success) {
        return Fail("validate acquire rejected valid projected acquire");
    }

    if (!SnapshotsMatch(before_success_snapshot, registry.Snapshot())) {
        return Fail("validate acquire success mutated registry");
    }

    const ResourceSnapshot before_type_snapshot = registry.Snapshot();
    const ResourceStatus type_status = registry.ValidateAcquire(result.handle, TYPE_AUDIO, 1U);
    if (type_status != ResourceStatus::TypeMismatch) {
        return Fail("validate acquire type mismatch returned wrong status");
    }

    if (!SnapshotsMatch(before_type_snapshot, registry.Snapshot())) {
        return Fail("validate acquire type mismatch mutated registry");
    }

    const ResourceSnapshot before_overflow_snapshot = registry.Snapshot();
    const ResourceStatus overflow_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 2U);
    if (overflow_status != ResourceStatus::ReferenceCountOverflow) {
        return Fail("validate acquire overflow returned wrong status");
    }

    if (!SnapshotsMatch(before_overflow_snapshot, registry.Snapshot())) {
        return Fail("validate acquire overflow mutated registry");
    }

    return 0;
}

int ResourceValidateAcquireDoesNotRequireWorld() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_world_free");
    if (!result.Succeeded()) {
        return Fail("validate acquire world-free registration failed");
    }

    const ResourceSnapshot before_validate_snapshot = registry.Snapshot();
    const ResourceStatus validate_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 1U);
    if (validate_status != ResourceStatus::Success) {
        return Fail("validate acquire world-free validation failed");
    }

    if (!SnapshotsMatch(before_validate_snapshot, registry.Snapshot())) {
        return Fail("validate acquire world-free mutated registry");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("validate acquire world-free acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("validate acquire world-free release failed");
    }

    return 0;
}

int ResourceRetireRejectsOutstandingAcquire() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed before retire");
    }

    const ResourceStatus retire_status = registry.Retire(result.handle);
    if (retire_status != ResourceStatus::StillReferenced) {
        return Fail("retire with outstanding acquire did not return explicit status");
    }

    if (registry.Snapshot().registered_resource_count != 1U) {
        return Fail("rejected retire changed registered count");
    }

    return 0;
}

int ResourceRetireRejectsLiveDependentEdge() {
    ResourceRegistry registry;
    const ResourceRegistrationResult dependency = Register(registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependency.Succeeded()) {
        return Fail("dependency registration failed");
    }

    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    if (registry.AddDependency(dependent.handle, dependency.handle) != ResourceStatus::Success) {
        return Fail("dependency edge registration failed");
    }

    if (registry.Retire(dependency.handle) != ResourceStatus::StillDependedOn) {
        return Fail("retire with live dependent edge did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != 1U) {
        return Fail("rejected dependency retire changed edge count");
    }

    if (registry.Retire(dependent.handle) != ResourceStatus::Success) {
        return Fail("dependent retire failed");
    }

    if (registry.Snapshot().dependency_edge_count != 0U) {
        return Fail("successful retire did not clear outbound dependency edge");
    }

    if (registry.Retire(dependency.handle) != ResourceStatus::Success) {
        return Fail("dependency retire failed after outbound edge cleared");
    }

    return 0;
}

int ResourceDependencyValidationRejectsMissingDependency() {
    ResourceRegistry registry;
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    const ResourceStatus status = registry.AddDependency(dependent.handle, ResourceHandle{31U, 1U});
    if (status != ResourceStatus::DependencyMissing) {
        return Fail("missing dependency did not return explicit status");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.dependency_edge_count != 0U) {
        return Fail("missing dependency changed edge count");
    }

    if (snapshot.dependency_validation_count != 1U) {
        return Fail("missing dependency validation was not counted");
    }

    return 0;
}

int ResourceDependencyValidationRejectsCycle() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first = Register(registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second = Register(registry, TYPE_MATERIAL, "material_a");
    const ResourceRegistrationResult third = Register(registry, TYPE_EFFECT, "effect_a");
    if (!first.Succeeded()) {
        return Fail("first registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second registration failed");
    }

    if (!third.Succeeded()) {
        return Fail("third registration failed");
    }

    if (registry.AddDependency(first.handle, first.handle) != ResourceStatus::DependencyCycle) {
        return Fail("self dependency did not return cycle status");
    }

    if (registry.AddDependency(first.handle, second.handle) != ResourceStatus::Success) {
        return Fail("first dependency edge failed");
    }

    if (registry.AddDependency(second.handle, third.handle) != ResourceStatus::Success) {
        return Fail("second dependency edge failed");
    }

    const ResourceStatus cycle_status = registry.AddDependency(third.handle, first.handle);
    if (cycle_status != ResourceStatus::DependencyCycle) {
        return Fail("dependency cycle did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != 2U) {
        return Fail("dependency cycle changed edge count");
    }

    return 0;
}

int ResourceNoFileOrPackageDependencyForHandleRegistry() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "synthetic_texture");
    if (!result.Succeeded()) {
        return Fail("synthetic resource registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("synthetic handle acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("synthetic handle release failed");
    }

    if (registry.Snapshot().registered_resource_count != 1U) {
        return Fail("synthetic handle registry left resource scope");
    }

    return 0;
}

int ResourceDisabledDiagnosticsDoesNotChangeResults() {
    ResourceRegistry recording_registry;
    ResourceRegistry diagnostics_disabled_registry;
    const ResourceRegistrationResult recording_result = Register(recording_registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult disabled_result = Register(diagnostics_disabled_registry, TYPE_TEXTURE, "texture_a");
    if (!recording_result.Succeeded()) {
        return Fail("recording registry registration failed");
    }

    if (!disabled_result.Succeeded()) {
        return Fail("disabled registry registration failed");
    }

    const ResourceStatus recording_acquire = recording_registry.Acquire(recording_result.handle, TYPE_TEXTURE);
    const ResourceStatus disabled_acquire = diagnostics_disabled_registry.Acquire(disabled_result.handle, TYPE_TEXTURE);
    if (recording_acquire != disabled_acquire) {
        return Fail("disabled diagnostics changed acquire status");
    }

    const ResourceStatus recording_mismatch = recording_registry.Acquire(recording_result.handle, TYPE_AUDIO);
    const ResourceStatus disabled_mismatch = diagnostics_disabled_registry.Acquire(disabled_result.handle, TYPE_AUDIO);
    if (recording_mismatch != disabled_mismatch) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!SnapshotsMatch(recording_registry.Snapshot(), diagnostics_disabled_registry.Snapshot())) {
        return Fail("disabled diagnostics changed resource snapshot");
    }

    return 0;
}

int ResourceNoHiddenAllocationUsesYuMemorySignal() {
    ResourceRegistry registry;
    const ResourceSnapshot initial_snapshot = registry.Snapshot();
    if (initial_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry did not expose YuMemory accounting vocabulary");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.resource_capacity != initial_snapshot.resource_capacity) {
        return Fail("resource capacity changed during handle fixture");
    }

    if (after_snapshot.dependency_edge_capacity != initial_snapshot.dependency_edge_capacity) {
        return Fail("dependency edge capacity changed during handle fixture");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry changed allocation accounting vocabulary");
    }

    return 0;
}

int ResourceLoadCommitUploadSuccessSetsTerminalState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_load_success");
    if (!result.Succeeded()) {
        return Fail("load commit fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::Success) {
        return Fail("load commit did not accept successful upload");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("successful load commit did not set uploaded state");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.load_commit_count != 1U) {
        return Fail("load commit count was not tracked");
    }

    if (snapshot.loaded_resource_count != 1U) {
        return Fail("loaded resource count was not tracked");
    }

    if (snapshot.last_load_state != ResourceLoadState::Uploaded) {
        return Fail("last load state did not report uploaded");
    }

    return 0;
}

int ResourceLoadCommitFailedUploadSetsFailedState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_load_failed");
    if (!result.Succeeded()) {
        return Fail("failed load fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::Success) {
        return Fail("load commit did not accept failed upload");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed)) {
        return Fail("failed load commit did not set failed state");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.failed_resource_count != 1U) {
        return Fail("failed resource count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitRejectsInvalidHandleWithoutSlotMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_invalid_handle_guard");
    if (!result.Succeeded()) {
        return Fail("invalid handle fixture registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceLoadCommitRequest request = LoadCommitRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::InvalidHandle) {
        return Fail("invalid handle load commit returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("invalid handle load commit changed live slot state");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.load_commit_count != before_snapshot.load_commit_count) {
        return Fail("invalid handle load commit changed committed count");
    }

    if (after_snapshot.load_commit_record_count != before_snapshot.load_commit_record_count) {
        return Fail("invalid handle load commit stored a record");
    }

    return 0;
}

int ResourceLoadCommitRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_type_guard");
    if (!result.Succeeded()) {
        return Fail("type mismatch fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_AUDIO,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::TypeMismatch) {
        return Fail("type mismatch load commit returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("type mismatch load commit changed load state");
    }

    if (registry.Snapshot().load_commit_count != 0U) {
        return Fail("type mismatch load commit changed committed count");
    }

    return 0;
}

int ResourceLoadCommitRejectsDuplicateCommitId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_duplicate_commit");
    if (!result.Succeeded()) {
        return Fail("duplicate commit fixture registration failed");
    }

    const ResourceLoadCommitRequest first_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(first_request) != ResourceLoadCommitStatus::Success) {
        return Fail("first duplicate fixture commit failed");
    }

    const ResourceLoadCommitRequest duplicate_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_TWO);
    if (registry.CommitUploadCompletion(duplicate_request) != ResourceLoadCommitStatus::DuplicateCommitId) {
        return Fail("duplicate commit id returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("duplicate commit id changed terminal state");
    }

    if (registry.Snapshot().duplicate_load_commit_count != 1U) {
        return Fail("duplicate load commit count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitRejectsInvalidTransition() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_transition_guard");
    if (!result.Succeeded()) {
        return Fail("invalid transition fixture registration failed");
    }

    const ResourceLoadCommitRequest first_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(first_request) != ResourceLoadCommitStatus::Success) {
        return Fail("first transition fixture commit failed");
    }

    const ResourceLoadCommitRequest second_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_TWO,
        UPLOAD_TWO);
    if (registry.CommitUploadCompletion(second_request) != ResourceLoadCommitStatus::InvalidTransition) {
        return Fail("invalid transition returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("invalid transition changed terminal state");
    }

    if (registry.Snapshot().invalid_load_transition_count != 1U) {
        return Fail("invalid transition count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitSnapshotTracksCounters() {
    ResourceRegistry registry;
    const ResourceRegistrationResult uploaded_result = Register(registry, TYPE_TEXTURE, "texture_snapshot_uploaded");
    const ResourceRegistrationResult failed_result = Register(registry, TYPE_MATERIAL, "material_snapshot_failed");
    if (!uploaded_result.Succeeded()) {
        return Fail("snapshot uploaded registration failed");
    }

    if (!failed_result.Succeeded()) {
        return Fail("snapshot failed registration failed");
    }

    const ResourceLoadCommitRequest uploaded_request = LoadCommitRequest(
        uploaded_result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    const ResourceLoadCommitRequest failed_request = LoadCommitRequest(
        failed_result.handle,
        TYPE_MATERIAL,
        ResourceLoadState::Failed,
        COMMIT_TWO,
        UPLOAD_TWO);
    registry.CommitUploadCompletion(uploaded_request);
    registry.CommitUploadCompletion(failed_request);

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.load_commit_record_count != 2U) {
        return Fail("snapshot load commit record count changed");
    }

    if (snapshot.load_commit_count != 2U) {
        return Fail("snapshot load commit count changed");
    }

    if (snapshot.loaded_resource_count != 1U) {
        return Fail("snapshot loaded resource count changed");
    }

    if (snapshot.failed_resource_count != 1U) {
        return Fail("snapshot failed resource count changed");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("load commit snapshot changed allocation vocabulary");
    }

    return 0;
}

int ResourceResidencyAdmitsUploadedSlotWithinBudget() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    if (registry.SetResidencyBudget(budget) != ResourceResidencyStatus::Success) {
        return Fail("residency budget setup failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_resident");
    if (!result.Succeeded()) {
        return Fail("residency admission registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("residency admission upload commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("uploaded resource was not admitted as resident");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("admitted resource did not become evictable resident");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.resident_resource_count != 1U) {
        return Fail("resident resource count was not tracked");
    }

    if (snapshot.resident_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("resident byte count was not tracked");
    }

    if (snapshot.evictable_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("evictable byte count was not tracked");
    }

    return 0;
}

int ResourceResidencyRejectsUnloadedWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_unloaded_residency");
    if (!result.Succeeded()) {
        return Fail("unloaded residency fixture registration failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::NotUploaded) {
        return Fail("unloaded residency request returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("unloaded residency rejection changed load state");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("unloaded residency rejection changed load commits");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("unloaded residency rejection changed resident count");
    }

    return 0;
}

int ResourceResidencyRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_failed_residency");
    if (!result.Succeeded()) {
        return Fail("failed residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("failed residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::FailedLoad) {
        return Fail("failed load residency request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load residency rejection changed state");
    }

    if (registry.ResidencySnapshot().resident_resource_count != 0U) {
        return Fail("failed load residency rejection admitted resource");
    }

    return 0;
}

int ResourceResidencyRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("type mismatch residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_AUDIO);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::TypeMismatch) {
        return Fail("residency type mismatch returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("residency type mismatch changed state");
    }

    if (registry.ResidencySnapshot().resident_resource_count != 0U) {
        return Fail("residency type mismatch admitted resource");
    }

    return 0;
}

int ResourceResidencyRejectsDuplicateAdmission() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_duplicate");
    if (!result.Succeeded()) {
        return Fail("duplicate residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("duplicate residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    if (registry.AdmitResident(request) != ResourceResidencyStatus::Success) {
        return Fail("first residency admission failed");
    }

    const ResourceResidencySnapshot before_snapshot = registry.ResidencySnapshot();
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::AlreadyResident) {
        return Fail("duplicate residency admission returned wrong status");
    }

    const ResourceResidencySnapshot after_snapshot = registry.ResidencySnapshot();
    if (after_snapshot.resident_resource_count != before_snapshot.resident_resource_count) {
        return Fail("duplicate residency admission changed resident count");
    }

    if (after_snapshot.resident_byte_count != before_snapshot.resident_byte_count) {
        return Fail("duplicate residency admission changed resident bytes");
    }

    return 0;
}

int ResourceResidencyRejectsBudgetOverflow() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 32U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_budget");
    if (!result.Succeeded()) {
        return Fail("budget residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("budget residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::BudgetExceeded) {
        return Fail("residency budget overflow returned wrong status");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.budget_rejected_residency_count != 1U) {
        return Fail("residency budget rejection was not counted");
    }

    if (snapshot.resident_byte_count != 0U) {
        return Fail("residency budget overflow changed resident bytes");
    }

    return 0;
}

int ResourceResidencyPinUnpinTracksCounters() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_pin");
    if (!result.Succeeded()) {
        return Fail("pin residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("pin residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    if (registry.AdmitResident(request) != ResourceResidencyStatus::Success) {
        return Fail("pin residency admission failed");
    }

    if (registry.PinResident(request) != ResourceResidencyStatus::Success) {
        return Fail("pin resident failed");
    }

    ResourceResidencySnapshot pinned_snapshot = registry.ResidencySnapshot();
    if (pinned_snapshot.pinned_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("pin resident did not track pinned bytes");
    }

    if (pinned_snapshot.evictable_byte_count != 0U) {
        return Fail("pin resident left resource evictable");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Pinned)) {
        return Fail("pin resident did not set pinned state");
    }

    if (registry.UnpinResident(request) != ResourceResidencyStatus::Success) {
        return Fail("unpin resident failed");
    }

    const ResourceResidencySnapshot unpinned_snapshot = registry.ResidencySnapshot();
    if (unpinned_snapshot.pinned_byte_count != 0U) {
        return Fail("unpin resident left pinned bytes");
    }

    if (unpinned_snapshot.evictable_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("unpin resident did not restore evictable bytes");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("unpin resident did not restore evictable state");
    }

    return 0;
}

int ResourceResidencySelectsEvictionCandidateInSlotOrder() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 256U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult first = Register(registry, TYPE_TEXTURE, "texture_candidate_first");
    const ResourceRegistrationResult second = Register(registry, TYPE_TEXTURE, "texture_candidate_second");
    const ResourceRegistrationResult third = Register(registry, TYPE_TEXTURE, "texture_candidate_third");
    if (!first.Succeeded()) {
        return Fail("first candidate registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second candidate registration failed");
    }

    if (!third.Succeeded()) {
        return Fail("third candidate registration failed");
    }

    const bool first_commit = CommitLoad(
        registry,
        first.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    const bool second_commit = CommitLoad(
        registry,
        second.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_TWO,
        UPLOAD_TWO);
    const bool third_commit = CommitLoad(
        registry,
        third.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_THREE,
        UPLOAD_THREE);
    if (!first_commit) {
        return Fail("first candidate load commit failed");
    }

    if (!second_commit) {
        return Fail("second candidate load commit failed");
    }

    if (!third_commit) {
        return Fail("third candidate load commit failed");
    }

    const ResourceResidencyRequest first_request = ResidencyRequest(first.handle, TYPE_TEXTURE);
    const ResourceResidencyRequest second_request = ResidencyRequest(second.handle, TYPE_TEXTURE);
    const ResourceResidencyRequest third_request = ResidencyRequest(third.handle, TYPE_TEXTURE);
    registry.AdmitResident(first_request);
    registry.AdmitResident(second_request);
    registry.AdmitResident(third_request);
    registry.PinResident(first_request);
    registry.Acquire(second.handle, TYPE_TEXTURE);

    ResourceHandle candidate;
    const ResourceResidencyStatus status = registry.SelectEvictionCandidate(&candidate);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("eviction candidate selection failed");
    }

    if (candidate.slot != third.handle.slot) {
        return Fail("eviction candidate did not use deterministic slot order");
    }

    if (candidate.generation != third.handle.generation) {
        return Fail("eviction candidate returned wrong generation");
    }

    if (registry.ResidencySnapshot().eviction_candidate_count != 1U) {
        return Fail("eviction candidate selection was not counted");
    }

    return 0;
}

int ResourceResidencyReportsNoEvictionCandidate() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_no_candidate");
    if (!result.Succeeded()) {
        return Fail("no candidate fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("no candidate fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    registry.AdmitResident(request);
    registry.PinResident(request);

    ResourceHandle candidate;
    const ResourceResidencyStatus status = registry.SelectEvictionCandidate(&candidate);
    if (status != ResourceResidencyStatus::NoCandidate) {
        return Fail("no candidate selection returned wrong status");
    }

    if (candidate.IsValid()) {
        return Fail("no candidate selection returned a valid handle");
    }

    if (registry.ResidencySnapshot().eviction_candidate_miss_count != 1U) {
        return Fail("no candidate selection was not counted");
    }

    return 0;
}

int ResourceResidencyFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult dependency = Register(registry, TYPE_TEXTURE, "texture_validation_dependency");
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_validation_dependent");
    if (!dependency.Succeeded()) {
        return Fail("validation dependency registration failed");
    }

    if (!dependent.Succeeded()) {
        return Fail("validation dependent registration failed");
    }

    if (registry.AddDependency(dependent.handle, dependency.handle) != ResourceStatus::Success) {
        return Fail("validation dependency edge failed");
    }

    const bool committed = CommitLoad(
        registry,
        dependency.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("validation fixture load commit failed");
    }

    const ResourceResidencyRequest admit_request = ResidencyRequest(dependency.handle, TYPE_TEXTURE);
    registry.AdmitResident(admit_request);
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();

    const ResourceResidencyRequest invalid_request = ResidencyRequest(ResourceHandle{}, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(invalid_request);
    if (status != ResourceResidencyStatus::InvalidHandle) {
        return Fail("invalid residency request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid residency request changed registered count");
    }

    if (after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count) {
        return Fail("invalid residency request changed dependency edges");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid residency request changed load commits");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid residency request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid residency request changed resident bytes");
    }

    return 0;
}

int ResourceResidencyEvictsResourceOwnedStateOnly() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_evict");
    if (!result.Succeeded()) {
        return Fail("evict residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("evict residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    registry.AdmitResident(request);
    const ResourceResidencyStatus status = registry.EvictResident(request);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("resource-owned eviction state failed");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evicted)) {
        return Fail("resource-owned eviction did not set evicted state");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("resource-owned eviction changed load state");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.resident_byte_count != 0U) {
        return Fail("resource-owned eviction left resident bytes");
    }

    if (snapshot.evicted_resource_count != 1U) {
        return Fail("resource-owned eviction was not tracked");
    }

    return 0;
}

int ResourceCachePayloadStoresAndReadsResidentBytes() {
    ResourceRegistry registry;
    if (!ConfigureCachePayloadBudget(registry, 128U)) {
        return Fail("cache payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_store");
    if (!result.Succeeded()) {
        return Fail("cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{11U, 22U, 33U, 44U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest store_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload store failed");
    }

    const ResourceCachePayloadSnapshot store_snapshot = registry.CachePayloadSnapshot();
    if (store_snapshot.cached_byte_count != payload_byte_count) {
        return Fail("cache payload store did not track cached bytes");
    }

    if (store_snapshot.cached_payload_count != 1U) {
        return Fail("cache payload store did not track active payload count");
    }

    std::array<std::uint8_t, 4U> output{};
    const std::uint32_t output_byte_capacity = static_cast<std::uint32_t>(output.size());
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest read_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    const ResourceCachePayloadStatus read_status = registry.ReadCachePayload(
        read_request,
        output.data(),
        output_byte_capacity,
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload read failed");
    }

    if (output_byte_count != payload_byte_count) {
        return Fail("cache payload read returned wrong byte count");
    }

    if (!BytesMatch(payload.data(), output.data(), payload_byte_count)) {
        return Fail("cache payload read did not return stored bytes");
    }

    if (registry.CachePayloadSnapshot().read_payload_count != 1U) {
        return Fail("cache payload read was not counted");
    }

    return 0;
}

int ResourceCachePayloadReleaseClearsPayloadOnly() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_release");
    if (!result.Succeeded()) {
        return Fail("cache payload release fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("cache payload release fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{1U, 2U, 3U, 4U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(request) != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload release fixture store failed");
    }

    const ResourceCachePayloadRequest release_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    if (registry.ReleaseCachePayload(release_request) != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload release failed");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 0U) {
        return Fail("cache payload release left active payload");
    }

    if (cache_snapshot.cached_byte_count != 0U) {
        return Fail("cache payload release left cached bytes");
    }

    if (cache_snapshot.released_payload_count != 1U) {
        return Fail("cache payload release was not counted");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("cache payload release changed load state");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("cache payload release changed residency state");
    }

    return 0;
}

int ResourceCachePayloadRejectsNotResidentWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_not_resident");
    if (!result.Succeeded()) {
        return Fail("not resident cache payload fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("not resident cache payload fixture load commit failed");
    }

    const std::array<std::uint8_t, 4U> payload{{5U, 6U, 7U, 8U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::NotResident) {
        return Fail("not resident cache payload request returned wrong status");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 0U) {
        return Fail("not resident cache payload request stored payload");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("not resident cache payload request changed residency state");
    }

    return 0;
}

int ResourceCachePayloadRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_failed_load");
    if (!result.Succeeded()) {
        return Fail("failed load cache payload fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("failed load cache payload fixture load commit failed");
    }

    const std::array<std::uint8_t, 4U> payload{{9U, 10U, 11U, 12U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::FailedLoad) {
        return Fail("failed load cache payload request returned wrong status");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 0U) {
        return Fail("failed load cache payload request stored payload");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load cache payload request changed residency state");
    }

    return 0;
}

int ResourceCachePayloadRejectsStaleHandleWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_stale");
    if (!result.Succeeded()) {
        return Fail("stale cache payload fixture registration failed");
    }

    if (registry.Retire(result.handle) != ResourceStatus::Success) {
        return Fail("stale cache payload fixture retire failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const std::array<std::uint8_t, 4U> payload{{13U, 14U, 15U, 16U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::GenerationMismatch) {
        return Fail("stale cache payload request returned wrong status");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("stale cache payload request changed registered count");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 0U) {
        return Fail("stale cache payload request stored payload");
    }

    return 0;
}

int ResourceCachePayloadRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("type mismatch cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{17U, 18U, 19U, 20U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_AUDIO,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::TypeMismatch) {
        return Fail("type mismatch cache payload request returned wrong status");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 0U) {
        return Fail("type mismatch cache payload request stored payload");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("type mismatch cache payload request changed residency state");
    }

    return 0;
}

int ResourceCachePayloadRejectsDuplicatePayloadId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first = Register(registry, TYPE_TEXTURE, "texture_cache_duplicate_a");
    const ResourceRegistrationResult second = Register(registry, TYPE_TEXTURE, "texture_cache_duplicate_b");
    if (!first.Succeeded()) {
        return Fail("first duplicate cache payload fixture registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second duplicate cache payload fixture registration failed");
    }

    if (!ConfigureResidencyBudget(registry, 256U)) {
        return Fail("duplicate cache payload residency budget failed");
    }

    if (!CommitLoad(registry, first.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("first duplicate cache payload load commit failed");
    }

    if (!CommitLoad(registry, second.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_TWO, UPLOAD_TWO)) {
        return Fail("second duplicate cache payload load commit failed");
    }

    const ResourceResidencyRequest first_residency = ResidencyRequest(first.handle, TYPE_TEXTURE);
    const ResourceResidencyRequest second_residency = ResidencyRequest(second.handle, TYPE_TEXTURE);
    registry.AdmitResident(first_residency);
    registry.AdmitResident(second_residency);

    const std::array<std::uint8_t, 4U> first_payload{{21U, 22U, 23U, 24U}};
    const std::array<std::uint8_t, 4U> second_payload{{25U, 26U, 27U, 28U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(first_payload.size());
    const ResourceCachePayloadRequest first_request = CachePayloadRequest(
        first.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        first_payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(first_request) != ResourceCachePayloadStatus::Success) {
        return Fail("first duplicate cache payload store failed");
    }

    const ResourceCachePayloadRequest second_request = CachePayloadRequest(
        second.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        second_payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(second_request);
    if (status != ResourceCachePayloadStatus::DuplicatePayloadId) {
        return Fail("duplicate cache payload id returned wrong status");
    }

    const ResourceCachePayloadSnapshot snapshot = registry.CachePayloadSnapshot();
    if (snapshot.cached_payload_count != 1U) {
        return Fail("duplicate cache payload id changed active payload count");
    }

    if (snapshot.duplicate_payload_rejected_count != 1U) {
        return Fail("duplicate cache payload id was not counted");
    }

    return 0;
}

int ResourceCachePayloadRejectsCapacityOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_capacity");
    if (!result.Succeeded()) {
        return Fail("capacity cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("capacity cache payload fixture residency failed");
    }

    std::array<std::uint8_t, MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD + 1U> payload{};
    payload[0U] = 1U;
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::CapacityExceeded) {
        return Fail("capacity cache payload request returned wrong status");
    }

    const ResourceCachePayloadSnapshot snapshot = registry.CachePayloadSnapshot();
    if (snapshot.capacity_rejected_payload_count != 1U) {
        return Fail("capacity cache payload rejection was not counted");
    }

    if (snapshot.cached_payload_count != 0U) {
        return Fail("capacity cache payload request stored payload");
    }

    return 0;
}

int ResourceCachePayloadRejectsBudgetOverflow() {
    ResourceRegistry registry;
    if (!ConfigureCachePayloadBudget(registry, 3U)) {
        return Fail("budget cache payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_budget");
    if (!result.Succeeded()) {
        return Fail("budget cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("budget cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{29U, 30U, 31U, 32U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::BudgetExceeded) {
        return Fail("budget cache payload request returned wrong status");
    }

    const ResourceCachePayloadSnapshot snapshot = registry.CachePayloadSnapshot();
    if (snapshot.budget_rejected_payload_count != 1U) {
        return Fail("budget cache payload rejection was not counted");
    }

    if (snapshot.cached_byte_count != 0U) {
        return Fail("budget cache payload request stored bytes");
    }

    return 0;
}

int ResourceCachePayloadReadRejectsOutputBufferTooSmall() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_output");
    if (!result.Succeeded()) {
        return Fail("output cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("output cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{33U, 34U, 35U, 36U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest store_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("output cache payload fixture store failed");
    }

    std::array<std::uint8_t, 2U> output{};
    const std::uint32_t output_byte_capacity = static_cast<std::uint32_t>(output.size());
    std::uint32_t output_byte_count = 99U;
    const ResourceCachePayloadRequest read_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    const ResourceCachePayloadStatus status = registry.ReadCachePayload(
        read_request,
        output.data(),
        output_byte_capacity,
        &output_byte_count);
    if (status != ResourceCachePayloadStatus::OutputBufferTooSmall) {
        return Fail("small output cache payload read returned wrong status");
    }

    if (output_byte_count != 0U) {
        return Fail("small output cache payload read wrote byte count");
    }

    const ResourceCachePayloadSnapshot snapshot = registry.CachePayloadSnapshot();
    if (snapshot.cached_payload_count != 1U) {
        return Fail("small output cache payload read changed active payload count");
    }

    if (snapshot.read_payload_count != 0U) {
        return Fail("small output cache payload read was counted as success");
    }

    return 0;
}

int ResourceCachePayloadFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_validation");
    if (!result.Succeeded()) {
        return Fail("validation cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("validation cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{37U, 38U, 39U, 40U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest store_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("validation cache payload fixture store failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceCachePayloadSnapshot before_cache_snapshot = registry.CachePayloadSnapshot();
    const ResourceCachePayloadRequest invalid_request = CachePayloadRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(invalid_request);
    if (status != ResourceCachePayloadStatus::InvalidHandle) {
        return Fail("invalid cache payload request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid cache payload request changed registered count");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid cache payload request changed load commit count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid cache payload request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid cache payload request changed resident bytes");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("invalid cache payload request changed active payload count");
    }

    if (after_cache_snapshot.cached_byte_count != before_cache_snapshot.cached_byte_count) {
        return Fail("invalid cache payload request changed cached bytes");
    }

    std::array<std::uint8_t, 4U> output{};
    const std::uint32_t output_byte_capacity = static_cast<std::uint32_t>(output.size());
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest read_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    const ResourceCachePayloadStatus read_status = registry.ReadCachePayload(
        read_request,
        output.data(),
        output_byte_capacity,
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("valid cache payload was not readable after invalid request");
    }

    if (!BytesMatch(payload.data(), output.data(), payload_byte_count)) {
        return Fail("invalid cache payload request changed stored bytes");
    }

    return 0;
}

int ResourceCachePayloadReleaseRejectsPinnedWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_pinned");
    if (!result.Succeeded()) {
        return Fail("pinned cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("pinned cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{41U, 42U, 43U, 44U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceCachePayloadRequest store_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("pinned cache payload fixture store failed");
    }

    const ResourceResidencyRequest residency_request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    if (registry.PinResident(residency_request) != ResourceResidencyStatus::Success) {
        return Fail("pinned cache payload fixture pin failed");
    }

    const ResourceCachePayloadRequest release_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    const ResourceCachePayloadStatus status = registry.ReleaseCachePayload(release_request);
    if (status != ResourceCachePayloadStatus::Pinned) {
        return Fail("pinned cache payload release returned wrong status");
    }

    const ResourceCachePayloadSnapshot snapshot = registry.CachePayloadSnapshot();
    if (snapshot.cached_payload_count != 1U) {
        return Fail("pinned cache payload release changed active payload count");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Pinned)) {
        return Fail("pinned cache payload release changed residency state");
    }

    return 0;
}

int ResourceDecodePlanCreatesQueriesAndReleasesMetadata() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_success");
    if (!result.Succeeded()) {
        return Fail("decode plan success fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("decode plan success fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("decode plan success fixture cache payload store failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CreateDecodePlan(request) != ResourceDecodePlanStatus::Success) {
        return Fail("decode plan create failed");
    }

    ResourceDecodePlanRecord record;
    if (registry.QueryDecodePlan(request, &record) != ResourceDecodePlanStatus::Success) {
        return Fail("decode plan query failed");
    }

    if (record.decode_plan_id != DECODE_PLAN_ONE) {
        return Fail("decode plan query returned wrong id");
    }

    if (record.payload_id != PAYLOAD_ONE) {
        return Fail("decode plan query returned wrong payload id");
    }

    if (record.asset_class != ResourceDecodePlanAssetClass::Texture) {
        return Fail("decode plan query returned wrong asset class");
    }

    if (record.source_byte_count != DECODE_PLAN_PAYLOAD_BYTE_COUNT) {
        return Fail("decode plan query returned wrong source byte count");
    }

    if (record.expected_decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("decode plan query returned wrong decoded byte count");
    }

    ResourceDecodePlanSnapshot snapshot = registry.DecodePlanSnapshot();
    if (snapshot.active_plan_count != 1U) {
        return Fail("decode plan create did not record active plan count");
    }

    if (snapshot.planned_decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("decode plan create did not record decoded byte budget");
    }

    if (snapshot.created_plan_count != 1U) {
        return Fail("decode plan create was not counted");
    }

    if (snapshot.queried_plan_count != 1U) {
        return Fail("decode plan query was not counted");
    }

    if (registry.ReleaseDecodePlan(request) != ResourceDecodePlanStatus::Success) {
        return Fail("decode plan release failed");
    }

    snapshot = registry.DecodePlanSnapshot();
    if (snapshot.active_plan_count != 0U) {
        return Fail("decode plan release did not clear active plan count");
    }

    if (snapshot.planned_decoded_byte_count != 0U) {
        return Fail("decode plan release did not clear decoded byte budget");
    }

    if (snapshot.released_plan_count != 1U) {
        return Fail("decode plan release was not counted");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 1U) {
        return Fail("decode plan release changed cache payload count");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("decode plan release changed residency state");
    }

    const ResourceDecodePlanStatus query_status = registry.QueryDecodePlan(request, &record);
    if (query_status != ResourceDecodePlanStatus::MissingPlan) {
        return Fail("released decode plan query returned wrong status");
    }

    return 0;
}

int ResourceDecodePlanRejectsMissingCachePayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_missing");
    if (!result.Succeeded()) {
        return Fail("missing payload decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("missing payload decode plan fixture residency failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::MissingCachePayload) {
        return Fail("missing payload decode plan request returned wrong status");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 0U) {
        return Fail("missing payload decode plan request created a plan");
    }

    return 0;
}

int ResourceDecodePlanRejectsInvalidHeader() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_header");
    if (!result.Succeeded()) {
        return Fail("invalid header decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("invalid header decode plan fixture residency failed");
    }

    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    payload[0U] = 0U;
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("invalid header decode plan cache payload store failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::InvalidHeader) {
        return Fail("invalid header decode plan request returned wrong status");
    }

    if (registry.DecodePlanSnapshot().invalid_header_rejected_count != 1U) {
        return Fail("invalid header decode plan rejection was not counted");
    }

    return 0;
}

int ResourceDecodePlanRejectsUnsupportedHeaderVersion() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_version");
    if (!result.Succeeded()) {
        return Fail("unsupported version decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("unsupported version decode plan fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION + 1U,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("unsupported version decode plan cache payload store failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::UnsupportedHeaderVersion) {
        return Fail("unsupported version decode plan request returned wrong status");
    }

    if (registry.DecodePlanSnapshot().invalid_header_rejected_count != 1U) {
        return Fail("unsupported version decode plan rejection was not counted");
    }

    return 0;
}

int ResourceDecodePlanRejectsStaleHandleWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_stale");
    if (!result.Succeeded()) {
        return Fail("stale decode plan fixture registration failed");
    }

    if (registry.Retire(result.handle) != ResourceStatus::Success) {
        return Fail("stale decode plan fixture retire failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::GenerationMismatch) {
        return Fail("stale decode plan request returned wrong status");
    }

    if (registry.Snapshot().registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("stale decode plan request changed registered count");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 0U) {
        return Fail("stale decode plan request created a plan");
    }

    return 0;
}

int ResourceDecodePlanRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch decode plan fixture registration failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_AUDIO,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::TypeMismatch) {
        return Fail("type mismatch decode plan request returned wrong status");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 0U) {
        return Fail("type mismatch decode plan request created a plan");
    }

    return 0;
}

int ResourceDecodePlanRejectsNotResidentWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_not_resident");
    if (!result.Succeeded()) {
        return Fail("not resident decode plan fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("not resident decode plan fixture load commit failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::NotResident) {
        return Fail("not resident decode plan request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("not resident decode plan request changed residency state");
    }

    return 0;
}

int ResourceDecodePlanRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_failed");
    if (!result.Succeeded()) {
        return Fail("failed load decode plan fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("failed load decode plan fixture load commit failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::FailedLoad) {
        return Fail("failed load decode plan request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load decode plan request changed residency state");
    }

    return 0;
}

int ResourceDecodePlanRejectsDuplicatePlanId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_duplicate");
    if (!result.Succeeded()) {
        return Fail("duplicate decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("duplicate decode plan fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("duplicate decode plan cache payload store failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CreateDecodePlan(request) != ResourceDecodePlanStatus::Success) {
        return Fail("first duplicate decode plan create failed");
    }

    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::DuplicatePlanId) {
        return Fail("duplicate decode plan id returned wrong status");
    }

    const ResourceDecodePlanSnapshot snapshot = registry.DecodePlanSnapshot();
    if (snapshot.active_plan_count != 1U) {
        return Fail("duplicate decode plan id changed active count");
    }

    if (snapshot.duplicate_plan_rejected_count != 1U) {
        return Fail("duplicate decode plan id was not counted");
    }

    return 0;
}

int ResourceDecodePlanRejectsCapacityOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_capacity");
    if (!result.Succeeded()) {
        return Fail("capacity decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("capacity decode plan fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        1U);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("capacity decode plan cache payload store failed");
    }

    std::uint32_t plan_index = 0U;
    while (plan_index < MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT) {
        const ResourceDecodePlanRequest request = DecodePlanRequest(
            result.handle,
            TYPE_TEXTURE,
            PAYLOAD_ONE,
            DECODE_PLAN_ONE + plan_index,
            ResourceDecodePlanAssetClass::Texture,
            1U);
        if (registry.CreateDecodePlan(request) != ResourceDecodePlanStatus::Success) {
            return Fail("capacity decode plan setup create failed");
        }

        ++plan_index;
    }

    const ResourceDecodePlanRequest overflow_request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE + MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT,
        ResourceDecodePlanAssetClass::Texture,
        1U);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(overflow_request);
    if (status != ResourceDecodePlanStatus::CapacityExceeded) {
        return Fail("capacity decode plan request returned wrong status");
    }

    const ResourceDecodePlanSnapshot snapshot = registry.DecodePlanSnapshot();
    if (snapshot.active_plan_count != MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT) {
        return Fail("capacity decode plan request changed active plan count");
    }

    if (snapshot.capacity_rejected_plan_count != 1U) {
        return Fail("capacity decode plan rejection was not counted");
    }

    return 0;
}

int ResourceDecodePlanRejectsBudgetOverflow() {
    ResourceRegistry registry;
    ResourceDecodePlanBudgetDesc budget;
    budget.decoded_byte_capacity = 1U;
    if (registry.SetDecodePlanBudget(budget) != ResourceDecodePlanStatus::Success) {
        return Fail("budget decode plan budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_budget");
    if (!result.Succeeded()) {
        return Fail("budget decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("budget decode plan fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("budget decode plan cache payload store failed");
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(request);
    if (status != ResourceDecodePlanStatus::BudgetExceeded) {
        return Fail("budget decode plan request returned wrong status");
    }

    const ResourceDecodePlanSnapshot snapshot = registry.DecodePlanSnapshot();
    if (snapshot.active_plan_count != 0U) {
        return Fail("budget decode plan request created a plan");
    }

    if (snapshot.budget_rejected_plan_count != 1U) {
        return Fail("budget decode plan rejection was not counted");
    }

    return 0;
}

int ResourceDecodePlanFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_plan_validation");
    if (!result.Succeeded()) {
        return Fail("validation decode plan fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("validation decode plan fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, payload)) {
        return Fail("validation decode plan cache payload store failed");
    }

    const ResourceDecodePlanRequest valid_request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CreateDecodePlan(valid_request) != ResourceDecodePlanStatus::Success) {
        return Fail("validation decode plan create failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceCachePayloadSnapshot before_cache_snapshot = registry.CachePayloadSnapshot();
    const ResourceDecodePlanSnapshot before_decode_snapshot = registry.DecodePlanSnapshot();
    const ResourceDecodePlanRequest invalid_request = DecodePlanRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        DECODE_PLAN_TWO,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodePlanStatus status = registry.CreateDecodePlan(invalid_request);
    if (status != ResourceDecodePlanStatus::InvalidHandle) {
        return Fail("invalid decode plan request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid decode plan request changed registered count");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid decode plan request changed load commit count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid decode plan request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid decode plan request changed resident bytes");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("invalid decode plan request changed cache payload count");
    }

    if (after_cache_snapshot.cached_byte_count != before_cache_snapshot.cached_byte_count) {
        return Fail("invalid decode plan request changed cache payload bytes");
    }

    const ResourceDecodePlanSnapshot after_decode_snapshot = registry.DecodePlanSnapshot();
    if (after_decode_snapshot.active_plan_count != before_decode_snapshot.active_plan_count) {
        return Fail("invalid decode plan request changed active plan count");
    }

    if (after_decode_snapshot.planned_decoded_byte_count != before_decode_snapshot.planned_decoded_byte_count) {
        return Fail("invalid decode plan request changed planned decoded bytes");
    }

    ResourceDecodePlanRecord record;
    if (registry.QueryDecodePlan(valid_request, &record) != ResourceDecodePlanStatus::Success) {
        return Fail("valid decode plan was not queryable after invalid request");
    }

    if (record.decode_plan_id != DECODE_PLAN_ONE) {
        return Fail("invalid decode plan request changed existing plan");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const TestRegistry test_registry{
        {TEST_REGISTER, ResourceRegisterSyntheticDescriptorReturnsGenerationHandle},
        {TEST_DUPLICATE, ResourceRegisterDuplicateReturnsExplicitStatus},
        {TEST_CAPACITY, ResourceRegistryRejectsCapacityOverflowWithoutMutation},
        {TEST_TYPE_CAPACITY, ResourceTypeCapacityOverflowDoesNotMutate},
        {TEST_WRONG_GENERATION, ResourceHandleRejectsWrongGeneration},
        {TEST_TYPE_MISMATCH, ResourceHandleRejectsTypeMismatch},
        {TEST_ACQUIRE_RELEASE, ResourceAcquireReleaseTracksReferenceCount},
        {TEST_REPEATED_ACQUIRE, ResourceRepeatedAcquireIncrementsReferenceCount},
        {TEST_REFERENCE_OVERFLOW, ResourceAcquireRejectsReferenceCountOverflow},
        {TEST_VALIDATE_ACQUIRE, ResourceValidateAcquireChecksHandleTypeAndOverflowWithoutMutation},
        {TEST_VALIDATE_ACQUIRE_WORLD_FREE, ResourceValidateAcquireDoesNotRequireWorld},
        {TEST_RETIRE_REFERENCED, ResourceRetireRejectsOutstandingAcquire},
        {TEST_RETIRE_DEPENDED_ON, ResourceRetireRejectsLiveDependentEdge},
        {TEST_MISSING_DEPENDENCY, ResourceDependencyValidationRejectsMissingDependency},
        {TEST_DEPENDENCY_CYCLE, ResourceDependencyValidationRejectsCycle},
        {TEST_NO_FILE_PACKAGE, ResourceNoFileOrPackageDependencyForHandleRegistry},
        {TEST_DISABLED_DIAGNOSTICS, ResourceDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_HIDDEN_ALLOCATION, ResourceNoHiddenAllocationUsesYuMemorySignal},
        {TEST_LOAD_COMMIT_SUCCESS, ResourceLoadCommitUploadSuccessSetsTerminalState},
        {TEST_LOAD_COMMIT_FAILED_UPLOAD, ResourceLoadCommitFailedUploadSetsFailedState},
        {TEST_LOAD_COMMIT_INVALID_HANDLE, ResourceLoadCommitRejectsInvalidHandleWithoutSlotMutation},
        {TEST_LOAD_COMMIT_TYPE_MISMATCH, ResourceLoadCommitRejectsTypeMismatchWithoutMutation},
        {TEST_LOAD_COMMIT_DUPLICATE, ResourceLoadCommitRejectsDuplicateCommitId},
        {TEST_LOAD_COMMIT_INVALID_TRANSITION, ResourceLoadCommitRejectsInvalidTransition},
        {TEST_LOAD_COMMIT_SNAPSHOT, ResourceLoadCommitSnapshotTracksCounters},
        {TEST_RESIDENCY_ADMIT, ResourceResidencyAdmitsUploadedSlotWithinBudget},
        {TEST_RESIDENCY_UNLOADED, ResourceResidencyRejectsUnloadedWithoutMutation},
        {TEST_RESIDENCY_FAILED_LOAD, ResourceResidencyRejectsFailedLoadWithoutMutation},
        {TEST_RESIDENCY_TYPE_MISMATCH, ResourceResidencyRejectsTypeMismatchWithoutMutation},
        {TEST_RESIDENCY_DUPLICATE, ResourceResidencyRejectsDuplicateAdmission},
        {TEST_RESIDENCY_BUDGET, ResourceResidencyRejectsBudgetOverflow},
        {TEST_RESIDENCY_PIN_UNPIN, ResourceResidencyPinUnpinTracksCounters},
        {TEST_RESIDENCY_CANDIDATE, ResourceResidencySelectsEvictionCandidateInSlotOrder},
        {TEST_RESIDENCY_NO_CANDIDATE, ResourceResidencyReportsNoEvictionCandidate},
        {TEST_RESIDENCY_FAILED_VALIDATION, ResourceResidencyFailedValidationDoesNotMutateResourceState},
        {TEST_RESIDENCY_EVICT, ResourceResidencyEvictsResourceOwnedStateOnly},
        {TEST_CACHE_PAYLOAD_STORE_READ, ResourceCachePayloadStoresAndReadsResidentBytes},
        {TEST_CACHE_PAYLOAD_RELEASE, ResourceCachePayloadReleaseClearsPayloadOnly},
        {TEST_CACHE_PAYLOAD_NOT_RESIDENT, ResourceCachePayloadRejectsNotResidentWithoutMutation},
        {TEST_CACHE_PAYLOAD_FAILED_LOAD, ResourceCachePayloadRejectsFailedLoadWithoutMutation},
        {TEST_CACHE_PAYLOAD_STALE_HANDLE, ResourceCachePayloadRejectsStaleHandleWithoutMutation},
        {TEST_CACHE_PAYLOAD_TYPE_MISMATCH, ResourceCachePayloadRejectsTypeMismatchWithoutMutation},
        {TEST_CACHE_PAYLOAD_DUPLICATE, ResourceCachePayloadRejectsDuplicatePayloadId},
        {TEST_CACHE_PAYLOAD_CAPACITY, ResourceCachePayloadRejectsCapacityOverflow},
        {TEST_CACHE_PAYLOAD_BUDGET, ResourceCachePayloadRejectsBudgetOverflow},
        {TEST_CACHE_PAYLOAD_OUTPUT_SMALL, ResourceCachePayloadReadRejectsOutputBufferTooSmall},
        {TEST_CACHE_PAYLOAD_FAILED_VALIDATION, ResourceCachePayloadFailedValidationDoesNotMutateResourceState},
        {TEST_CACHE_PAYLOAD_PINNED_RELEASE, ResourceCachePayloadReleaseRejectsPinnedWithoutMutation},
        {TEST_DECODE_PLAN_CREATE_QUERY_RELEASE, ResourceDecodePlanCreatesQueriesAndReleasesMetadata},
        {TEST_DECODE_PLAN_MISSING_PAYLOAD, ResourceDecodePlanRejectsMissingCachePayload},
        {TEST_DECODE_PLAN_INVALID_HEADER, ResourceDecodePlanRejectsInvalidHeader},
        {TEST_DECODE_PLAN_UNSUPPORTED_VERSION, ResourceDecodePlanRejectsUnsupportedHeaderVersion},
        {TEST_DECODE_PLAN_STALE_HANDLE, ResourceDecodePlanRejectsStaleHandleWithoutMutation},
        {TEST_DECODE_PLAN_TYPE_MISMATCH, ResourceDecodePlanRejectsTypeMismatchWithoutMutation},
        {TEST_DECODE_PLAN_NOT_RESIDENT, ResourceDecodePlanRejectsNotResidentWithoutMutation},
        {TEST_DECODE_PLAN_FAILED_LOAD, ResourceDecodePlanRejectsFailedLoadWithoutMutation},
        {TEST_DECODE_PLAN_DUPLICATE, ResourceDecodePlanRejectsDuplicatePlanId},
        {TEST_DECODE_PLAN_CAPACITY, ResourceDecodePlanRejectsCapacityOverflow},
        {TEST_DECODE_PLAN_BUDGET, ResourceDecodePlanRejectsBudgetOverflow},
        {TEST_DECODE_PLAN_FAILED_VALIDATION, ResourceDecodePlanFailedValidationDoesNotMutateResourceState}};

    const std::string_view test_name(argv[1]);
    const auto test_entry = test_registry.find(test_name);
    if (test_entry == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_entry->second();
}
