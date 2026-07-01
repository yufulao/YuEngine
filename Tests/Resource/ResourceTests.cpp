// 模块：Tests Resource
// 文件：Tests/Resource/ResourceTests.cpp

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
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadOperation.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadSnapshot.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanRecord.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanSnapshot.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultRecord.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultSnapshot.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
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
using yuengine::resource::ResourceDependencyBatchResult;
using yuengine::resource::ResourceDependencyRequest;
using yuengine::resource::ResourceDescriptorBatchLookupResult;
using yuengine::resource::ResourceDescriptorBatchResult;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceDescriptorLookupQuery;
using yuengine::resource::ResourceDescriptorLookupRecord;
using yuengine::resource::ResourceDecodedPayloadBudgetDesc;
using yuengine::resource::ResourceDecodedPayloadOperation;
using yuengine::resource::ResourceDecodedPayloadRecord;
using yuengine::resource::ResourceDecodedPayloadRequest;
using yuengine::resource::ResourceDecodedPayloadSnapshot;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodePlanBudgetDesc;
using yuengine::resource::ResourceDecodePlanOperation;
using yuengine::resource::ResourceDecodePlanRecord;
using yuengine::resource::ResourceDecodePlanRequest;
using yuengine::resource::ResourceDecodePlanSnapshot;
using yuengine::resource::ResourceDecodePlanStatus;
using yuengine::resource::ResourceDecodeResultBudgetDesc;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodeResultRecord;
using yuengine::resource::ResourceDecodeResultRequest;
using yuengine::resource::ResourceDecodeResultSnapshot;
using yuengine::resource::ResourceDecodeResultStatus;
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
using yuengine::resource::MAX_LOGICAL_KEY_BYTES;
using yuengine::resource::MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT;
using yuengine::resource::MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT;
using yuengine::resource::MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT;
using yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD;
using yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT;
using yuengine::resource::MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES;
using yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD;
using yuengine::resource::MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_VERSION;

namespace {
constexpr const char* TEST_REGISTER = "Resource_RegisterSyntheticDescriptor_ReturnsGenerationHandle";
constexpr const char *TEST_DESCRIPTOR_BATCH =
    "Resource_DescriptorBatchRegistrationSubmitsRowsAndStopsOnFirstFailure";
constexpr const char *TEST_DESCRIPTOR_ENUMERATION =
    "Resource_DescriptorEnumerationReportsRegisteredSyntheticDescriptors";
constexpr const char *TEST_DESCRIPTOR_EXACT_LOOKUP =
    "Resource_DescriptorExactLookupFindsRegisteredSyntheticDescriptor";
constexpr const char *TEST_DESCRIPTOR_BATCH_EXACT_LOOKUP =
    "Resource_DescriptorBatchExactLookupReturnsAtomicRows";
constexpr const char* TEST_INVALID_DESCRIPTOR =
    "Resource_RegisterRejectsInvalidDescriptorWithoutMutation";
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
constexpr const char *TEST_DEPENDENCY_BATCH =
    "Resource_DependencyBatchSubmitsRowsAndStopsOnFirstFailure";
constexpr const char *TEST_DEPENDENCY_EDGE_EXACT_LOOKUP =
    "Resource_DependencyEdgeExactLookupFindsDirectEdge";
constexpr const char *TEST_DEPENDENCY_EDGE_COUNT =
    "Resource_DependencyEdgeCountSnapshotMatchesDirectEdges";
constexpr const char *TEST_DEPENDENCY_EDGE_ENUMERATION =
    "Resource_DependencyEdgeEnumerationReportsCommittedRows";
constexpr const char *TEST_DEPENDENCY_TRAVERSAL =
    "Resource_DependencyTraversalReturnsExplicitClosureHandles";
constexpr const char *TEST_DEPENDENCY_MULTIROOT_TRAVERSAL =
    "Resource_DependencyTraversalMultiRootDeduplicatesClosureHandles";
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
constexpr const char *TEST_LOAD_COMMIT_CAPACITY_ENTRY =
    "Resource_LoadCommit_CapacityEntryRecordsRejectedIdentity";
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
constexpr const char *TEST_CACHE_PAYLOAD_WINDOW_REFERENCE =
    "Resource_CachePayload_StoresWindowMetadataAndReferenceBudget";
constexpr const char *TEST_CACHE_PAYLOAD_U64_LOGICAL_WINDOW =
    "Resource_CachePayload_StoresU64LogicalWindowWithU32LocalBytes";
constexpr const char *TEST_CACHE_PAYLOAD_WINDOW_OVERFLOW =
    "Resource_CachePayload_RejectsWindowOverflowWithoutMutation";
constexpr const char *TEST_CACHE_PAYLOAD_REFERENCE_BUDGET =
    "Resource_CachePayload_RejectsReferenceBudgetWithoutMutation";
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
constexpr const char *TEST_DECODE_RESULT_COMMIT_QUERY_RELEASE =
    "Resource_DecodeResult_CommitsQueriesAndReleasesMetadata";
constexpr const char *TEST_DECODE_RESULT_MISSING_PLAN =
    "Resource_DecodeResult_RejectsMissingDecodePlan";
constexpr const char *TEST_DECODE_RESULT_STALE_HANDLE =
    "Resource_DecodeResult_RejectsStaleHandleWithoutMutation";
constexpr const char *TEST_DECODE_RESULT_TYPE_MISMATCH =
    "Resource_DecodeResult_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_DECODE_RESULT_NOT_RESIDENT =
    "Resource_DecodeResult_RejectsNotResidentWithoutMutation";
constexpr const char *TEST_DECODE_RESULT_FAILED_LOAD =
    "Resource_DecodeResult_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_DECODE_RESULT_DUPLICATE =
    "Resource_DecodeResult_RejectsDuplicateResultId";
constexpr const char *TEST_DECODE_RESULT_CAPACITY =
    "Resource_DecodeResult_RejectsCapacityOverflow";
constexpr const char *TEST_DECODE_RESULT_CAPACITY_ENTRY =
    "Resource_DecodeResult_CapacityEntryClearsOnNonCapacity";
constexpr const char *TEST_DECODE_RESULT_BUDGET =
    "Resource_DecodeResult_RejectsBudgetOverflow";
constexpr const char *TEST_DECODE_RESULT_ASSET_CLASS =
    "Resource_DecodeResult_RejectsAssetClassMismatch";
constexpr const char *TEST_DECODE_RESULT_RESULT_CLASS =
    "Resource_DecodeResult_RejectsResultClassMismatch";
constexpr const char *TEST_DECODE_RESULT_DECODED_BYTES =
    "Resource_DecodeResult_RejectsDecodedByteCountMismatch";
constexpr const char *TEST_DECODE_RESULT_PLAN_RELEASE =
    "Resource_DecodeResult_ReleasingPlanClearsDependentRecords";
constexpr const char *TEST_DECODE_RESULT_PAYLOAD_RELEASE =
    "Resource_DecodeResult_ReleasingPayloadClearsDependentRecords";
constexpr const char *TEST_DECODE_RESULT_FAILED_VALIDATION =
    "Resource_DecodeResult_FailedValidationDoesNotMutateResourceState";
constexpr const char *TEST_DECODED_PAYLOAD_STORE_READ =
    "Resource_DecodedPayload_StoresReadsQueriesAndReleasesBytes";
constexpr const char *TEST_DECODED_PAYLOAD_MISSING_CACHE =
    "Resource_DecodedPayload_RejectsMissingCachePayload";
constexpr const char *TEST_DECODED_PAYLOAD_MISSING_PLAN =
    "Resource_DecodedPayload_RejectsMissingDecodePlan";
constexpr const char *TEST_DECODED_PAYLOAD_MISSING_RESULT =
    "Resource_DecodedPayload_RejectsMissingDecodeResult";
constexpr const char *TEST_DECODED_PAYLOAD_NULL_INPUT =
    "Resource_DecodedPayload_RejectsNullInputBytes";
constexpr const char *TEST_DECODED_PAYLOAD_EMPTY =
    "Resource_DecodedPayload_RejectsEmptyPayload";
constexpr const char *TEST_DECODED_PAYLOAD_OUTPUT_SMALL =
    "Resource_DecodedPayload_ReadRejectsOutputBufferTooSmall";
constexpr const char *TEST_DECODED_PAYLOAD_WINDOW_REFERENCE =
    "Resource_DecodedPayload_StoresWindowMetadataAndReferenceBudget";
constexpr const char *TEST_DECODED_PAYLOAD_U64_LOGICAL_WINDOW =
    "Resource_DecodedPayload_StoresU64LogicalWindowWithU32LocalBytes";
constexpr const char *TEST_DECODED_PAYLOAD_WINDOW_MISMATCH =
    "Resource_DecodedPayload_RejectsWindowMismatchWithoutMutation";
constexpr const char *TEST_DECODED_PAYLOAD_REFERENCE_BUDGET =
    "Resource_DecodedPayload_RejectsReferenceBudgetWithoutMutation";
constexpr const char *TEST_DECODED_PAYLOAD_DUPLICATE =
    "Resource_DecodedPayload_RejectsDuplicatePayloadId";
constexpr const char *TEST_DECODED_PAYLOAD_CAPACITY =
    "Resource_DecodedPayload_RejectsCapacityOverflow";
constexpr const char *TEST_DECODED_PAYLOAD_CAPACITY_ENTRY =
    "Resource_DecodedPayload_CapacityEntryRecordsRejectedIdentity";
constexpr const char *TEST_DECODED_PAYLOAD_CAPACITY_ENTRY_LIMITS =
    "Resource_DecodedPayload_CapacityEntryRecordsAllCapacityLimits";
constexpr const char *TEST_DECODED_PAYLOAD_BUDGET =
    "Resource_DecodedPayload_RejectsBudgetOverflow";
constexpr const char *TEST_DECODED_PAYLOAD_ASSET_CLASS =
    "Resource_DecodedPayload_RejectsAssetClassMismatch";
constexpr const char *TEST_DECODED_PAYLOAD_RESULT_CLASS =
    "Resource_DecodedPayload_RejectsResultClassMismatch";
constexpr const char *TEST_DECODED_PAYLOAD_DECODED_BYTES =
    "Resource_DecodedPayload_RejectsDecodedByteCountMismatch";
constexpr const char *TEST_DECODED_PAYLOAD_TYPE_MISMATCH =
    "Resource_DecodedPayload_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_DECODED_PAYLOAD_NOT_RESIDENT =
    "Resource_DecodedPayload_RejectsNotResidentWithoutMutation";
constexpr const char *TEST_DECODED_PAYLOAD_FAILED_LOAD =
    "Resource_DecodedPayload_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_DECODED_PAYLOAD_RESULT_RELEASE =
    "Resource_DecodedPayload_ReleasingResultClearsPayload";
constexpr const char *TEST_DECODED_PAYLOAD_PLAN_RELEASE =
    "Resource_DecodedPayload_ReleasingPlanClearsPayload";
constexpr const char *TEST_DECODED_PAYLOAD_CACHE_RELEASE =
    "Resource_DecodedPayload_ReleasingCachePayloadClearsPayload";
constexpr const char *TEST_DECODED_PAYLOAD_FAILED_VALIDATION =
    "Resource_DecodedPayload_FailedValidationDoesNotMutateResourceState";
constexpr const char *TEST_PAYLOAD_WINDOW_REFERENCE_STABILITY =
    "Resource_PayloadWindow_DoesNotChangeResourceReferenceCountOrResidency";
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
constexpr std::uint64_t DECODE_PLAN_THREE = 6003U;
constexpr std::uint64_t DECODE_RESULT_ONE = 7001U;
constexpr std::uint64_t DECODE_RESULT_TWO = 7002U;
constexpr std::uint64_t DECODED_PAYLOAD_ONE = 8001U;
constexpr std::uint64_t DECODED_PAYLOAD_TWO = 8002U;
constexpr std::uint64_t U64_LOGICAL_PAYLOAD_BYTE_COUNT = 0x100001000ULL;
constexpr std::uint64_t U64_PAYLOAD_WINDOW_BYTE_OFFSET = 0x100000040ULL;
constexpr std::uint32_t DECODE_PLAN_DECODED_BYTE_COUNT = 128U;
constexpr std::uint32_t DECODE_PLAN_PAYLOAD_BYTE_COUNT = RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT + 4U;
constexpr std::uint32_t DECODED_PAYLOAD_BYTE_COUNT = DECODE_PLAN_DECODED_BYTE_COUNT;
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

bool DescriptorMatches(
    const ResourceDescriptor &descriptor,
    ResourceTypeId type,
    const char *key,
    std::uint32_t reference_count) {
    if (descriptor.type.value != type.value) {
        return false;
    }

    const ResourceLogicalKey logical_key(key);
    if (!descriptor.logical_key.Equals(logical_key)) {
        return false;
    }

    return descriptor.initial_reference_count == reference_count;
}

ResourceDescriptorLookupQuery DescriptorLookupQuery(ResourceTypeId type, const char *key) {
    return ResourceDescriptorLookupQuery{type, ResourceLogicalKey(key)};
}

bool LookupRecordMatches(
    const ResourceDescriptorLookupRecord &record,
    ResourceHandle handle,
    ResourceTypeId type,
    const char *key,
    std::uint32_t reference_count) {
    if (record.handle.slot != handle.slot) {
        return false;
    }

    if (record.handle.generation != handle.generation) {
        return false;
    }

    return DescriptorMatches(record.descriptor, type, key, reference_count);
}

bool LookupRecordEquals(
    const ResourceDescriptorLookupRecord &left,
    const ResourceDescriptorLookupRecord &right) {
    if (left.handle.slot != right.handle.slot) {
        return false;
    }

    if (left.handle.generation != right.handle.generation) {
        return false;
    }

    if (left.descriptor.type.value != right.descriptor.type.value) {
        return false;
    }

    if (!left.descriptor.logical_key.Equals(right.descriptor.logical_key)) {
        return false;
    }

    return left.descriptor.initial_reference_count == right.descriptor.initial_reference_count;
}

ResourceRegistrationResult Register(ResourceRegistry& registry, ResourceTypeId type, const char* key) {
    return registry.RegisterSyntheticDescriptor(Descriptor(type, key));
}

bool InvalidDescriptorRegistrationFailsWithoutMutation(
    ResourceRegistry& registry,
    const ResourceDescriptor& descriptor) {
    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult result = registry.RegisterSyntheticDescriptor(descriptor);
    if (result.status != ResourceStatus::InvalidDescriptor) {
        return false;
    }

    if (result.handle.IsValid()) {
        return false;
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return false;
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return false;
    }

    if (after_snapshot.acquired_handle_count != before_snapshot.acquired_handle_count) {
        return false;
    }

    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count + 1U) {
        return false;
    }

    return after_snapshot.last_status == ResourceStatus::InvalidDescriptor;
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

int ExpectLoadCommitCapacityEntryCleared(const ResourceSnapshot &snapshot) {
    if (snapshot.last_required_load_commit_count != 0U) {
        return Fail("load commit capacity entry kept required count");
    }

    if (snapshot.last_failed_load_commit_resource.IsValid()) {
        return Fail("load commit capacity entry kept resource");
    }

    if (snapshot.last_failed_load_commit_type.IsValid()) {
        return Fail("load commit capacity entry kept type");
    }

    if (snapshot.last_failed_load_commit_id != 0U) {
        return Fail("load commit capacity entry kept commit id");
    }

    if (snapshot.last_failed_load_upload_id != 0U) {
        return Fail("load commit capacity entry kept upload id");
    }

    if (snapshot.last_failed_load_staging_request_id != 0U) {
        return Fail("load commit capacity entry kept staging request id");
    }

    if (snapshot.last_failed_load_upload_byte_count != 0U) {
        return Fail("load commit capacity entry kept upload byte count");
    }

    if (snapshot.last_failed_load_state != ResourceLoadState::Unloaded) {
        return Fail("load commit capacity entry kept load state");
    }

    if (snapshot.last_failed_load_commit_capacity != 0U) {
        return Fail("load commit capacity entry kept capacity");
    }

    if (snapshot.last_failed_load_commit_count != 0U) {
        return Fail("load commit capacity entry kept current count");
    }

    if (snapshot.last_failed_required_load_commit_count != 0U) {
        return Fail("load commit capacity entry kept failed required count");
    }

    return 0;
}

int ExpectLoadCommitCapacityEntryMatches(
    const ResourceSnapshot &snapshot,
    const ResourceLoadCommitRequest &request,
    std::uint32_t current_load_commit_count,
    std::uint32_t required_load_commit_count) {
    if (snapshot.last_required_load_commit_count != required_load_commit_count) {
        return Fail("load commit capacity entry reported wrong required count");
    }

    if (snapshot.last_failed_required_load_commit_count != required_load_commit_count) {
        return Fail("load commit capacity entry stored wrong required count");
    }

    if (snapshot.last_failed_load_commit_resource.slot != request.resource.slot) {
        return Fail("load commit capacity entry stored wrong resource slot");
    }

    if (snapshot.last_failed_load_commit_resource.generation != request.resource.generation) {
        return Fail("load commit capacity entry stored wrong resource generation");
    }

    if (snapshot.last_failed_load_commit_type.value != request.expected_type.value) {
        return Fail("load commit capacity entry stored wrong type");
    }

    if (snapshot.last_failed_load_commit_id != request.commit_id) {
        return Fail("load commit capacity entry stored wrong commit id");
    }

    if (snapshot.last_failed_load_upload_id != request.upload_id) {
        return Fail("load commit capacity entry stored wrong upload id");
    }

    if (snapshot.last_failed_load_staging_request_id != request.staging_request_id) {
        return Fail("load commit capacity entry stored wrong staging request id");
    }

    if (snapshot.last_failed_load_upload_byte_count != request.upload_byte_count) {
        return Fail("load commit capacity entry stored wrong upload byte count");
    }

    if (snapshot.last_failed_load_state != request.load_state) {
        return Fail("load commit capacity entry stored wrong load state");
    }

    if (snapshot.last_failed_load_commit_capacity != MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT) {
        return Fail("load commit capacity entry stored wrong capacity");
    }

    if (snapshot.last_failed_load_commit_count != current_load_commit_count) {
        return Fail("load commit capacity entry stored wrong current count");
    }

    return 0;
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

ResourceCachePayloadRequest CachePayloadWindowRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    const std::uint8_t *payload_bytes,
    std::uint32_t payload_byte_count,
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size) {
    ResourceCachePayloadRequest request = CachePayloadRequest(
        resource,
        expected_type,
        payload_id,
        payload_bytes,
        payload_byte_count);
    request.payload_window_byte_offset = payload_window_byte_offset;
    request.payload_window_byte_size = payload_window_byte_size;
    return request;
}

bool ResourceHandlesMatch(ResourceHandle left, ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool CachePayloadFailedEntryIsClear(const ResourceCachePayloadSnapshot &snapshot) {
    if (snapshot.last_failed_resource.IsValid()) {
        return false;
    }

    if (snapshot.last_failed_expected_type.IsValid()) {
        return false;
    }

    if (snapshot.last_failed_payload_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_logical_byte_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_window_byte_offset != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_window_byte_size != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_byte_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_byte_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_reference_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_payload_current_reference_count != 0U) {
        return false;
    }

    return snapshot.last_failed_payload_reference_count == 0U;
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

ResourceDecodeResultRequest DecodeResultRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    ResourceDecodePlanAssetClass asset_class,
    ResourceDecodeResultClass result_class,
    std::uint32_t decoded_byte_count) {
    ResourceDecodeResultRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.payload_id = payload_id;
    request.decode_plan_id = decode_plan_id;
    request.decode_result_id = decode_result_id;
    request.asset_class = asset_class;
    request.result_class = result_class;
    request.decoded_byte_count = decoded_byte_count;
    return request;
}

std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> DecodedPayloadBytes(std::uint8_t seed) {
    std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes{};
    std::uint32_t byte_index = 0U;
    while (byte_index < DECODED_PAYLOAD_BYTE_COUNT) {
        bytes[byte_index] = static_cast<std::uint8_t>(seed + static_cast<std::uint8_t>(byte_index));
        ++byte_index;
    }

    return bytes;
}

ResourceDecodedPayloadRequest DecodedPayloadRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    std::uint64_t decoded_payload_id,
    ResourceDecodePlanAssetClass asset_class,
    ResourceDecodeResultClass result_class,
    const std::uint8_t *decoded_bytes,
    std::uint32_t decoded_byte_count) {
    ResourceDecodedPayloadRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.payload_id = payload_id;
    request.decode_plan_id = decode_plan_id;
    request.decode_result_id = decode_result_id;
    request.decoded_payload_id = decoded_payload_id;
    request.asset_class = asset_class;
    request.result_class = result_class;
    request.decoded_bytes = decoded_bytes;
    request.decoded_byte_count = decoded_byte_count;
    return request;
}

ResourceDecodedPayloadRequest DecodedPayloadWindowRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    std::uint64_t decoded_payload_id,
    ResourceDecodePlanAssetClass asset_class,
    ResourceDecodeResultClass result_class,
    const std::uint8_t *decoded_bytes,
    std::uint32_t decoded_byte_count,
    std::uint64_t payload_window_byte_offset,
    std::uint64_t payload_window_byte_size) {
    ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        resource,
        expected_type,
        payload_id,
        decode_plan_id,
        decode_result_id,
        decoded_payload_id,
        asset_class,
        result_class,
        decoded_bytes,
        decoded_byte_count);
    request.payload_window_byte_offset = payload_window_byte_offset;
    request.payload_window_byte_size = payload_window_byte_size;
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

bool DecodePlanCapacityEntryMatches(
    const ResourceDecodePlanSnapshot &snapshot,
    ResourceDecodePlanOperation operation,
    const ResourceDecodePlanRequest &request,
    std::uint32_t expected_plan_capacity,
    std::uint32_t expected_plan_count,
    std::uint32_t expected_required_plan_count) {
    if (snapshot.last_failed_operation != operation) {
        return false;
    }

    if (snapshot.last_required_plan_count != expected_required_plan_count) {
        return false;
    }

    if (snapshot.last_failed_resource.slot != request.resource.slot) {
        return false;
    }

    if (snapshot.last_failed_resource.generation != request.resource.generation) {
        return false;
    }

    if (snapshot.last_failed_expected_type.value != request.expected_type.value) {
        return false;
    }

    if (snapshot.last_failed_payload_id != request.payload_id) {
        return false;
    }

    if (snapshot.last_failed_decode_plan_id != request.decode_plan_id) {
        return false;
    }

    if (snapshot.last_failed_asset_class != request.asset_class) {
        return false;
    }

    if (snapshot.last_failed_plan_capacity != expected_plan_capacity) {
        return false;
    }

    if (snapshot.last_failed_plan_count != expected_plan_count) {
        return false;
    }

    if (snapshot.last_failed_source_byte_count != request.source_byte_count) {
        return false;
    }

    return snapshot.last_failed_expected_decoded_byte_count ==
        request.expected_decoded_byte_count;
}

bool DecodePlanCapacityEntryIsClear(const ResourceDecodePlanSnapshot &snapshot) {
    if (snapshot.last_failed_operation != ResourceDecodePlanOperation::None) {
        return false;
    }

    if (snapshot.last_failed_resource.IsValid()) {
        return false;
    }

    if (snapshot.last_failed_expected_type.IsValid()) {
        return false;
    }

    if (snapshot.last_failed_payload_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_decode_plan_id != 0U) {
        return false;
    }

    if (snapshot.last_failed_asset_class != ResourceDecodePlanAssetClass::Unknown) {
        return false;
    }

    if (snapshot.last_failed_plan_capacity != 0U) {
        return false;
    }

    if (snapshot.last_failed_plan_count != 0U) {
        return false;
    }

    if (snapshot.last_failed_source_byte_count != 0U) {
        return false;
    }

    return snapshot.last_failed_expected_decoded_byte_count == 0U;
}

bool CreateDecodePlanMetadata(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    ResourceDecodePlanAssetClass asset_class,
    std::uint32_t decoded_byte_count) {
    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload = DecodePlanPayload(
        asset_class,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        decoded_byte_count);
    if (!StoreDecodePlanPayload(registry, resource, expected_type, payload_id, payload)) {
        return false;
    }

    const ResourceDecodePlanRequest request = DecodePlanRequest(
        resource,
        expected_type,
        payload_id,
        decode_plan_id,
        asset_class,
        decoded_byte_count);
    return registry.CreateDecodePlan(request) == ResourceDecodePlanStatus::Success;
}

bool CommitDecodeResultMetadata(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t decode_plan_id,
    std::uint64_t decode_result_id,
    ResourceDecodePlanAssetClass asset_class,
    ResourceDecodeResultClass result_class,
    std::uint32_t decoded_byte_count) {
    if (!CreateDecodePlanMetadata(
        registry,
        resource,
        expected_type,
        payload_id,
        decode_plan_id,
        asset_class,
        decoded_byte_count)) {
        return false;
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        resource,
        expected_type,
        payload_id,
        decode_plan_id,
        decode_result_id,
        asset_class,
        result_class,
        decoded_byte_count);
    return registry.CommitDecodeResult(request) == ResourceDecodeResultStatus::Success;
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

bool ConfigureCachePayloadBudget(
    ResourceRegistry &registry,
    std::uint32_t byte_capacity,
    std::uint32_t payload_reference_capacity) {
    ResourceCachePayloadBudgetDesc budget;
    budget.byte_capacity = byte_capacity;
    budget.payload_reference_capacity = payload_reference_capacity;
    return registry.SetCachePayloadBudget(budget) == ResourceCachePayloadStatus::Success;
}

bool ConfigureDecodeResultBudget(ResourceRegistry &registry, std::uint32_t byte_capacity) {
    ResourceDecodeResultBudgetDesc budget;
    budget.decoded_byte_capacity = byte_capacity;
    return registry.SetDecodeResultBudget(budget) == ResourceDecodeResultStatus::Success;
}

int ExpectDecodeResultCapacityEntryClear(const ResourceDecodeResultSnapshot &snapshot) {
    if (snapshot.last_failed_resource.IsValid()) {
        return Fail("decode result capacity entry retained failed resource");
    }

    if (snapshot.last_failed_payload_id != 0U) {
        return Fail("decode result capacity entry retained failed payload id");
    }

    if (snapshot.last_failed_decode_plan_id != 0U) {
        return Fail("decode result capacity entry retained failed plan id");
    }

    if (snapshot.last_failed_decode_result_id != 0U) {
        return Fail("decode result capacity entry retained failed result id");
    }

    if (snapshot.last_failed_asset_class != ResourceDecodePlanAssetClass::Unknown) {
        return Fail("decode result capacity entry retained failed asset class");
    }

    if (snapshot.last_failed_result_class != ResourceDecodeResultClass::Unknown) {
        return Fail("decode result capacity entry retained failed result class");
    }

    if (snapshot.last_failed_result_capacity != 0U) {
        return Fail("decode result capacity entry retained failed result capacity");
    }

    if (snapshot.last_failed_result_count != 0U) {
        return Fail("decode result capacity entry retained failed result count");
    }

    if (snapshot.last_failed_decoded_byte_count != 0U) {
        return Fail("decode result capacity entry retained failed decoded byte count");
    }

    return 0;
}

int ExpectDecodeResultCapacityEntryMatches(
    const ResourceDecodeResultSnapshot &snapshot,
    ResourceHandle resource,
    const ResourceDecodeResultRequest &request,
    std::uint32_t expected_result_capacity,
    std::uint32_t expected_result_count,
    std::uint32_t expected_required_result_count) {
    if (snapshot.last_failed_resource.slot != resource.slot) {
        return Fail("decode result capacity entry missed resource slot");
    }

    if (snapshot.last_failed_resource.generation != resource.generation) {
        return Fail("decode result capacity entry missed resource generation");
    }

    if (snapshot.last_failed_payload_id != request.payload_id) {
        return Fail("decode result capacity entry missed payload id");
    }

    if (snapshot.last_failed_decode_plan_id != request.decode_plan_id) {
        return Fail("decode result capacity entry missed plan id");
    }

    if (snapshot.last_failed_decode_result_id != request.decode_result_id) {
        return Fail("decode result capacity entry missed result id");
    }

    if (snapshot.last_failed_asset_class != request.asset_class) {
        return Fail("decode result capacity entry missed asset class");
    }

    if (snapshot.last_failed_result_class != request.result_class) {
        return Fail("decode result capacity entry missed result class");
    }

    if (snapshot.last_failed_result_capacity != expected_result_capacity) {
        return Fail("decode result capacity entry missed result capacity");
    }

    if (snapshot.last_failed_result_count != expected_result_count) {
        return Fail("decode result capacity entry missed result count");
    }

    if (snapshot.last_required_result_count != expected_required_result_count) {
        return Fail("decode result capacity entry missed required result count");
    }

    if (snapshot.last_failed_decoded_byte_count != request.decoded_byte_count) {
        return Fail("decode result capacity entry missed decoded byte count");
    }

    return 0;
}

bool ConfigureDecodedPayloadBudget(ResourceRegistry &registry, std::uint32_t byte_capacity) {
    ResourceDecodedPayloadBudgetDesc budget;
    budget.decoded_byte_capacity = byte_capacity;
    return registry.SetDecodedPayloadBudget(budget) == ResourceDecodedPayloadStatus::Success;
}

bool ConfigureDecodedPayloadBudget(
    ResourceRegistry &registry,
    std::uint32_t byte_capacity,
    std::uint32_t payload_reference_capacity) {
    ResourceDecodedPayloadBudgetDesc budget{};
    budget.decoded_byte_capacity = byte_capacity;
    budget.payload_reference_capacity = payload_reference_capacity;
    return registry.SetDecodedPayloadBudget(budget) == ResourceDecodedPayloadStatus::Success;
}

int ExpectDecodedPayloadCapacityEntryCleared(const ResourceDecodedPayloadSnapshot &snapshot) {
    if (snapshot.last_failed_decoded_payload_resource.IsValid()) {
        return Fail("decoded payload capacity entry kept resource");
    }

    if (snapshot.last_failed_decoded_payload_logical_key.IsValid()) {
        return Fail("decoded payload capacity entry kept logical key");
    }

    if (snapshot.last_failed_decoded_payload_type.IsValid()) {
        return Fail("decoded payload capacity entry kept type");
    }

    if (snapshot.last_failed_payload_id != 0U) {
        return Fail("decoded payload capacity entry kept payload id");
    }

    if (snapshot.last_failed_decode_plan_id != 0U) {
        return Fail("decoded payload capacity entry kept decode plan id");
    }

    if (snapshot.last_failed_decode_result_id != 0U) {
        return Fail("decoded payload capacity entry kept decode result id");
    }

    if (snapshot.last_failed_decoded_payload_id != 0U) {
        return Fail("decoded payload capacity entry kept decoded payload id");
    }

    if (snapshot.last_failed_payload_logical_byte_count != 0U) {
        return Fail("decoded payload capacity entry kept logical byte count");
    }

    if (snapshot.last_failed_payload_window_byte_offset != 0U) {
        return Fail("decoded payload capacity entry kept window offset");
    }

    if (snapshot.last_failed_payload_window_byte_size != 0U) {
        return Fail("decoded payload capacity entry kept window size");
    }

    if (snapshot.last_failed_asset_class != ResourceDecodePlanAssetClass::Unknown) {
        return Fail("decoded payload capacity entry kept asset class");
    }

    if (snapshot.last_failed_result_class != ResourceDecodeResultClass::Unknown) {
        return Fail("decoded payload capacity entry kept result class");
    }

    if (snapshot.last_failed_decoded_byte_count != 0U) {
        return Fail("decoded payload capacity entry kept decoded byte count");
    }

    if (snapshot.last_failed_required_decoded_byte_count != 0U) {
        return Fail("decoded payload capacity entry kept required decoded bytes");
    }

    if (snapshot.last_failed_required_payload_reference_count != 0U) {
        return Fail("decoded payload capacity entry kept required references");
    }

    if (snapshot.last_failed_decoded_byte_capacity != 0U) {
        return Fail("decoded payload capacity entry kept byte capacity");
    }

    if (snapshot.last_failed_payload_reference_capacity != 0U) {
        return Fail("decoded payload capacity entry kept reference capacity");
    }

    if (snapshot.last_failed_payload_record_capacity != 0U) {
        return Fail("decoded payload capacity entry kept record capacity");
    }

    if (snapshot.last_failed_payload_count != 0U) {
        return Fail("decoded payload capacity entry kept payload count");
    }

    return 0;
}

int ExpectDecodedPayloadCapacityEntryMatches(
    const ResourceDecodedPayloadSnapshot &snapshot,
    ResourceHandle resource,
    const ResourceLogicalKey &logical_key,
    const ResourceDecodedPayloadRequest &request,
    std::uint32_t required_decoded_byte_count,
    std::uint32_t required_payload_reference_count,
    std::uint32_t decoded_byte_capacity,
    std::uint32_t payload_reference_capacity,
    std::uint32_t payload_record_capacity,
    std::uint32_t payload_count) {
    if (snapshot.last_required_decoded_byte_count != required_decoded_byte_count) {
        return Fail("decoded payload capacity entry reported wrong required decoded bytes");
    }

    if (snapshot.last_required_payload_reference_count != required_payload_reference_count) {
        return Fail("decoded payload capacity entry reported wrong required references");
    }

    if (snapshot.last_failed_required_decoded_byte_count != required_decoded_byte_count) {
        return Fail("decoded payload capacity entry stored wrong required decoded bytes");
    }

    if (snapshot.last_failed_required_payload_reference_count != required_payload_reference_count) {
        return Fail("decoded payload capacity entry stored wrong required references");
    }

    if (snapshot.last_failed_decoded_payload_resource.slot != resource.slot) {
        return Fail("decoded payload capacity entry stored wrong resource slot");
    }

    if (snapshot.last_failed_decoded_payload_resource.generation != resource.generation) {
        return Fail("decoded payload capacity entry stored wrong resource generation");
    }

    if (!snapshot.last_failed_decoded_payload_logical_key.Equals(logical_key)) {
        return Fail("decoded payload capacity entry stored wrong logical key");
    }

    if (snapshot.last_failed_decoded_payload_type.value != request.expected_type.value) {
        return Fail("decoded payload capacity entry stored wrong resource type");
    }

    if (snapshot.last_failed_payload_id != request.payload_id) {
        return Fail("decoded payload capacity entry stored wrong payload id");
    }

    if (snapshot.last_failed_decode_plan_id != request.decode_plan_id) {
        return Fail("decoded payload capacity entry stored wrong decode plan id");
    }

    if (snapshot.last_failed_decode_result_id != request.decode_result_id) {
        return Fail("decoded payload capacity entry stored wrong decode result id");
    }

    if (snapshot.last_failed_decoded_payload_id != request.decoded_payload_id) {
        return Fail("decoded payload capacity entry stored wrong decoded payload id");
    }

    if (snapshot.last_failed_payload_logical_byte_count != request.payload_logical_byte_count) {
        return Fail("decoded payload capacity entry stored wrong logical byte count");
    }

    if (snapshot.last_failed_payload_window_byte_offset != request.payload_window_byte_offset) {
        return Fail("decoded payload capacity entry stored wrong window offset");
    }

    if (snapshot.last_failed_payload_window_byte_size != request.payload_window_byte_size) {
        return Fail("decoded payload capacity entry stored wrong window size");
    }

    if (snapshot.last_failed_asset_class != request.asset_class) {
        return Fail("decoded payload capacity entry stored wrong asset class");
    }

    if (snapshot.last_failed_result_class != request.result_class) {
        return Fail("decoded payload capacity entry stored wrong result class");
    }

    if (snapshot.last_failed_decoded_byte_count != request.decoded_byte_count) {
        return Fail("decoded payload capacity entry stored wrong decoded byte count");
    }

    if (snapshot.last_failed_decoded_byte_capacity != decoded_byte_capacity) {
        return Fail("decoded payload capacity entry stored wrong decoded byte capacity");
    }

    if (snapshot.last_failed_payload_reference_capacity != payload_reference_capacity) {
        return Fail("decoded payload capacity entry stored wrong reference capacity");
    }

    if (snapshot.last_failed_payload_record_capacity != payload_record_capacity) {
        return Fail("decoded payload capacity entry stored wrong record capacity");
    }

    if (snapshot.last_failed_payload_count != payload_count) {
        return Fail("decoded payload capacity entry stored wrong payload count");
    }

    return 0;
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

bool CreateTextureDecodeResultChain(
    ResourceRegistry &registry,
    ResourceHandle resource,
    std::uint32_t decoded_byte_count) {
    if (!AdmitUploadedResident(registry, resource, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return false;
    }

    return CommitDecodeResultMetadata(
        registry,
        resource,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        decoded_byte_count);
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

int ResourceDescriptorBatchRegistrationSubmitsRowsAndStopsOnFirstFailure() {
    ResourceRegistry registry;
    const ResourceDescriptor first_descriptor = Descriptor(TYPE_TEXTURE, "batch_texture_a");
    const ResourceDescriptor second_descriptor = Descriptor(TYPE_MATERIAL, "batch_material_a");
    const std::array<ResourceDescriptor, 2U> success_descriptors{{
        first_descriptor,
        second_descriptor}};

    const ResourceDescriptorBatchResult empty_result = registry.RegisterSyntheticDescriptors(nullptr, 0U);
    if (!empty_result.Succeeded() || empty_result.committed_descriptor_count != 0U) {
        return Fail("descriptor batch empty submission did not succeed");
    }

    const ResourceSnapshot before_null_snapshot = registry.Snapshot();
    const ResourceDescriptorBatchResult null_result = registry.RegisterSyntheticDescriptors(nullptr, 1U);
    if (null_result.status != ResourceStatus::InvalidDescriptor ||
        null_result.committed_descriptor_count != 0U ||
        null_result.failed_descriptor_index != 0U) {
        return Fail("descriptor batch null submission missed explicit failure result");
    }

    const ResourceSnapshot after_null_snapshot = registry.Snapshot();
    if (after_null_snapshot.registered_resource_count != before_null_snapshot.registered_resource_count ||
        after_null_snapshot.type_count != before_null_snapshot.type_count) {
        return Fail("descriptor batch null submission mutated registration counts");
    }

    const ResourceDescriptor invalid_descriptor = Descriptor(ResourceTypeId{}, "batch_invalid_type");
    const std::array<ResourceDescriptor, 2U> invalid_first_descriptors{{
        invalid_descriptor,
        first_descriptor}};
    const ResourceSnapshot before_invalid_snapshot = registry.Snapshot();
    const ResourceDescriptorBatchResult invalid_result = registry.RegisterSyntheticDescriptors(
        invalid_first_descriptors.data(),
        static_cast<std::uint32_t>(invalid_first_descriptors.size()));
    if (invalid_result.status != ResourceStatus::InvalidDescriptor ||
        invalid_result.committed_descriptor_count != 0U ||
        invalid_result.failed_descriptor_index != 0U) {
        return Fail("descriptor batch invalid first row missed explicit failure result");
    }

    const ResourceSnapshot after_invalid_snapshot = registry.Snapshot();
    if (after_invalid_snapshot.registered_resource_count != before_invalid_snapshot.registered_resource_count ||
        after_invalid_snapshot.type_count != before_invalid_snapshot.type_count) {
        return Fail("descriptor batch invalid first row mutated registration counts");
    }

    const ResourceDescriptorBatchResult success_result = registry.RegisterSyntheticDescriptors(
        success_descriptors.data(),
        static_cast<std::uint32_t>(success_descriptors.size()));
    if (!success_result.Succeeded() ||
        success_result.committed_descriptor_count != 2U ||
        success_result.failed_descriptor_index != 0U) {
        return Fail("descriptor batch success result was not deterministic");
    }

    const ResourceSnapshot after_success_snapshot = registry.Snapshot();
    if (after_success_snapshot.registered_resource_count != 2U ||
        after_success_snapshot.type_count != 2U) {
        return Fail("descriptor batch success missed registration counts");
    }

    const std::array<ResourceDescriptor, 3U> duplicate_descriptors{{
        Descriptor(TYPE_EFFECT, "batch_effect_a"),
        Descriptor(TYPE_TEXTURE, "batch_texture_a"),
        Descriptor(TYPE_AUDIO, "batch_audio_after_duplicate")}};
    const ResourceSnapshot before_duplicate_snapshot = registry.Snapshot();
    const ResourceDescriptorBatchResult duplicate_result = registry.RegisterSyntheticDescriptors(
        duplicate_descriptors.data(),
        static_cast<std::uint32_t>(duplicate_descriptors.size()));
    if (duplicate_result.status != ResourceStatus::DuplicateResource ||
        duplicate_result.committed_descriptor_count != 1U ||
        duplicate_result.failed_descriptor_index != 1U) {
        return Fail("descriptor batch duplicate row missed committed count or failed index");
    }

    const ResourceSnapshot after_duplicate_snapshot = registry.Snapshot();
    if (after_duplicate_snapshot.registered_resource_count !=
            before_duplicate_snapshot.registered_resource_count + 1U ||
        after_duplicate_snapshot.type_count != before_duplicate_snapshot.type_count + 1U) {
        return Fail("descriptor batch duplicate failure rolled back or over-committed rows");
    }

    const ResourceRegistrationResult retry_result =
        Register(registry, TYPE_AUDIO, "batch_audio_after_duplicate");
    if (!retry_result.Succeeded()) {
        return Fail("descriptor batch duplicate failure registered rows after failure");
    }

    ResourceRegistry capacity_registry(ResourceRegistryDesc{1U, 2U, 1U});
    const std::array<ResourceDescriptor, 2U> capacity_descriptors{{
        Descriptor(TYPE_TEXTURE, "batch_capacity_texture"),
        Descriptor(TYPE_MATERIAL, "batch_capacity_material")}};
    const ResourceDescriptorBatchResult capacity_result = capacity_registry.RegisterSyntheticDescriptors(
        capacity_descriptors.data(),
        static_cast<std::uint32_t>(capacity_descriptors.size()));
    if (capacity_result.status != ResourceStatus::CapacityExceeded ||
        capacity_result.committed_descriptor_count != 1U ||
        capacity_result.failed_descriptor_index != 1U ||
        capacity_result.required_resource_count != 2U) {
        return Fail("descriptor batch capacity failure missed committed count or required count");
    }

    const ResourceSnapshot capacity_snapshot = capacity_registry.Snapshot();
    if (capacity_snapshot.registered_resource_count != 1U ||
        capacity_snapshot.last_required_resource_count != 2U) {
        return Fail("descriptor batch capacity failure mutated counts incorrectly");
    }

    return 0;
}

int ResourceDescriptorEnumerationReportsRegisteredSyntheticDescriptors() {
    ResourceRegistry empty_registry;
    std::uint32_t empty_descriptor_count = 7U;
    const ResourceSnapshot empty_before_snapshot = empty_registry.Snapshot();
    const ResourceStatus empty_status = empty_registry.EnumerateSyntheticDescriptors(
        nullptr,
        0U,
        &empty_descriptor_count);
    if (empty_status != ResourceStatus::Success || empty_descriptor_count != 0U) {
        return Fail("descriptor enumeration empty registry did not succeed");
    }

    const ResourceSnapshot empty_after_snapshot = empty_registry.Snapshot();
    if (empty_after_snapshot.registered_resource_count != empty_before_snapshot.registered_resource_count ||
        empty_after_snapshot.type_count != empty_before_snapshot.type_count ||
        empty_after_snapshot.dependency_validation_count != empty_before_snapshot.dependency_validation_count) {
        return Fail("descriptor enumeration empty registry mutated counters");
    }

    ResourceRegistry registry;
    const ResourceDescriptor single_descriptor =
        DescriptorWithReferenceCount(TYPE_TEXTURE, "enumerate_texture", 2U);
    const ResourceRegistrationResult single_result =
        registry.RegisterSyntheticDescriptor(single_descriptor);
    if (!single_result.Succeeded()) {
        return Fail("descriptor enumeration single fixture registration failed");
    }

    std::array<ResourceDescriptor, 1U> single_output_descriptors{};
    std::uint32_t single_descriptor_count = 0U;
    const ResourceSnapshot before_single_snapshot = registry.Snapshot();
    const ResourceStatus single_status = registry.EnumerateSyntheticDescriptors(
        single_output_descriptors.data(),
        static_cast<std::uint32_t>(single_output_descriptors.size()),
        &single_descriptor_count);
    if (single_status != ResourceStatus::Success || single_descriptor_count != 1U) {
        return Fail("descriptor enumeration single registration returned wrong count");
    }

    if (!DescriptorMatches(single_output_descriptors[0U], TYPE_TEXTURE, "enumerate_texture", 2U)) {
        return Fail("descriptor enumeration single registration returned wrong descriptor");
    }

    const ResourceSnapshot after_single_snapshot = registry.Snapshot();
    if (after_single_snapshot.registered_resource_count != before_single_snapshot.registered_resource_count ||
        after_single_snapshot.type_count != before_single_snapshot.type_count ||
        after_single_snapshot.dependency_validation_count != before_single_snapshot.dependency_validation_count) {
        return Fail("descriptor enumeration single registration mutated counters");
    }

    const std::array<ResourceDescriptor, 2U> batch_descriptors{{
        DescriptorWithReferenceCount(TYPE_MATERIAL, "enumerate_material", 1U),
        DescriptorWithReferenceCount(TYPE_AUDIO, "enumerate_audio", 3U)}};
    const ResourceDescriptorBatchResult batch_result = registry.RegisterSyntheticDescriptors(
        batch_descriptors.data(),
        static_cast<std::uint32_t>(batch_descriptors.size()));
    if (!batch_result.Succeeded()) {
        return Fail("descriptor enumeration batch fixture registration failed");
    }

    const ResourceRegistrationResult invalid_result =
        registry.RegisterSyntheticDescriptor(Descriptor(ResourceTypeId{}, "enumerate_invalid"));
    if (invalid_result.status != ResourceStatus::InvalidDescriptor) {
        return Fail("descriptor enumeration invalid descriptor fixture did not fail");
    }

    const ResourceRegistrationResult duplicate_result =
        registry.RegisterSyntheticDescriptor(Descriptor(TYPE_TEXTURE, "enumerate_texture"));
    if (duplicate_result.status != ResourceStatus::DuplicateResource) {
        return Fail("descriptor enumeration duplicate descriptor fixture did not fail");
    }

    std::array<ResourceDescriptor, 3U> output_descriptors{};
    std::uint32_t output_descriptor_count = 0U;
    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceStatus enumerate_status = registry.EnumerateSyntheticDescriptors(
        output_descriptors.data(),
        static_cast<std::uint32_t>(output_descriptors.size()),
        &output_descriptor_count);
    if (enumerate_status != ResourceStatus::Success || output_descriptor_count != 3U) {
        return Fail("descriptor enumeration success returned wrong count");
    }

    if (!DescriptorMatches(output_descriptors[0U], TYPE_TEXTURE, "enumerate_texture", 2U) ||
        !DescriptorMatches(output_descriptors[1U], TYPE_MATERIAL, "enumerate_material", 1U) ||
        !DescriptorMatches(output_descriptors[2U], TYPE_AUDIO, "enumerate_audio", 3U)) {
        return Fail("descriptor enumeration did not preserve committed descriptor order");
    }

    const ResourceSnapshot after_success_snapshot = registry.Snapshot();
    if (after_success_snapshot.registered_resource_count != before_success_snapshot.registered_resource_count ||
        after_success_snapshot.type_count != before_success_snapshot.type_count ||
        after_success_snapshot.dependency_validation_count != before_success_snapshot.dependency_validation_count) {
        return Fail("descriptor enumeration success mutated counters");
    }

    std::array<ResourceDescriptor, 2U> small_output_descriptors{};
    std::uint32_t required_descriptor_count = 0U;
    const ResourceSnapshot before_capacity_snapshot = registry.Snapshot();
    const ResourceStatus capacity_status = registry.EnumerateSyntheticDescriptors(
        small_output_descriptors.data(),
        static_cast<std::uint32_t>(small_output_descriptors.size()),
        &required_descriptor_count);
    if (capacity_status != ResourceStatus::CapacityExceeded || required_descriptor_count != 3U) {
        return Fail("descriptor enumeration capacity failure missed required count");
    }

    const ResourceSnapshot after_capacity_snapshot = registry.Snapshot();
    if (after_capacity_snapshot.registered_resource_count != before_capacity_snapshot.registered_resource_count ||
        after_capacity_snapshot.type_count != before_capacity_snapshot.type_count ||
        after_capacity_snapshot.dependency_validation_count != before_capacity_snapshot.dependency_validation_count ||
        after_capacity_snapshot.last_required_resource_count != 3U) {
        return Fail("descriptor enumeration capacity failure mutated counters or missed snapshot count");
    }

    std::uint32_t invalid_descriptor_count = 0U;
    const ResourceSnapshot before_invalid_snapshot = registry.Snapshot();
    const ResourceStatus invalid_output_status = registry.EnumerateSyntheticDescriptors(
        nullptr,
        1U,
        &invalid_descriptor_count);
    if (invalid_output_status != ResourceStatus::InvalidHandle || invalid_descriptor_count != 0U) {
        return Fail("descriptor enumeration invalid output did not fail explicitly");
    }

    const ResourceSnapshot after_invalid_snapshot = registry.Snapshot();
    if (after_invalid_snapshot.registered_resource_count != before_invalid_snapshot.registered_resource_count ||
        after_invalid_snapshot.type_count != before_invalid_snapshot.type_count ||
        after_invalid_snapshot.dependency_validation_count != before_invalid_snapshot.dependency_validation_count) {
        return Fail("descriptor enumeration invalid output mutated counters");
    }

    return 0;
}

int ResourceDescriptorExactLookupFindsRegisteredSyntheticDescriptor() {
    ResourceRegistry registry;
    const std::array<ResourceDescriptor, 2U> descriptors{{
        DescriptorWithReferenceCount(TYPE_TEXTURE, "lookup_texture", 2U),
        DescriptorWithReferenceCount(TYPE_MATERIAL, "lookup_material", 1U)}};
    const ResourceDescriptorBatchResult batch_result = registry.RegisterSyntheticDescriptors(
        descriptors.data(),
        static_cast<std::uint32_t>(descriptors.size()));
    if (!batch_result.Succeeded()) {
        return Fail("descriptor exact lookup fixture registration failed");
    }

    const ResourceDescriptor duplicate_descriptor =
        DescriptorWithReferenceCount(TYPE_TEXTURE, "lookup_texture", 9U);
    const ResourceRegistrationResult duplicate_result =
        registry.RegisterSyntheticDescriptor(duplicate_descriptor);
    if (duplicate_result.status != ResourceStatus::DuplicateResource) {
        return Fail("descriptor exact lookup duplicate fixture did not fail");
    }

    const ResourceTypeId invalid_type{};
    const ResourceDescriptor invalid_descriptor = Descriptor(invalid_type, "lookup_invalid");
    const ResourceRegistrationResult invalid_result =
        registry.RegisterSyntheticDescriptor(invalid_descriptor);
    if (invalid_result.status != ResourceStatus::InvalidDescriptor) {
        return Fail("descriptor exact lookup invalid fixture did not fail");
    }

    ResourceDescriptor output_descriptor{};
    const ResourceLogicalKey lookup_texture_key("lookup_texture");
    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceStatus success_status = registry.FindSyntheticDescriptor(
        TYPE_TEXTURE,
        lookup_texture_key,
        &output_descriptor);
    if (success_status != ResourceStatus::Success) {
        return Fail("descriptor exact lookup did not find registered descriptor");
    }

    if (!DescriptorMatches(output_descriptor, TYPE_TEXTURE, "lookup_texture", 2U)) {
        return Fail("descriptor exact lookup returned unexpected descriptor");
    }

    const ResourceSnapshot after_success_snapshot = registry.Snapshot();
    if (after_success_snapshot.registered_resource_count != before_success_snapshot.registered_resource_count ||
        after_success_snapshot.type_count != before_success_snapshot.type_count ||
        after_success_snapshot.dependency_validation_count != before_success_snapshot.dependency_validation_count) {
        return Fail("descriptor exact lookup success mutated counters");
    }

    ResourceDescriptor missing_output_descriptor = Descriptor(TYPE_AUDIO, "lookup_sentinel");
    const ResourceLogicalKey missing_key("lookup_missing");
    const ResourceSnapshot before_missing_snapshot = registry.Snapshot();
    const ResourceStatus missing_status = registry.FindSyntheticDescriptor(
        TYPE_TEXTURE,
        missing_key,
        &missing_output_descriptor);
    if (missing_status != ResourceStatus::NotFound) {
        return Fail("descriptor exact lookup missing key did not return NotFound");
    }

    if (!DescriptorMatches(missing_output_descriptor, TYPE_AUDIO, "lookup_sentinel", 0U)) {
        return Fail("descriptor exact lookup missing key mutated output descriptor");
    }

    const ResourceSnapshot after_missing_snapshot = registry.Snapshot();
    if (after_missing_snapshot.registered_resource_count != before_missing_snapshot.registered_resource_count ||
        after_missing_snapshot.type_count != before_missing_snapshot.type_count ||
        after_missing_snapshot.dependency_validation_count != before_missing_snapshot.dependency_validation_count) {
        return Fail("descriptor exact lookup missing key mutated counters");
    }

    ResourceDescriptor invalid_output_descriptor = Descriptor(TYPE_AUDIO, "lookup_invalid_sentinel");
    const ResourceStatus invalid_type_status = registry.FindSyntheticDescriptor(
        invalid_type,
        lookup_texture_key,
        &invalid_output_descriptor);
    if (invalid_type_status != ResourceStatus::InvalidDescriptor) {
        return Fail("descriptor exact lookup invalid type did not fail explicitly");
    }

    if (!DescriptorMatches(invalid_output_descriptor, TYPE_AUDIO, "lookup_invalid_sentinel", 0U)) {
        return Fail("descriptor exact lookup invalid type mutated output descriptor");
    }

    const ResourceLogicalKey empty_key("");
    const ResourceStatus invalid_key_status = registry.FindSyntheticDescriptor(
        TYPE_TEXTURE,
        empty_key,
        &invalid_output_descriptor);
    if (invalid_key_status != ResourceStatus::InvalidDescriptor) {
        return Fail("descriptor exact lookup invalid key did not fail explicitly");
    }

    const ResourceStatus invalid_output_status = registry.FindSyntheticDescriptor(
        TYPE_TEXTURE,
        lookup_texture_key,
        nullptr);
    if (invalid_output_status != ResourceStatus::InvalidHandle) {
        return Fail("descriptor exact lookup null output did not fail explicitly");
    }

    std::array<ResourceDescriptor, 2U> enumerated_descriptors{};
    std::uint32_t enumerated_descriptor_count = 0U;
    const ResourceStatus enumerate_status = registry.EnumerateSyntheticDescriptors(
        enumerated_descriptors.data(),
        static_cast<std::uint32_t>(enumerated_descriptors.size()),
        &enumerated_descriptor_count);
    if (enumerate_status != ResourceStatus::Success || enumerated_descriptor_count != 2U) {
        return Fail("descriptor exact lookup changed enumeration count");
    }

    if (!DescriptorMatches(enumerated_descriptors[0U], TYPE_TEXTURE, "lookup_texture", 2U) ||
        !DescriptorMatches(enumerated_descriptors[1U], TYPE_MATERIAL, "lookup_material", 1U)) {
        return Fail("descriptor exact lookup exposed failed descriptor rows");
    }

    return 0;
}

int ResourceDescriptorBatchExactLookupReturnsAtomicRows() {
    ResourceRegistry registry;
    const std::array<ResourceDescriptor, 2U> descriptors{{
        DescriptorWithReferenceCount(TYPE_TEXTURE, "batch_lookup_texture", 2U),
        DescriptorWithReferenceCount(TYPE_MATERIAL, "batch_lookup_material", 1U)}};
    const ResourceDescriptorBatchResult registration_result = registry.RegisterSyntheticDescriptors(
        descriptors.data(),
        static_cast<std::uint32_t>(descriptors.size()));
    if (!registration_result.Succeeded()) {
        return Fail("descriptor batch lookup fixture registration failed");
    }

    const std::array<ResourceDescriptorLookupQuery, 3U> success_queries{{
        DescriptorLookupQuery(TYPE_TEXTURE, "batch_lookup_texture"),
        DescriptorLookupQuery(TYPE_MATERIAL, "batch_lookup_material"),
        DescriptorLookupQuery(TYPE_TEXTURE, "batch_lookup_texture")}};
    std::array<ResourceDescriptorLookupRecord, 3U> output_records{};
    std::uint32_t output_record_count = 99U;
    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceDescriptorBatchLookupResult success_result = registry.FindSyntheticDescriptors(
        success_queries.data(),
        static_cast<std::uint32_t>(success_queries.size()),
        output_records.data(),
        static_cast<std::uint32_t>(output_records.size()),
        &output_record_count);
    if (!success_result.Succeeded() ||
        success_result.matched_descriptor_count != 3U ||
        output_record_count != 3U) {
        return Fail("descriptor batch lookup success returned wrong counts");
    }

    const ResourceHandle texture_handle{0U, 1U};
    const ResourceHandle material_handle{1U, 1U};
    if (!LookupRecordMatches(output_records[0U], texture_handle, TYPE_TEXTURE, "batch_lookup_texture", 2U) ||
        !LookupRecordMatches(output_records[1U], material_handle, TYPE_MATERIAL, "batch_lookup_material", 1U) ||
        !LookupRecordMatches(output_records[2U], texture_handle, TYPE_TEXTURE, "batch_lookup_texture", 2U)) {
        return Fail("descriptor batch lookup did not preserve query order");
    }

    const ResourceSnapshot after_success_snapshot = registry.Snapshot();
    if (after_success_snapshot.registered_resource_count != before_success_snapshot.registered_resource_count ||
        after_success_snapshot.type_count != before_success_snapshot.type_count ||
        after_success_snapshot.dependency_validation_count != before_success_snapshot.dependency_validation_count) {
        return Fail("descriptor batch lookup success mutated counters");
    }

    const std::array<ResourceDescriptorLookupRecord, 3U> stable_records = output_records;
    const std::uint32_t stable_output_record_count = output_record_count;
    const ResourceDescriptorBatchLookupResult capacity_result = registry.FindSyntheticDescriptors(
        success_queries.data(),
        static_cast<std::uint32_t>(success_queries.size()),
        output_records.data(),
        2U,
        &output_record_count);
    if (capacity_result.status != ResourceStatus::CapacityExceeded ||
        capacity_result.required_descriptor_count != 3U ||
        output_record_count != stable_output_record_count) {
        return Fail("descriptor batch lookup capacity failure missed atomic count or required count");
    }

    if (!LookupRecordEquals(output_records[0U], stable_records[0U]) ||
        !LookupRecordEquals(output_records[1U], stable_records[1U]) ||
        !LookupRecordEquals(output_records[2U], stable_records[2U])) {
        return Fail("descriptor batch lookup capacity failure mutated output rows");
    }

    const std::array<ResourceDescriptorLookupQuery, 3U> missing_queries{{
        DescriptorLookupQuery(TYPE_TEXTURE, "batch_lookup_texture"),
        DescriptorLookupQuery(TYPE_TEXTURE, "batch_lookup_missing"),
        DescriptorLookupQuery(TYPE_MATERIAL, "batch_lookup_material")}};
    const ResourceDescriptorBatchLookupResult missing_result = registry.FindSyntheticDescriptors(
        missing_queries.data(),
        static_cast<std::uint32_t>(missing_queries.size()),
        output_records.data(),
        static_cast<std::uint32_t>(output_records.size()),
        &output_record_count);
    if (missing_result.status != ResourceStatus::NotFound ||
        missing_result.failed_query_index != 1U ||
        output_record_count != stable_output_record_count) {
        return Fail("descriptor batch lookup missing query missed failed index or mutated count");
    }

    if (!LookupRecordEquals(output_records[0U], stable_records[0U]) ||
        !LookupRecordEquals(output_records[1U], stable_records[1U]) ||
        !LookupRecordEquals(output_records[2U], stable_records[2U])) {
        return Fail("descriptor batch lookup missing query mutated output rows");
    }

    const ResourceTypeId invalid_type{};
    const std::array<ResourceDescriptorLookupQuery, 2U> invalid_type_queries{{
        DescriptorLookupQuery(TYPE_TEXTURE, "batch_lookup_texture"),
        DescriptorLookupQuery(invalid_type, "batch_lookup_invalid")}};
    const ResourceDescriptorBatchLookupResult invalid_type_result = registry.FindSyntheticDescriptors(
        invalid_type_queries.data(),
        static_cast<std::uint32_t>(invalid_type_queries.size()),
        output_records.data(),
        static_cast<std::uint32_t>(output_records.size()),
        &output_record_count);
    if (invalid_type_result.status != ResourceStatus::InvalidDescriptor ||
        invalid_type_result.failed_query_index != 1U ||
        output_record_count != stable_output_record_count) {
        return Fail("descriptor batch lookup invalid type missed failed index or mutated count");
    }

    const std::array<ResourceDescriptorLookupQuery, 1U> invalid_key_queries{{
        DescriptorLookupQuery(TYPE_TEXTURE, "")}};
    const ResourceDescriptorBatchLookupResult invalid_key_result = registry.FindSyntheticDescriptors(
        invalid_key_queries.data(),
        static_cast<std::uint32_t>(invalid_key_queries.size()),
        output_records.data(),
        static_cast<std::uint32_t>(output_records.size()),
        &output_record_count);
    if (invalid_key_result.status != ResourceStatus::InvalidDescriptor ||
        invalid_key_result.failed_query_index != 0U ||
        output_record_count != stable_output_record_count) {
        return Fail("descriptor batch lookup invalid key missed failed index or mutated count");
    }

    const ResourceDescriptorBatchLookupResult null_output_result = registry.FindSyntheticDescriptors(
        success_queries.data(),
        static_cast<std::uint32_t>(success_queries.size()),
        nullptr,
        static_cast<std::uint32_t>(output_records.size()),
        &output_record_count);
    if (null_output_result.status != ResourceStatus::InvalidHandle ||
        output_record_count != stable_output_record_count) {
        return Fail("descriptor batch lookup null output missed invalid handle or mutated count");
    }

    std::uint32_t empty_output_count = 44U;
    const ResourceDescriptorBatchLookupResult empty_result = registry.FindSyntheticDescriptors(
        nullptr,
        0U,
        nullptr,
        0U,
        &empty_output_count);
    if (!empty_result.Succeeded() || empty_output_count != 0U) {
        return Fail("descriptor batch lookup empty query did not succeed");
    }

    return 0;
}

int ResourceRegisterRejectsInvalidDescriptorWithoutMutation() {
    ResourceRegistry registry;
    const ResourceDescriptor invalid_type = Descriptor(ResourceTypeId{}, "texture_invalid_type");
    if (!InvalidDescriptorRegistrationFailsWithoutMutation(registry, invalid_type)) {
        return Fail("invalid type descriptor did not return explicit status");
    }

    const ResourceDescriptor empty_key = Descriptor(TYPE_TEXTURE, "");
    if (!InvalidDescriptorRegistrationFailsWithoutMutation(registry, empty_key)) {
        return Fail("empty key descriptor did not return explicit status");
    }

    const std::size_t overlong_key_byte_count = MAX_LOGICAL_KEY_BYTES + 1U;
    const std::string overlong_key(overlong_key_byte_count, 'a');
    const ResourceDescriptor overlong_key_descriptor = Descriptor(TYPE_TEXTURE, overlong_key.c_str());
    if (!InvalidDescriptorRegistrationFailsWithoutMutation(registry, overlong_key_descriptor)) {
        return Fail("overlong key descriptor did not return explicit status");
    }

    const ResourceRegistrationResult retry_result = Register(registry, TYPE_TEXTURE, "texture_valid_after_invalid");
    if (!retry_result.Succeeded()) {
        return Fail("valid registration after invalid descriptor failures failed");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_resource_count != 1U || snapshot.type_count != 1U) {
        return Fail("invalid descriptor failures mutated registration counts");
    }

    if (snapshot.last_status != ResourceStatus::Success) {
        return Fail("valid registration did not clear failed last status");
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

    const std::uint32_t required_resource_count = before_snapshot.registered_resource_count + 1U;
    if (overflow_result.required_resource_count != required_resource_count) {
        return Fail("resource capacity overflow did not report required resource count");
    }

    if (overflow_result.required_type_count != 0U) {
        return Fail("resource capacity overflow reported type count");
    }

    if (overflow_result.required_dependency_edge_count != 0U) {
        return Fail("resource capacity overflow reported dependency count");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("resource capacity overflow changed registered count");
    }

    if (after_snapshot.last_required_resource_count != required_resource_count) {
        return Fail("resource capacity overflow snapshot missed required resource count");
    }

    if (after_snapshot.last_required_type_count != 0U) {
        return Fail("resource capacity overflow snapshot reported type count");
    }

    if (after_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("resource capacity overflow snapshot reported dependency count");
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

    const std::uint32_t required_type_count = before_snapshot.type_count + 1U;
    if (overflow_result.required_type_count != required_type_count) {
        return Fail("type capacity overflow did not report required type count");
    }

    if (overflow_result.required_resource_count != 0U) {
        return Fail("type capacity overflow reported resource count");
    }

    if (overflow_result.required_dependency_edge_count != 0U) {
        return Fail("type capacity overflow reported dependency count");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail(TYPE_CAPACITY_TYPE_COUNT_FAILED);
    }

    if (after_snapshot.last_required_type_count != required_type_count) {
        return Fail("type capacity overflow snapshot missed required type count");
    }

    if (after_snapshot.last_required_resource_count != 0U) {
        return Fail("type capacity overflow snapshot reported resource count");
    }

    if (after_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("type capacity overflow snapshot reported dependency count");
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

    if (registry.Snapshot().last_required_type_count != 0U) {
        return Fail("successful retry left required type count diagnostic");
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

    const ResourceSnapshot failed_snapshot = registry.Snapshot();
    if (failed_snapshot.acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail("type mismatch changed reference count");
    }

    if (failed_snapshot.last_status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch did not record failed last status");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("successful acquire after failure failed");
    }

    const ResourceSnapshot success_snapshot = registry.Snapshot();
    if (success_snapshot.last_status != ResourceStatus::Success) {
        return Fail("successful acquire did not clear failed last status");
    }

    if (success_snapshot.last_load_commit_status != failed_snapshot.last_load_commit_status) {
        return Fail("successful acquire changed load commit status");
    }

    if (success_snapshot.last_load_state != failed_snapshot.last_load_state) {
        return Fail("successful acquire changed load state");
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

    ResourceRegistry capacity_registry(ResourceRegistryDesc{3U, 3U, 1U});
    const ResourceRegistrationResult capacity_dependency =
        Register(capacity_registry, TYPE_TEXTURE, "texture_capacity_dependency");
    const ResourceRegistrationResult capacity_first_dependent =
        Register(capacity_registry, TYPE_MATERIAL, "material_capacity_dependent");
    const ResourceRegistrationResult capacity_second_dependent =
        Register(capacity_registry, TYPE_EFFECT, "effect_capacity_dependent");
    if (!capacity_dependency.Succeeded()) {
        return Fail("dependency capacity dependency registration failed");
    }

    if (!capacity_first_dependent.Succeeded()) {
        return Fail("dependency capacity first dependent registration failed");
    }

    if (!capacity_second_dependent.Succeeded()) {
        return Fail("dependency capacity second dependent registration failed");
    }

    if (capacity_registry.AddDependency(
        capacity_first_dependent.handle,
        capacity_dependency.handle) != ResourceStatus::Success) {
        return Fail("dependency capacity fixture edge failed");
    }

    const ResourceSnapshot capacity_before_snapshot = capacity_registry.Snapshot();
    const ResourceStatus capacity_status = capacity_registry.AddDependency(
        capacity_second_dependent.handle,
        capacity_dependency.handle);
    if (capacity_status != ResourceStatus::CapacityExceeded) {
        return Fail("dependency capacity overflow did not return explicit status");
    }

    const ResourceSnapshot capacity_after_snapshot = capacity_registry.Snapshot();
    if (capacity_after_snapshot.dependency_edge_count != capacity_before_snapshot.dependency_edge_count) {
        return Fail("dependency capacity overflow changed edge count");
    }

    const std::uint32_t required_dependency_edge_count = capacity_before_snapshot.dependency_edge_count + 1U;
    if (capacity_after_snapshot.last_required_dependency_edge_count != required_dependency_edge_count) {
        return Fail("dependency capacity overflow missed required dependency count");
    }

    if (capacity_after_snapshot.last_required_resource_count != 0U) {
        return Fail("dependency capacity overflow reported resource count");
    }

    if (capacity_after_snapshot.last_required_type_count != 0U) {
        return Fail("dependency capacity overflow reported type count");
    }

    const std::uint32_t expected_dependency_validation_count =
        capacity_before_snapshot.dependency_validation_count + 1U;
    if (capacity_after_snapshot.dependency_validation_count != expected_dependency_validation_count) {
        return Fail("dependency capacity overflow missed validation count");
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

    const ResourceSnapshot failed_snapshot = registry.Snapshot();
    if (failed_snapshot.last_status != ResourceStatus::DependencyCycle) {
        return Fail("self dependency did not record failed last status");
    }

    if (registry.AddDependency(first.handle, second.handle) != ResourceStatus::Success) {
        return Fail("first dependency edge failed");
    }

    const ResourceSnapshot first_success_snapshot = registry.Snapshot();
    if (first_success_snapshot.last_status != ResourceStatus::Success) {
        return Fail("dependency success did not clear failed last status");
    }

    if (first_success_snapshot.last_load_commit_status != failed_snapshot.last_load_commit_status) {
        return Fail("dependency success changed load commit status");
    }

    if (first_success_snapshot.last_load_state != failed_snapshot.last_load_state) {
        return Fail("dependency success changed load state");
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

int ResourceDependencyBatchSubmitsRowsAndStopsOnFirstFailure() {
    ResourceRegistry registry;
    const ResourceRegistrationResult root = Register(registry, TYPE_MATERIAL, "batch_root");
    const ResourceRegistrationResult shared = Register(registry, TYPE_TEXTURE, "batch_shared");
    const ResourceRegistrationResult leaf = Register(registry, TYPE_AUDIO, "batch_leaf");
    if (!root.Succeeded()) {
        return Fail("dependency batch root registration failed");
    }

    if (!shared.Succeeded()) {
        return Fail("dependency batch shared registration failed");
    }

    if (!leaf.Succeeded()) {
        return Fail("dependency batch leaf registration failed");
    }

    const ResourceDependencyBatchResult empty_result = registry.AddDependencies(nullptr, 0U);
    if (!empty_result.Succeeded() || empty_result.committed_dependency_edge_count != 0U) {
        return Fail("dependency batch empty submission did not succeed");
    }

    const ResourceSnapshot before_null_snapshot = registry.Snapshot();
    const ResourceDependencyBatchResult null_result = registry.AddDependencies(nullptr, 1U);
    if (null_result.status != ResourceStatus::InvalidHandle ||
        null_result.committed_dependency_edge_count != 0U ||
        null_result.failed_dependency_edge_index != 0U) {
        return Fail("dependency batch null submission missed explicit failure result");
    }

    const ResourceSnapshot after_null_snapshot = registry.Snapshot();
    if (after_null_snapshot.dependency_edge_count != before_null_snapshot.dependency_edge_count) {
        return Fail("dependency batch null submission changed edge count");
    }

    const std::array<ResourceDependencyRequest, 2U> success_rows{{
        ResourceDependencyRequest{root.handle, shared.handle},
        ResourceDependencyRequest{shared.handle, leaf.handle}}};
    const ResourceDependencyBatchResult success_result = registry.AddDependencies(
        success_rows.data(),
        static_cast<std::uint32_t>(success_rows.size()));
    if (!success_result.Succeeded() ||
        success_result.committed_dependency_edge_count != 2U ||
        success_result.failed_dependency_edge_index != 0U) {
        return Fail("dependency batch success result was not deterministic");
    }

    std::array<ResourceHandle, 2U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const ResourceStatus traverse_status = registry.TraverseDependencies(
        root.handle,
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        &dependency_count);
    if (traverse_status != ResourceStatus::Success || dependency_count != 2U) {
        return Fail("dependency batch committed edges were not visible to traversal");
    }

    if (dependencies[0U].slot != shared.handle.slot ||
        dependencies[1U].slot != leaf.handle.slot) {
        return Fail("dependency batch traversal returned wrong committed order");
    }

    const ResourceSnapshot before_duplicate_snapshot = registry.Snapshot();
    const std::array<ResourceDependencyRequest, 1U> duplicate_rows{{
        ResourceDependencyRequest{root.handle, shared.handle}}};
    const ResourceDependencyBatchResult duplicate_result = registry.AddDependencies(
        duplicate_rows.data(),
        static_cast<std::uint32_t>(duplicate_rows.size()));
    if (!duplicate_result.Succeeded() || duplicate_result.committed_dependency_edge_count != 1U) {
        return Fail("dependency batch duplicate row did not reuse AddDependency success semantics");
    }

    if (registry.Snapshot().dependency_edge_count != before_duplicate_snapshot.dependency_edge_count) {
        return Fail("dependency batch duplicate row added a second edge");
    }

    const ResourceRegistrationResult cycle_first = Register(registry, TYPE_EFFECT, "batch_cycle_first");
    const ResourceRegistrationResult cycle_second = Register(registry, TYPE_AUDIO, "batch_cycle_second");
    const ResourceRegistrationResult cycle_third = Register(registry, TYPE_TEXTURE, "batch_cycle_third");
    if (!cycle_first.Succeeded() || !cycle_second.Succeeded() || !cycle_third.Succeeded()) {
        return Fail("dependency batch cycle fixture registration failed");
    }

    const ResourceSnapshot before_cycle_snapshot = registry.Snapshot();
    const std::array<ResourceDependencyRequest, 3U> cycle_rows{{
        ResourceDependencyRequest{cycle_first.handle, cycle_second.handle},
        ResourceDependencyRequest{cycle_second.handle, cycle_first.handle},
        ResourceDependencyRequest{cycle_second.handle, cycle_third.handle}}};
    const ResourceDependencyBatchResult cycle_result = registry.AddDependencies(
        cycle_rows.data(),
        static_cast<std::uint32_t>(cycle_rows.size()));
    if (cycle_result.status != ResourceStatus::DependencyCycle ||
        cycle_result.committed_dependency_edge_count != 1U ||
        cycle_result.failed_dependency_edge_index != 1U) {
        return Fail("dependency batch cycle failure missed committed count or failed index");
    }

    const ResourceSnapshot after_cycle_snapshot = registry.Snapshot();
    if (after_cycle_snapshot.dependency_edge_count != before_cycle_snapshot.dependency_edge_count + 1U) {
        return Fail("dependency batch cycle failure rolled back or over-committed rows");
    }

    std::array<ResourceHandle, 1U> cycle_dependencies{};
    std::uint32_t cycle_dependency_count = 0U;
    const ResourceStatus cycle_traverse_status = registry.TraverseDependencies(
        cycle_second.handle,
        cycle_dependencies.data(),
        static_cast<std::uint32_t>(cycle_dependencies.size()),
        &cycle_dependency_count);
    if (cycle_traverse_status != ResourceStatus::Success || cycle_dependency_count != 0U) {
        return Fail("dependency batch applied rows after first failure");
    }

    ResourceRegistry capacity_registry(ResourceRegistryDesc{3U, 3U, 1U});
    const ResourceRegistrationResult capacity_dependency =
        Register(capacity_registry, TYPE_TEXTURE, "batch_capacity_dependency");
    const ResourceRegistrationResult capacity_first =
        Register(capacity_registry, TYPE_MATERIAL, "batch_capacity_first");
    const ResourceRegistrationResult capacity_second =
        Register(capacity_registry, TYPE_EFFECT, "batch_capacity_second");
    if (!capacity_dependency.Succeeded() ||
        !capacity_first.Succeeded() ||
        !capacity_second.Succeeded()) {
        return Fail("dependency batch capacity fixture registration failed");
    }

    const std::array<ResourceDependencyRequest, 2U> capacity_rows{{
        ResourceDependencyRequest{capacity_first.handle, capacity_dependency.handle},
        ResourceDependencyRequest{capacity_second.handle, capacity_dependency.handle}}};
    const ResourceDependencyBatchResult capacity_result = capacity_registry.AddDependencies(
        capacity_rows.data(),
        static_cast<std::uint32_t>(capacity_rows.size()));
    if (capacity_result.status != ResourceStatus::CapacityExceeded ||
        capacity_result.committed_dependency_edge_count != 1U ||
        capacity_result.failed_dependency_edge_index != 1U) {
        return Fail("dependency batch capacity failure missed committed count or failed index");
    }

    const ResourceSnapshot capacity_snapshot = capacity_registry.Snapshot();
    if (capacity_snapshot.dependency_edge_count != 1U ||
        capacity_snapshot.last_required_dependency_edge_count != 2U) {
        return Fail("dependency batch capacity failure missed required edge count");
    }

    return 0;
}

int ResourceDependencyEdgeExactLookupFindsDirectEdge() {
    ResourceRegistry registry;
    const ResourceRegistrationResult root = Register(registry, TYPE_MATERIAL, "edge_lookup_root");
    const ResourceRegistrationResult shared = Register(registry, TYPE_TEXTURE, "edge_lookup_shared");
    const ResourceRegistrationResult leaf = Register(registry, TYPE_AUDIO, "edge_lookup_leaf");
    const ResourceRegistrationResult unrelated = Register(registry, TYPE_EFFECT, "edge_lookup_unrelated");
    if (!root.Succeeded() ||
        !shared.Succeeded() ||
        !leaf.Succeeded() ||
        !unrelated.Succeeded()) {
        return Fail("dependency edge exact lookup fixture registration failed");
    }

    if (registry.AddDependency(root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("dependency edge exact lookup root edge setup failed");
    }

    if (registry.AddDependency(shared.handle, leaf.handle) != ResourceStatus::Success) {
        return Fail("dependency edge exact lookup transitive edge setup failed");
    }

    bool edge_exists = false;
    const ResourceSnapshot before_direct_snapshot = registry.Snapshot();
    const ResourceStatus direct_status = registry.FindDependencyEdge(
        root.handle,
        shared.handle,
        &edge_exists);
    if (direct_status != ResourceStatus::Success || !edge_exists) {
        return Fail("dependency edge exact lookup did not find direct edge");
    }

    ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.dependency_edge_count != before_direct_snapshot.dependency_edge_count ||
        after_snapshot.dependency_validation_count != before_direct_snapshot.dependency_validation_count) {
        return Fail("dependency edge exact lookup direct success mutated dependency counters");
    }

    if (after_snapshot.last_status != ResourceStatus::Success) {
        return Fail("dependency edge exact lookup direct success did not record last status");
    }

    bool preserve_output = true;
    const ResourceStatus transitive_status = registry.FindDependencyEdge(
        root.handle,
        leaf.handle,
        &preserve_output);
    if (transitive_status != ResourceStatus::NotFound || !preserve_output) {
        return Fail("dependency edge exact lookup accepted transitive edge or mutated output");
    }

    after_snapshot = registry.Snapshot();
    if (after_snapshot.dependency_edge_count != before_direct_snapshot.dependency_edge_count ||
        after_snapshot.dependency_validation_count != before_direct_snapshot.dependency_validation_count) {
        return Fail("dependency edge exact lookup transitive miss mutated dependency counters");
    }

    if (after_snapshot.last_status != ResourceStatus::NotFound) {
        return Fail("dependency edge exact lookup transitive miss did not record not found");
    }

    preserve_output = true;
    const ResourceStatus missing_status = registry.FindDependencyEdge(
        unrelated.handle,
        root.handle,
        &preserve_output);
    if (missing_status != ResourceStatus::NotFound || !preserve_output) {
        return Fail("dependency edge exact lookup accepted missing edge or mutated output");
    }

    preserve_output = true;
    const ResourceStatus invalid_dependent_status = registry.FindDependencyEdge(
        ResourceHandle{},
        shared.handle,
        &preserve_output);
    if (invalid_dependent_status != ResourceStatus::InvalidHandle || !preserve_output) {
        return Fail("dependency edge exact lookup invalid dependent mutated output");
    }

    preserve_output = true;
    const ResourceStatus invalid_dependency_status = registry.FindDependencyEdge(
        root.handle,
        ResourceHandle{},
        &preserve_output);
    if (invalid_dependency_status != ResourceStatus::InvalidHandle || !preserve_output) {
        return Fail("dependency edge exact lookup invalid dependency mutated output");
    }

    const ResourceRegistrationResult stale_result = Register(registry, TYPE_TEXTURE, "edge_lookup_stale");
    if (!stale_result.Succeeded()) {
        return Fail("dependency edge exact lookup stale fixture registration failed");
    }

    if (registry.Retire(stale_result.handle) != ResourceStatus::Success) {
        return Fail("dependency edge exact lookup stale fixture retire failed");
    }

    preserve_output = true;
    const ResourceStatus stale_status = registry.FindDependencyEdge(
        stale_result.handle,
        shared.handle,
        &preserve_output);
    if (stale_status != ResourceStatus::GenerationMismatch || !preserve_output) {
        return Fail("dependency edge exact lookup stale dependent mutated output");
    }

    const ResourceStatus null_output_status = registry.FindDependencyEdge(
        root.handle,
        shared.handle,
        nullptr);
    if (null_output_status != ResourceStatus::InvalidHandle) {
        return Fail("dependency edge exact lookup null output did not fail explicitly");
    }

    after_snapshot = registry.Snapshot();
    if (after_snapshot.dependency_edge_count != before_direct_snapshot.dependency_edge_count ||
        after_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("dependency edge exact lookup failure mutated dependency capacity data");
    }

    return 0;
}

int ResourceDependencyEdgeCountSnapshotMatchesDirectEdges() {
    ResourceRegistry empty_registry;
    std::uint32_t empty_dependency_edge_count = 7U;
    const ResourceSnapshot empty_before_snapshot = empty_registry.Snapshot();
    const ResourceStatus empty_status = empty_registry.CountDependencyEdges(&empty_dependency_edge_count);
    if (empty_status != ResourceStatus::Success || empty_dependency_edge_count != 0U) {
        return Fail("dependency edge count empty graph did not return zero");
    }

    const ResourceSnapshot empty_after_snapshot = empty_registry.Snapshot();
    if (empty_after_snapshot.dependency_edge_count != empty_before_snapshot.dependency_edge_count ||
        empty_after_snapshot.dependency_validation_count != empty_before_snapshot.dependency_validation_count) {
        return Fail("dependency edge count empty graph mutated dependency counters");
    }

    const ResourceStatus empty_null_status = empty_registry.CountDependencyEdges(nullptr);
    if (empty_null_status != ResourceStatus::InvalidHandle) {
        return Fail("dependency edge count null output did not fail explicitly");
    }

    ResourceRegistry registry;
    const ResourceRegistrationResult root = Register(registry, TYPE_MATERIAL, "edge_count_root");
    const ResourceRegistrationResult shared = Register(registry, TYPE_TEXTURE, "edge_count_shared");
    const ResourceRegistrationResult leaf = Register(registry, TYPE_AUDIO, "edge_count_leaf");
    const ResourceRegistrationResult second_root = Register(registry, TYPE_EFFECT, "edge_count_second_root");
    if (!root.Succeeded() ||
        !shared.Succeeded() ||
        !leaf.Succeeded() ||
        !second_root.Succeeded()) {
        return Fail("dependency edge count fixture registration failed");
    }

    if (registry.AddDependency(root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("dependency edge count first edge failed");
    }

    if (registry.AddDependency(shared.handle, leaf.handle) != ResourceStatus::Success) {
        return Fail("dependency edge count second edge failed");
    }

    if (registry.AddDependency(second_root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("dependency edge count third edge failed");
    }

    std::uint32_t count_snapshot = 77U;
    const ResourceSnapshot before_count_snapshot = registry.Snapshot();
    const ResourceStatus count_status = registry.CountDependencyEdges(&count_snapshot);
    if (count_status != ResourceStatus::Success || count_snapshot != 3U) {
        return Fail("dependency edge count did not match direct edge count");
    }

    const ResourceSnapshot after_count_snapshot = registry.Snapshot();
    if (after_count_snapshot.dependency_edge_count != before_count_snapshot.dependency_edge_count ||
        after_count_snapshot.dependency_validation_count != before_count_snapshot.dependency_validation_count) {
        return Fail("dependency edge count success mutated dependency counters");
    }

    const std::uint32_t stable_count_snapshot = count_snapshot;
    const ResourceStatus null_output_status = registry.CountDependencyEdges(nullptr);
    if (null_output_status != ResourceStatus::InvalidHandle || count_snapshot != stable_count_snapshot) {
        return Fail("dependency edge count null output mutated previous count");
    }

    bool edge_exists = false;
    const ResourceStatus direct_lookup_status = registry.FindDependencyEdge(
        root.handle,
        shared.handle,
        &edge_exists);
    if (direct_lookup_status != ResourceStatus::Success || !edge_exists) {
        return Fail("dependency edge count direct lookup setup failed");
    }

    std::uint32_t after_lookup_count = 0U;
    const ResourceStatus after_lookup_status = registry.CountDependencyEdges(&after_lookup_count);
    if (after_lookup_status != ResourceStatus::Success || after_lookup_count != stable_count_snapshot) {
        return Fail("dependency edge count changed after direct lookup");
    }

    return 0;
}

bool DependencyRequestMatches(
    const ResourceDependencyRequest &request,
    ResourceHandle dependent,
    ResourceHandle dependency) {
    if (request.dependent.slot != dependent.slot ||
        request.dependent.generation != dependent.generation) {
        return false;
    }

    if (request.dependency.slot != dependency.slot ||
        request.dependency.generation != dependency.generation) {
        return false;
    }

    return true;
}

int ResourceDependencyEdgeEnumerationReportsCommittedRows() {
    ResourceRegistry empty_registry;
    std::uint32_t empty_dependency_count = 7U;
    const ResourceSnapshot empty_before_snapshot = empty_registry.Snapshot();
    const ResourceStatus empty_status = empty_registry.EnumerateDependencyEdges(
        nullptr,
        0U,
        &empty_dependency_count);
    if (empty_status != ResourceStatus::Success || empty_dependency_count != 0U) {
        return Fail("dependency edge enumeration empty graph did not succeed");
    }

    const ResourceSnapshot empty_after_snapshot = empty_registry.Snapshot();
    if (empty_after_snapshot.dependency_edge_count != empty_before_snapshot.dependency_edge_count ||
        empty_after_snapshot.dependency_validation_count != empty_before_snapshot.dependency_validation_count) {
        return Fail("dependency edge enumeration empty graph mutated dependency counters");
    }

    ResourceRegistry registry;
    const ResourceRegistrationResult root = Register(registry, TYPE_MATERIAL, "enumerate_root");
    const ResourceRegistrationResult shared = Register(registry, TYPE_TEXTURE, "enumerate_shared");
    const ResourceRegistrationResult leaf = Register(registry, TYPE_AUDIO, "enumerate_leaf");
    const ResourceRegistrationResult second_root = Register(registry, TYPE_EFFECT, "enumerate_second_root");
    if (!root.Succeeded() ||
        !shared.Succeeded() ||
        !leaf.Succeeded() ||
        !second_root.Succeeded()) {
        return Fail("dependency edge enumeration fixture registration failed");
    }

    if (registry.AddDependency(root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("dependency edge enumeration single edge failed");
    }

    const std::array<ResourceDependencyRequest, 2U> batch_rows{{
        ResourceDependencyRequest{shared.handle, leaf.handle},
        ResourceDependencyRequest{second_root.handle, shared.handle}}};
    const ResourceDependencyBatchResult batch_result = registry.AddDependencies(
        batch_rows.data(),
        static_cast<std::uint32_t>(batch_rows.size()));
    if (!batch_result.Succeeded()) {
        return Fail("dependency edge enumeration batch fixture failed");
    }

    std::array<ResourceDependencyRequest, 3U> output_dependencies{};
    std::uint32_t output_dependency_count = 0U;
    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceStatus enumerate_status = registry.EnumerateDependencyEdges(
        output_dependencies.data(),
        static_cast<std::uint32_t>(output_dependencies.size()),
        &output_dependency_count);
    if (enumerate_status != ResourceStatus::Success || output_dependency_count != 3U) {
        return Fail("dependency edge enumeration success returned wrong count");
    }

    if (!DependencyRequestMatches(output_dependencies[0U], root.handle, shared.handle) ||
        !DependencyRequestMatches(output_dependencies[1U], shared.handle, leaf.handle) ||
        !DependencyRequestMatches(output_dependencies[2U], second_root.handle, shared.handle)) {
        return Fail("dependency edge enumeration did not preserve committed edge order");
    }

    const ResourceSnapshot after_success_snapshot = registry.Snapshot();
    if (after_success_snapshot.dependency_edge_count != before_success_snapshot.dependency_edge_count ||
        after_success_snapshot.dependency_validation_count != before_success_snapshot.dependency_validation_count) {
        return Fail("dependency edge enumeration success mutated dependency counters");
    }

    std::array<ResourceDependencyRequest, 2U> small_output_dependencies{};
    std::uint32_t required_dependency_count = 0U;
    const ResourceSnapshot before_capacity_snapshot = registry.Snapshot();
    const ResourceStatus capacity_status = registry.EnumerateDependencyEdges(
        small_output_dependencies.data(),
        static_cast<std::uint32_t>(small_output_dependencies.size()),
        &required_dependency_count);
    if (capacity_status != ResourceStatus::CapacityExceeded || required_dependency_count != 3U) {
        return Fail("dependency edge enumeration capacity failure missed required count");
    }

    const ResourceSnapshot after_capacity_snapshot = registry.Snapshot();
    if (after_capacity_snapshot.dependency_edge_count != before_capacity_snapshot.dependency_edge_count ||
        after_capacity_snapshot.dependency_validation_count != before_capacity_snapshot.dependency_validation_count ||
        after_capacity_snapshot.last_required_dependency_edge_count != 3U) {
        return Fail("dependency edge enumeration capacity failure mutated counters or missed snapshot count");
    }

    std::uint32_t invalid_dependency_count = 0U;
    const ResourceSnapshot before_invalid_snapshot = registry.Snapshot();
    const ResourceStatus invalid_status = registry.EnumerateDependencyEdges(
        nullptr,
        1U,
        &invalid_dependency_count);
    if (invalid_status != ResourceStatus::InvalidHandle || invalid_dependency_count != 0U) {
        return Fail("dependency edge enumeration invalid output did not fail explicitly");
    }

    const ResourceSnapshot after_invalid_snapshot = registry.Snapshot();
    if (after_invalid_snapshot.dependency_edge_count != before_invalid_snapshot.dependency_edge_count ||
        after_invalid_snapshot.dependency_validation_count != before_invalid_snapshot.dependency_validation_count) {
        return Fail("dependency edge enumeration invalid output mutated dependency counters");
    }

    return 0;
}

int ResourceDependencyTraversalReturnsExplicitClosureHandles() {
    ResourceRegistry registry;
    const ResourceRegistrationResult root = Register(registry, TYPE_MATERIAL, "material_root");
    const ResourceRegistrationResult texture = Register(registry, TYPE_TEXTURE, "texture_dependency");
    const ResourceRegistrationResult effect = Register(registry, TYPE_EFFECT, "effect_dependency");
    if (!root.Succeeded()) {
        return Fail("dependency traversal root registration failed");
    }

    if (!texture.Succeeded()) {
        return Fail("dependency traversal texture registration failed");
    }

    if (!effect.Succeeded()) {
        return Fail("dependency traversal effect registration failed");
    }

    if (registry.AddDependency(root.handle, texture.handle) != ResourceStatus::Success) {
        return Fail("dependency traversal direct edge failed");
    }

    if (registry.AddDependency(texture.handle, effect.handle) != ResourceStatus::Success) {
        return Fail("dependency traversal transitive edge failed");
    }

    std::array<ResourceHandle, 2U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const ResourceStatus traverse_status = registry.TraverseDependencies(
        root.handle,
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        &dependency_count);
    if (traverse_status != ResourceStatus::Success) {
        return Fail("dependency traversal failed");
    }

    if (dependency_count != 2U) {
        return Fail("dependency traversal returned wrong count");
    }

    if (dependencies[0U].slot != texture.handle.slot ||
        dependencies[0U].generation != texture.handle.generation) {
        return Fail("dependency traversal missed direct handle");
    }

    if (dependencies[1U].slot != effect.handle.slot ||
        dependencies[1U].generation != effect.handle.generation) {
        return Fail("dependency traversal missed transitive handle");
    }

    std::array<ResourceHandle, 1U> small_dependencies{};
    std::uint32_t required_dependency_count = 0U;
    const ResourceSnapshot before_small_snapshot = registry.Snapshot();
    const ResourceStatus small_status = registry.TraverseDependencies(
        root.handle,
        small_dependencies.data(),
        static_cast<std::uint32_t>(small_dependencies.size()),
        &required_dependency_count);
    if (small_status != ResourceStatus::CapacityExceeded) {
        return Fail("dependency traversal small buffer did not fail deterministically");
    }

    if (required_dependency_count != 2U) {
        return Fail("dependency traversal small buffer missed required count");
    }

    const ResourceSnapshot after_small_snapshot = registry.Snapshot();
    if (after_small_snapshot.dependency_edge_count != before_small_snapshot.dependency_edge_count) {
        return Fail("dependency traversal small buffer mutated edge count");
    }

    if (after_small_snapshot.last_required_dependency_edge_count != 2U) {
        return Fail("dependency traversal small buffer missed snapshot required count");
    }

    return 0;
}

int ResourceDependencyTraversalMultiRootDeduplicatesClosureHandles() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first_root = Register(registry, TYPE_MATERIAL, "material_root_a");
    const ResourceRegistrationResult second_root = Register(registry, TYPE_EFFECT, "effect_root_b");
    const ResourceRegistrationResult shared = Register(registry, TYPE_TEXTURE, "texture_shared");
    const ResourceRegistrationResult leaf = Register(registry, TYPE_AUDIO, "audio_leaf");
    if (!first_root.Succeeded()) {
        return Fail("multi-root traversal first root registration failed");
    }

    if (!second_root.Succeeded()) {
        return Fail("multi-root traversal second root registration failed");
    }

    if (!shared.Succeeded()) {
        return Fail("multi-root traversal shared registration failed");
    }

    if (!leaf.Succeeded()) {
        return Fail("multi-root traversal leaf registration failed");
    }

    if (registry.AddDependency(first_root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("multi-root traversal first edge failed");
    }

    if (registry.AddDependency(second_root.handle, shared.handle) != ResourceStatus::Success) {
        return Fail("multi-root traversal second edge failed");
    }

    if (registry.AddDependency(shared.handle, leaf.handle) != ResourceStatus::Success) {
        return Fail("multi-root traversal transitive edge failed");
    }

    const std::array<ResourceHandle, 3U> roots{
        first_root.handle,
        second_root.handle,
        first_root.handle};
    std::array<ResourceHandle, 2U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const ResourceStatus traversal_status = registry.TraverseDependencies(
        roots.data(),
        static_cast<std::uint32_t>(roots.size()),
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        &dependency_count);
    if (traversal_status != ResourceStatus::Success) {
        return Fail("multi-root traversal failed");
    }

    if (dependency_count != 2U) {
        return Fail("multi-root traversal returned wrong dependency count");
    }

    if (dependencies[0U].slot != shared.handle.slot ||
        dependencies[0U].generation != shared.handle.generation) {
        return Fail("multi-root traversal missed shared dependency handle");
    }

    if (dependencies[1U].slot != leaf.handle.slot ||
        dependencies[1U].generation != leaf.handle.generation) {
        return Fail("multi-root traversal missed transitive dependency handle");
    }

    std::array<ResourceHandle, 1U> small_dependencies{};
    std::uint32_t required_dependency_count = 0U;
    const ResourceSnapshot before_small_snapshot = registry.Snapshot();
    const ResourceStatus small_status = registry.TraverseDependencies(
        roots.data(),
        static_cast<std::uint32_t>(roots.size()),
        small_dependencies.data(),
        static_cast<std::uint32_t>(small_dependencies.size()),
        &required_dependency_count);
    if (small_status != ResourceStatus::CapacityExceeded) {
        return Fail("multi-root traversal small buffer did not fail deterministically");
    }

    if (required_dependency_count != 2U) {
        return Fail("multi-root traversal small buffer missed required count");
    }

    const ResourceSnapshot after_small_snapshot = registry.Snapshot();
    if (after_small_snapshot.dependency_edge_count != before_small_snapshot.dependency_edge_count) {
        return Fail("multi-root traversal small buffer mutated edge count");
    }

    if (after_small_snapshot.last_required_dependency_edge_count != 2U) {
        return Fail("multi-root traversal small buffer missed snapshot required count");
    }

    std::uint32_t empty_dependency_count = 7U;
    const ResourceStatus empty_status = registry.TraverseDependencies(
        nullptr,
        0U,
        nullptr,
        0U,
        &empty_dependency_count);
    if (empty_status != ResourceStatus::Success || empty_dependency_count != 0U) {
        return Fail("multi-root traversal empty root list did not succeed with zero output");
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

int ResourceLoadCommitCapacityEntryRecordsRejectedIdentity() {
    ResourceRegistry registry;
    ResourceHandle retired_handle{};
    ResourceHandle first_committed_handle{};
    std::uint32_t commit_index = 0U;
    while (commit_index < MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT) {
        const std::string key = "texture_load_commit_capacity_" + std::to_string(commit_index);
        const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, key.c_str());
        if (!result.Succeeded()) {
            return Fail("load commit capacity fixture registration failed");
        }

        ResourceLoadCommitRequest request = LoadCommitRequest(
            result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Uploaded,
            COMMIT_ONE + commit_index,
            UPLOAD_ONE + commit_index);
        request.staging_request_id = STAGING_ONE + commit_index;
        request.upload_byte_count = UPLOAD_BYTE_COUNT + commit_index;
        if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::Success) {
            return Fail("load commit capacity fixture commit failed");
        }

        if (commit_index == 0U) {
            retired_handle = result.handle;
        }

        if (commit_index == 1U) {
            first_committed_handle = result.handle;
        }

        ++commit_index;
    }

    const ResourceStatus setup_retire_status = registry.Retire(retired_handle);
    if (setup_retire_status != ResourceStatus::Success) {
        return Fail("load commit capacity fixture retire failed");
    }

    const ResourceRegistrationResult overflow_result =
        Register(registry, TYPE_TEXTURE, "texture_load_commit_capacity_overflow");
    if (!overflow_result.Succeeded()) {
        return Fail("load commit capacity overflow registration failed");
    }

    ResourceLoadCommitRequest overflow_request = LoadCommitRequest(
        overflow_result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 1U,
        UPLOAD_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 1U);
    overflow_request.staging_request_id = STAGING_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 1U;
    overflow_request.upload_byte_count = UPLOAD_BYTE_COUNT + 7U;
    const ResourceSnapshot before_capacity_snapshot = registry.Snapshot();
    const ResourceLoadCommitStatus capacity_status = registry.CommitUploadCompletion(overflow_request);
    if (capacity_status != ResourceLoadCommitStatus::CapacityExceeded) {
        return Fail("load commit capacity entry returned wrong status");
    }

    if (!LoadStateMatches(registry, overflow_result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("load commit capacity entry changed overflow resource state");
    }

    const ResourceSnapshot capacity_snapshot = registry.Snapshot();
    if (capacity_snapshot.load_commit_record_count != before_capacity_snapshot.load_commit_record_count) {
        return Fail("load commit capacity entry mutated record count");
    }

    if (capacity_snapshot.load_commit_count != before_capacity_snapshot.load_commit_count) {
        return Fail("load commit capacity entry mutated commit count");
    }

    const std::uint32_t required_load_commit_count = before_capacity_snapshot.load_commit_record_count + 1U;
    int entry_status = ExpectLoadCommitCapacityEntryMatches(
        capacity_snapshot,
        overflow_request,
        before_capacity_snapshot.load_commit_record_count,
        required_load_commit_count);
    if (entry_status != 0) {
        return entry_status;
    }

    const ResourceLoadCommitRequest duplicate_request = LoadCommitRequest(
        overflow_result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_TWO);
    const ResourceLoadCommitStatus duplicate_status = registry.CommitUploadCompletion(duplicate_request);
    if (duplicate_status != ResourceLoadCommitStatus::DuplicateCommitId) {
        return Fail("load commit capacity entry duplicate returned wrong status");
    }

    ResourceSnapshot cleared_snapshot = registry.Snapshot();
    int clear_status = ExpectLoadCommitCapacityEntryCleared(cleared_snapshot);
    if (clear_status != 0) {
        return clear_status;
    }

    if (registry.CommitUploadCompletion(overflow_request) != ResourceLoadCommitStatus::CapacityExceeded) {
        return Fail("load commit capacity entry second overflow failed");
    }

    const ResourceLoadCommitRequest type_mismatch_request = LoadCommitRequest(
        overflow_result.handle,
        TYPE_AUDIO,
        ResourceLoadState::Uploaded,
        COMMIT_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 2U,
        UPLOAD_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 2U);
    const ResourceLoadCommitStatus type_mismatch_status = registry.CommitUploadCompletion(type_mismatch_request);
    if (type_mismatch_status != ResourceLoadCommitStatus::TypeMismatch) {
        return Fail("load commit capacity entry type mismatch returned wrong status");
    }

    cleared_snapshot = registry.Snapshot();
    clear_status = ExpectLoadCommitCapacityEntryCleared(cleared_snapshot);
    if (clear_status != 0) {
        return clear_status;
    }

    if (registry.CommitUploadCompletion(overflow_request) != ResourceLoadCommitStatus::CapacityExceeded) {
        return Fail("load commit capacity entry third overflow failed");
    }

    const ResourceLoadCommitRequest invalid_transition_request = LoadCommitRequest(
        first_committed_handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 3U,
        UPLOAD_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 3U);
    const ResourceLoadCommitStatus invalid_transition_status =
        registry.CommitUploadCompletion(invalid_transition_request);
    if (invalid_transition_status != ResourceLoadCommitStatus::InvalidTransition) {
        return Fail("load commit capacity entry invalid transition returned wrong status");
    }

    cleared_snapshot = registry.Snapshot();
    clear_status = ExpectLoadCommitCapacityEntryCleared(cleared_snapshot);
    if (clear_status != 0) {
        return clear_status;
    }

    if (registry.CommitUploadCompletion(overflow_request) != ResourceLoadCommitStatus::CapacityExceeded) {
        return Fail("load commit capacity entry fourth overflow failed");
    }

    const ResourceLoadCommitRequest invalid_handle_request = LoadCommitRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 4U,
        UPLOAD_ONE + MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT + 4U);
    const ResourceLoadCommitStatus invalid_handle_status = registry.CommitUploadCompletion(invalid_handle_request);
    if (invalid_handle_status != ResourceLoadCommitStatus::InvalidHandle) {
        return Fail("load commit capacity entry invalid handle returned wrong status");
    }

    cleared_snapshot = registry.Snapshot();
    clear_status = ExpectLoadCommitCapacityEntryCleared(cleared_snapshot);
    if (clear_status != 0) {
        return clear_status;
    }

    if (registry.CommitUploadCompletion(overflow_request) != ResourceLoadCommitStatus::CapacityExceeded) {
        return Fail("load commit capacity entry fifth overflow failed");
    }

    const ResourceStatus retire_status = registry.Retire(overflow_result.handle);
    if (retire_status != ResourceStatus::Success) {
        return Fail("load commit capacity entry retire clear failed");
    }

    cleared_snapshot = registry.Snapshot();
    clear_status = ExpectLoadCommitCapacityEntryCleared(cleared_snapshot);
    if (clear_status != 0) {
        return clear_status;
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

    if (snapshot.last_required_resident_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("residency budget required bytes were not reported");
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

    if (snapshot.last_required_payload_byte_count != payload_byte_count) {
        return Fail("capacity cache payload required bytes were not reported");
    }

    if (snapshot.last_required_payload_reference_count != 0U) {
        return Fail("capacity cache payload reported reference count");
    }

    if (!ResourceHandlesMatch(snapshot.last_failed_resource, result.handle)) {
        return Fail("capacity cache payload missed failed resource");
    }

    if (snapshot.last_failed_expected_type.value != TYPE_TEXTURE.value) {
        return Fail("capacity cache payload missed failed type");
    }

    if (snapshot.last_failed_payload_id != PAYLOAD_ONE) {
        return Fail("capacity cache payload missed failed payload id");
    }

    if (snapshot.last_failed_payload_byte_capacity != MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD) {
        return Fail("capacity cache payload missed failed byte capacity");
    }

    if (snapshot.last_failed_payload_byte_count != payload_byte_count) {
        return Fail("capacity cache payload missed failed byte count");
    }

    if (snapshot.last_failed_payload_reference_capacity != MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return Fail("capacity cache payload missed failed reference capacity");
    }

    if (snapshot.last_failed_payload_current_reference_count != 0U) {
        return Fail("capacity cache payload missed current reference count");
    }

    if (snapshot.last_failed_payload_reference_count != 0U) {
        return Fail("capacity cache payload reported failed reference count");
    }

    if (snapshot.cached_payload_count != 0U) {
        return Fail("capacity cache payload request stored payload");
    }

    const std::array<std::uint8_t, 4U> recovery_payload{{2U, 3U, 4U, 5U}};
    const ResourceCachePayloadRequest recovery_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        recovery_payload.data(),
        static_cast<std::uint32_t>(recovery_payload.size()));
    if (registry.StoreCachePayload(recovery_request) != ResourceCachePayloadStatus::Success) {
        return Fail("capacity cache payload recovery store failed");
    }

    if (!CachePayloadFailedEntryIsClear(registry.CachePayloadSnapshot())) {
        return Fail("successful cache payload store left stale failed entry");
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

    if (snapshot.last_required_payload_byte_count != payload_byte_count) {
        return Fail("budget cache payload required bytes were not reported");
    }

    if (snapshot.last_required_payload_reference_count != 0U) {
        return Fail("budget cache payload reported reference count");
    }

    if (!ResourceHandlesMatch(snapshot.last_failed_resource, result.handle)) {
        return Fail("budget cache payload missed failed resource");
    }

    if (snapshot.last_failed_expected_type.value != TYPE_TEXTURE.value) {
        return Fail("budget cache payload missed failed type");
    }

    if (snapshot.last_failed_payload_id != PAYLOAD_ONE) {
        return Fail("budget cache payload missed failed payload id");
    }

    if (snapshot.last_failed_payload_byte_capacity != 3U) {
        return Fail("budget cache payload missed failed byte capacity");
    }

    if (snapshot.last_failed_payload_byte_count != payload_byte_count) {
        return Fail("budget cache payload missed failed byte count");
    }

    if (snapshot.last_failed_payload_reference_capacity != MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return Fail("budget cache payload missed failed reference capacity");
    }

    if (snapshot.last_failed_payload_current_reference_count != 0U) {
        return Fail("budget cache payload missed current reference count");
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

    std::array<std::uint8_t, 2U> output{{90U, 91U}};
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

    if (snapshot.last_required_payload_byte_count != payload_byte_count) {
        return Fail("small output cache payload missed required byte count");
    }

    if (!ResourceHandlesMatch(snapshot.last_failed_resource, result.handle)) {
        return Fail("small output cache payload missed failed resource");
    }

    if (snapshot.last_failed_expected_type.value != TYPE_TEXTURE.value) {
        return Fail("small output cache payload missed failed type");
    }

    if (snapshot.last_failed_payload_id != PAYLOAD_ONE) {
        return Fail("small output cache payload missed failed payload id");
    }

    if (snapshot.last_failed_payload_byte_count != payload_byte_count) {
        return Fail("small output cache payload missed failed byte count");
    }

    if (snapshot.last_failed_payload_reference_capacity != MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT) {
        return Fail("small output cache payload missed failed reference capacity");
    }

    if (snapshot.last_failed_payload_current_reference_count != 1U) {
        return Fail("small output cache payload missed current reference count");
    }

    if (output[0U] != 90U || output[1U] != 91U) {
        return Fail("small output cache payload read mutated output bytes");
    }

    std::array<std::uint8_t, 4U> recovery_output{};
    std::uint32_t recovery_output_byte_count = 0U;
    if (registry.ReadCachePayload(
        read_request,
        recovery_output.data(),
        static_cast<std::uint32_t>(recovery_output.size()),
        &recovery_output_byte_count) != ResourceCachePayloadStatus::Success) {
        return Fail("small output cache payload recovery read failed");
    }

    if (!CachePayloadFailedEntryIsClear(registry.CachePayloadSnapshot())) {
        return Fail("successful cache payload read left stale failed entry");
    }

    return 0;
}

int ResourceCachePayloadStoresWindowMetadataAndReferenceBudget() {
    ResourceRegistry registry;
    if (!ConfigureCachePayloadBudget(registry, 128U, 1U)) {
        return Fail("window cache payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_window");
    if (!result.Succeeded()) {
        return Fail("window cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("window cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{41U, 42U, 43U, 44U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const std::uint64_t payload_window_byte_offset = 1024U;
    const std::uint64_t payload_window_byte_size = payload_byte_count;
    const ResourceCachePayloadRequest store_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count,
        payload_window_byte_offset,
        payload_window_byte_size);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("window cache payload store failed");
    }

    const ResourceCachePayloadSnapshot store_snapshot = registry.CachePayloadSnapshot();
    if (store_snapshot.budget_payload_reference_capacity != 1U) {
        return Fail("window cache payload reference budget was not configured");
    }

    if (store_snapshot.last_payload_window_byte_offset != payload_window_byte_offset) {
        return Fail("window cache payload offset was not tracked");
    }

    if (store_snapshot.last_payload_window_byte_size != payload_window_byte_size) {
        return Fail("window cache payload size was not tracked");
    }

    std::array<std::uint8_t, 4U> whole_output{};
    std::uint32_t whole_output_byte_count = 0U;
    const ResourceCachePayloadRequest whole_read_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    if (registry.ReadCachePayload(
        whole_read_request,
        whole_output.data(),
        static_cast<std::uint32_t>(whole_output.size()),
        &whole_output_byte_count) != ResourceCachePayloadStatus::Success) {
        return Fail("window cache payload whole read failed");
    }

    if (!BytesMatch(payload.data(), whole_output.data(), payload_byte_count)) {
        return Fail("window cache payload whole read returned wrong bytes");
    }

    std::array<std::uint8_t, 2U> output{};
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest read_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U,
        payload_window_byte_offset + 1U,
        static_cast<std::uint64_t>(output.size()));
    const ResourceCachePayloadStatus read_status = registry.ReadCachePayload(
        read_request,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("window cache payload sub read failed");
    }

    if (output_byte_count != static_cast<std::uint32_t>(output.size())) {
        return Fail("window cache payload sub read returned wrong byte count");
    }

    if (output[0U] != payload[1U]) {
        return Fail("window cache payload sub read returned wrong first byte");
    }

    if (output[1U] != payload[2U]) {
        return Fail("window cache payload sub read returned wrong second byte");
    }

    return 0;
}

int ResourceCachePayloadStoresU64LogicalWindowWithU32LocalBytes() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_u64_window");
    if (!result.Succeeded()) {
        return Fail("u64 window cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("u64 window cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{61U, 62U, 63U, 64U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const std::uint64_t payload_window_byte_size = payload_byte_count;
    ResourceCachePayloadRequest store_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count,
        U64_PAYLOAD_WINDOW_BYTE_OFFSET,
        payload_window_byte_size);
    store_request.payload_logical_byte_count = U64_LOGICAL_PAYLOAD_BYTE_COUNT;
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("u64 window cache payload store failed");
    }

    const ResourceCachePayloadSnapshot store_snapshot = registry.CachePayloadSnapshot();
    if (store_snapshot.last_payload_logical_byte_count != U64_LOGICAL_PAYLOAD_BYTE_COUNT) {
        return Fail("u64 window cache payload logical total was not tracked");
    }

    if (store_snapshot.last_payload_window_byte_offset != U64_PAYLOAD_WINDOW_BYTE_OFFSET) {
        return Fail("u64 window cache payload offset was not tracked");
    }

    if (store_snapshot.last_payload_window_byte_size != payload_window_byte_size) {
        return Fail("u64 window cache payload size was not tracked");
    }

    std::array<std::uint8_t, 2U> output{};
    std::uint32_t output_byte_count = 0U;
    const std::uint64_t read_window_byte_offset = U64_PAYLOAD_WINDOW_BYTE_OFFSET + 1U;
    const std::uint64_t read_window_byte_size = static_cast<std::uint64_t>(output.size());
    ResourceCachePayloadRequest read_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U,
        read_window_byte_offset,
        read_window_byte_size);
    read_request.payload_logical_byte_count = U64_LOGICAL_PAYLOAD_BYTE_COUNT;
    const std::uint32_t output_byte_capacity = static_cast<std::uint32_t>(output.size());
    const ResourceCachePayloadStatus read_status = registry.ReadCachePayload(
        read_request,
        output.data(),
        output_byte_capacity,
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("u64 window cache payload read failed");
    }

    if (output_byte_count != output_byte_capacity) {
        return Fail("u64 window cache payload read returned wrong byte count");
    }

    if (output[0U] != payload[1U]) {
        return Fail("u64 window cache payload read returned wrong first byte");
    }

    if (output[1U] != payload[2U]) {
        return Fail("u64 window cache payload read returned wrong second byte");
    }

    return 0;
}

int ResourceCachePayloadRejectsWindowOverflowWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_window_overflow");
    if (!result.Succeeded()) {
        return Fail("window overflow cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("window overflow cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{45U, 46U, 47U, 48U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceCachePayloadSnapshot before_cache_snapshot = registry.CachePayloadSnapshot();
    const ResourceCachePayloadRequest request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count,
        std::numeric_limits<std::uint64_t>::max(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(request);
    if (status != ResourceCachePayloadStatus::PayloadWindowOutOfBounds) {
        return Fail("window overflow cache payload returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("window overflow cache payload changed registered count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("window overflow cache payload changed resident count");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("window overflow cache payload changed active count");
    }

    if (after_cache_snapshot.cached_byte_count != before_cache_snapshot.cached_byte_count) {
        return Fail("window overflow cache payload changed cached bytes");
    }

    if (after_cache_snapshot.payload_window_rejected_count != 1U) {
        return Fail("window overflow cache payload was not counted");
    }

    return 0;
}

int ResourceCachePayloadRejectsReferenceBudgetWithoutMutation() {
    ResourceRegistry registry;
    if (!ConfigureCachePayloadBudget(registry, 128U, 1U)) {
        return Fail("reference budget cache payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_cache_reference_budget");
    if (!result.Succeeded()) {
        return Fail("reference budget cache payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("reference budget cache payload fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> first_payload{{49U, 50U, 51U, 52U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(first_payload.size());
    const ResourceCachePayloadRequest first_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        first_payload.data(),
        payload_byte_count);
    if (registry.StoreCachePayload(first_request) != ResourceCachePayloadStatus::Success) {
        return Fail("reference budget first cache payload store failed");
    }

    const ResourceCachePayloadSnapshot before_snapshot = registry.CachePayloadSnapshot();
    const std::array<std::uint8_t, 4U> second_payload{{53U, 54U, 55U, 56U}};
    const ResourceCachePayloadRequest second_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        second_payload.data(),
        payload_byte_count);
    const ResourceCachePayloadStatus status = registry.StoreCachePayload(second_request);
    if (status != ResourceCachePayloadStatus::ReferenceBudgetExceeded) {
        return Fail("reference budget cache payload returned wrong status");
    }

    const ResourceCachePayloadSnapshot after_snapshot = registry.CachePayloadSnapshot();
    if (after_snapshot.cached_payload_count != before_snapshot.cached_payload_count) {
        return Fail("reference budget cache payload changed active count");
    }

    if (after_snapshot.cached_byte_count != before_snapshot.cached_byte_count) {
        return Fail("reference budget cache payload changed cached bytes");
    }

    if (after_snapshot.reference_budget_rejected_payload_count != 1U) {
        return Fail("reference budget cache payload was not counted");
    }

    const std::uint32_t required_payload_reference_count = before_snapshot.cached_payload_count + 1U;
    if (after_snapshot.last_required_payload_reference_count != required_payload_reference_count) {
        return Fail("reference budget cache payload required references were not reported");
    }

    if (after_snapshot.last_required_payload_byte_count != 0U) {
        return Fail("reference budget cache payload reported byte count");
    }

    if (!ResourceHandlesMatch(after_snapshot.last_failed_resource, result.handle)) {
        return Fail("reference budget cache payload missed failed resource");
    }

    if (after_snapshot.last_failed_expected_type.value != TYPE_TEXTURE.value) {
        return Fail("reference budget cache payload missed failed type");
    }

    if (after_snapshot.last_failed_payload_id != PAYLOAD_TWO) {
        return Fail("reference budget cache payload missed failed payload id");
    }

    if (after_snapshot.last_failed_payload_byte_count != payload_byte_count) {
        return Fail("reference budget cache payload missed failed byte count");
    }

    if (after_snapshot.last_failed_payload_reference_capacity != 1U) {
        return Fail("reference budget cache payload missed failed reference capacity");
    }

    if (after_snapshot.last_failed_payload_current_reference_count != before_snapshot.cached_payload_count) {
        return Fail("reference budget cache payload missed current reference count");
    }

    if (after_snapshot.last_failed_payload_reference_count != required_payload_reference_count) {
        return Fail("reference budget cache payload missed failed reference count");
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

    if (!CachePayloadFailedEntryIsClear(after_cache_snapshot)) {
        return Fail("invalid cache payload request reported failed capacity entry");
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

    constexpr std::uint32_t REQUIRED_PLAN_COUNT = MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT + 1U;
    if (snapshot.last_required_plan_count != REQUIRED_PLAN_COUNT) {
        return Fail("capacity decode plan required count was not reported");
    }

    if (snapshot.last_required_decoded_byte_count != 0U) {
        return Fail("capacity decode plan reported decoded byte count");
    }

    const bool capacity_entry_matches = DecodePlanCapacityEntryMatches(
        snapshot,
        ResourceDecodePlanOperation::Create,
        overflow_request,
        MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT,
        MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT,
        REQUIRED_PLAN_COUNT);
    if (!capacity_entry_matches) {
        return Fail("capacity decode plan failed entry mismatch");
    }

    ResourceDecodePlanRequest invalid_request = overflow_request;
    invalid_request.decode_plan_id = 0U;
    const ResourceDecodePlanStatus invalid_status = registry.CreateDecodePlan(invalid_request);
    if (invalid_status != ResourceDecodePlanStatus::InvalidArgument) {
        return Fail("capacity decode plan invalid request returned wrong status");
    }

    const ResourceDecodePlanSnapshot invalid_snapshot = registry.DecodePlanSnapshot();
    if (!DecodePlanCapacityEntryIsClear(invalid_snapshot)) {
        return Fail("capacity decode plan invalid request left stale entry");
    }

    const ResourceDecodePlanStatus second_status = registry.CreateDecodePlan(overflow_request);
    if (second_status != ResourceDecodePlanStatus::CapacityExceeded) {
        return Fail("capacity decode plan second overflow returned wrong status");
    }

    ResourceDecodePlanRecord queried_record{};
    const ResourceDecodePlanRequest query_request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        1U);
    const ResourceDecodePlanStatus query_status = registry.QueryDecodePlan(query_request, &queried_record);
    if (query_status != ResourceDecodePlanStatus::Success) {
        return Fail("capacity decode plan query cleanup failed");
    }

    const ResourceDecodePlanSnapshot query_snapshot = registry.DecodePlanSnapshot();
    if (!DecodePlanCapacityEntryIsClear(query_snapshot)) {
        return Fail("capacity decode plan query success left stale entry");
    }

    const ResourceDecodePlanStatus third_status = registry.CreateDecodePlan(overflow_request);
    if (third_status != ResourceDecodePlanStatus::CapacityExceeded) {
        return Fail("capacity decode plan third overflow returned wrong status");
    }

    const ResourceDecodePlanStatus release_status = registry.ReleaseDecodePlan(query_request);
    if (release_status != ResourceDecodePlanStatus::Success) {
        return Fail("capacity decode plan release cleanup failed");
    }

    const ResourceDecodePlanSnapshot release_snapshot = registry.DecodePlanSnapshot();
    if (!DecodePlanCapacityEntryIsClear(release_snapshot)) {
        return Fail("capacity decode plan release success left stale entry");
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

    if (snapshot.last_required_decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("budget decode plan required decoded bytes were not reported");
    }

    if (snapshot.last_required_plan_count != 0U) {
        return Fail("budget decode plan reported plan count");
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

int ResourceDecodeResultCommitsQueriesAndReleasesMetadata() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_success");
    if (!result.Succeeded()) {
        return Fail("decode result success fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("decode result success fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("decode result success fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CommitDecodeResult(request) != ResourceDecodeResultStatus::Success) {
        return Fail("decode result commit failed");
    }

    ResourceDecodeResultRecord record;
    if (registry.QueryDecodeResult(request, &record) != ResourceDecodeResultStatus::Success) {
        return Fail("decode result query failed");
    }

    if (record.decode_result_id != DECODE_RESULT_ONE) {
        return Fail("decode result query returned wrong id");
    }

    if (record.decode_plan_id != DECODE_PLAN_ONE) {
        return Fail("decode result query returned wrong plan id");
    }

    if (record.result_class != ResourceDecodeResultClass::Texture) {
        return Fail("decode result query returned wrong result class");
    }

    if (record.decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("decode result query returned wrong decoded byte count");
    }

    ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 1U) {
        return Fail("decode result commit did not track active count");
    }

    if (snapshot.committed_decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("decode result commit did not track decoded bytes");
    }

    if (snapshot.committed_result_count != 1U) {
        return Fail("decode result commit was not counted");
    }

    if (snapshot.queried_result_count != 1U) {
        return Fail("decode result query was not counted");
    }

    if (registry.ReleaseDecodeResult(request) != ResourceDecodeResultStatus::Success) {
        return Fail("decode result release failed");
    }

    snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 0U) {
        return Fail("decode result release left active result");
    }

    if (snapshot.committed_decoded_byte_count != 0U) {
        return Fail("decode result release left decoded bytes");
    }

    if (snapshot.released_result_count != 1U) {
        return Fail("decode result release was not counted");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 1U) {
        return Fail("decode result release changed decode plan count");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 1U) {
        return Fail("decode result release changed cache payload count");
    }

    const ResourceDecodeResultStatus query_status = registry.QueryDecodeResult(request, &record);
    if (query_status != ResourceDecodeResultStatus::MissingDecodeResult) {
        return Fail("released decode result query returned wrong status");
    }

    return 0;
}

int ResourceDecodeResultRejectsMissingDecodePlan() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_missing_plan");
    if (!result.Succeeded()) {
        return Fail("missing plan decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("missing plan decode result fixture residency failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::MissingDecodePlan) {
        return Fail("missing plan decode result request returned wrong status");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("missing plan decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultRejectsStaleHandleWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_stale");
    if (!result.Succeeded()) {
        return Fail("stale decode result fixture registration failed");
    }

    if (registry.Retire(result.handle) != ResourceStatus::Success) {
        return Fail("stale decode result fixture retire failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::GenerationMismatch) {
        return Fail("stale decode result request returned wrong status");
    }

    if (registry.Snapshot().registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("stale decode result request changed registered count");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("stale decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch decode result fixture registration failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_AUDIO,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::TypeMismatch) {
        return Fail("type mismatch decode result request returned wrong status");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("type mismatch decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultRejectsNotResidentWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_not_resident");
    if (!result.Succeeded()) {
        return Fail("not resident decode result fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("not resident decode result fixture load commit failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::NotResident) {
        return Fail("not resident decode result request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("not resident decode result request changed residency state");
    }

    return 0;
}

int ResourceDecodeResultRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_failed");
    if (!result.Succeeded()) {
        return Fail("failed load decode result fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("failed load decode result fixture load commit failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::FailedLoad) {
        return Fail("failed load decode result request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load decode result request changed residency state");
    }

    return 0;
}

int ResourceDecodeResultRejectsDuplicateResultId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_duplicate");
    if (!result.Succeeded()) {
        return Fail("duplicate decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("duplicate decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("duplicate decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CommitDecodeResult(request) != ResourceDecodeResultStatus::Success) {
        return Fail("first duplicate decode result commit failed");
    }

    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::DuplicateDecodeResultId) {
        return Fail("duplicate decode result id returned wrong status");
    }

    const ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 1U) {
        return Fail("duplicate decode result id changed active count");
    }

    if (snapshot.duplicate_result_rejected_count != 1U) {
        return Fail("duplicate decode result id was not counted");
    }

    return 0;
}

int ResourceDecodeResultRejectsCapacityOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_capacity");
    if (!result.Succeeded()) {
        return Fail("capacity decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("capacity decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        1U)) {
        return Fail("capacity decode result fixture plan create failed");
    }

    std::uint32_t result_index = 0U;
    while (result_index < MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        const ResourceDecodeResultRequest request = DecodeResultRequest(
            result.handle,
            TYPE_TEXTURE,
            PAYLOAD_ONE,
            DECODE_PLAN_ONE,
            DECODE_RESULT_ONE + result_index,
            ResourceDecodePlanAssetClass::Texture,
            ResourceDecodeResultClass::Texture,
            1U);
        if (registry.CommitDecodeResult(request) != ResourceDecodeResultStatus::Success) {
            return Fail("capacity decode result setup commit failed");
        }

        ++result_index;
    }

    const ResourceDecodeResultRequest overflow_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE + MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        1U);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(overflow_request);
    if (status != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity decode result request returned wrong status");
    }

    const ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        return Fail("capacity decode result request changed active result count");
    }

    if (snapshot.capacity_rejected_result_count != 1U) {
        return Fail("capacity decode result rejection was not counted");
    }

    constexpr std::uint32_t REQUIRED_RESULT_COUNT = MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT + 1U;
    if (snapshot.last_required_result_count != REQUIRED_RESULT_COUNT) {
        return Fail("capacity decode result required count was not reported");
    }

    if (snapshot.last_required_decoded_byte_count != 0U) {
        return Fail("capacity decode result reported decoded byte count");
    }

    const int entry_result = ExpectDecodeResultCapacityEntryMatches(
        snapshot,
        result.handle,
        overflow_request,
        MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        REQUIRED_RESULT_COUNT);
    if (entry_result != 0) {
        return entry_result;
    }

    return 0;
}

int ResourceDecodeResultCapacityEntryClearsOnNonCapacity() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(
        registry,
        TYPE_TEXTURE,
        "texture_decode_result_capacity_entry");
    if (!result.Succeeded()) {
        return Fail("capacity entry decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("capacity entry decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        1U)) {
        return Fail("capacity entry decode result fixture plan create failed");
    }

    std::uint32_t result_index = 0U;
    while (result_index < MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT) {
        const ResourceDecodeResultRequest request = DecodeResultRequest(
            result.handle,
            TYPE_TEXTURE,
            PAYLOAD_ONE,
            DECODE_PLAN_ONE,
            DECODE_RESULT_ONE + result_index,
            ResourceDecodePlanAssetClass::Texture,
            ResourceDecodeResultClass::Texture,
            1U);
        if (registry.CommitDecodeResult(request) != ResourceDecodeResultStatus::Success) {
            return Fail("capacity entry decode result setup commit failed");
        }

        ++result_index;
    }

    const ResourceDecodeResultRequest existing_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        1U);
    const ResourceDecodeResultRequest overflow_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE + MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        1U);
    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not reject overflow");
    }

    ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    int clear_result = ExpectDecodeResultCapacityEntryMatches(
        snapshot,
        result.handle,
        overflow_request,
        MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT,
        MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT + 1U);
    if (clear_result != 0) {
        return clear_result;
    }

    ResourceDecodeResultRecord record{};
    if (registry.QueryDecodeResult(existing_request, &record) != ResourceDecodeResultStatus::Success) {
        return Fail("capacity entry decode result query success failed");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before missing plan");
    }

    const ResourceDecodeResultRequest missing_plan_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_TWO,
        DECODE_RESULT_ONE + MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT + 1U,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        1U);
    if (registry.CommitDecodeResult(missing_plan_request) != ResourceDecodeResultStatus::MissingDecodePlan) {
        return Fail("capacity entry decode result missing plan returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before duplicate");
    }

    if (registry.CommitDecodeResult(existing_request) != ResourceDecodeResultStatus::DuplicateDecodeResultId) {
        return Fail("capacity entry decode result duplicate returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before budget");
    }

    ResourceDecodeResultBudgetDesc budget{};
    budget.decoded_byte_capacity = 1U;
    if (registry.SetDecodeResultBudget(budget) != ResourceDecodeResultStatus::BudgetExceeded) {
        return Fail("capacity entry decode result budget returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before query miss");
    }

    ResourceDecodeResultRecord missing_record{};
    if (registry.QueryDecodeResult(overflow_request, &missing_record) != ResourceDecodeResultStatus::MissingDecodeResult) {
        return Fail("capacity entry decode result query miss returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before release miss");
    }

    if (registry.ReleaseDecodeResult(overflow_request) != ResourceDecodeResultStatus::MissingDecodeResult) {
        return Fail("capacity entry decode result release miss returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    if (registry.CommitDecodeResult(overflow_request) != ResourceDecodeResultStatus::CapacityExceeded) {
        return Fail("capacity entry decode result did not refresh before invalid handle");
    }

    const ResourceDecodeResultRequest invalid_resource_request = DecodeResultRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE + MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT + 2U,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        1U);
    if (registry.CommitDecodeResult(invalid_resource_request) != ResourceDecodeResultStatus::InvalidHandle) {
        return Fail("capacity entry decode result invalid handle returned wrong status");
    }

    snapshot = registry.DecodeResultSnapshot();
    clear_result = ExpectDecodeResultCapacityEntryClear(snapshot);
    if (clear_result != 0) {
        return clear_result;
    }

    return 0;
}

int ResourceDecodeResultRejectsBudgetOverflow() {
    ResourceRegistry registry;
    if (!ConfigureDecodeResultBudget(registry, 1U)) {
        return Fail("budget decode result budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_budget");
    if (!result.Succeeded()) {
        return Fail("budget decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("budget decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("budget decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::BudgetExceeded) {
        return Fail("budget decode result request returned wrong status");
    }

    const ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 0U) {
        return Fail("budget decode result request created a result");
    }

    if (snapshot.budget_rejected_result_count != 1U) {
        return Fail("budget decode result rejection was not counted");
    }

    if (snapshot.last_required_decoded_byte_count != DECODE_PLAN_DECODED_BYTE_COUNT) {
        return Fail("budget decode result required decoded bytes were not reported");
    }

    if (snapshot.last_required_result_count != 0U) {
        return Fail("budget decode result reported result count");
    }

    return 0;
}

int ResourceDecodeResultRejectsAssetClassMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_asset_class");
    if (!result.Succeeded()) {
        return Fail("asset class decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("asset class decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("asset class decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Audio,
        ResourceDecodeResultClass::Audio,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::AssetClassMismatch) {
        return Fail("asset class decode result request returned wrong status");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("asset class decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultRejectsResultClassMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_result_class");
    if (!result.Succeeded()) {
        return Fail("result class decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("result class decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("result class decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Audio,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::ResultClassMismatch) {
        return Fail("result class decode result request returned wrong status");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("result class decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultRejectsDecodedByteCountMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_decoded_bytes");
    if (!result.Succeeded()) {
        return Fail("decoded byte decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("decoded byte decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("decoded byte decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT + 1U);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(request);
    if (status != ResourceDecodeResultStatus::DecodedByteCountMismatch) {
        return Fail("decoded byte decode result request returned wrong status");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("decoded byte decode result request created a result");
    }

    return 0;
}

int ResourceDecodeResultReleasingPlanClearsDependentRecords() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_plan_release");
    if (!result.Succeeded()) {
        return Fail("plan release decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("plan release decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("plan release decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest result_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CommitDecodeResult(result_request) != ResourceDecodeResultStatus::Success) {
        return Fail("plan release decode result commit failed");
    }

    const ResourceDecodePlanRequest plan_request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.ReleaseDecodePlan(plan_request) != ResourceDecodePlanStatus::Success) {
        return Fail("plan release decode result plan release failed");
    }

    const ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 0U) {
        return Fail("plan release left active decode result");
    }

    if (snapshot.committed_decoded_byte_count != 0U) {
        return Fail("plan release left decode result bytes");
    }

    ResourceDecodeResultRecord record;
    const ResourceDecodeResultStatus query_status = registry.QueryDecodeResult(result_request, &record);
    if (query_status != ResourceDecodeResultStatus::MissingDecodeResult) {
        return Fail("plan release decode result query returned wrong status");
    }

    return 0;
}

int ResourceDecodeResultReleasingPayloadClearsDependentRecords() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_payload_release");
    if (!result.Succeeded()) {
        return Fail("payload release decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("payload release decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("payload release decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest result_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CommitDecodeResult(result_request) != ResourceDecodeResultStatus::Success) {
        return Fail("payload release decode result commit failed");
    }

    const ResourceCachePayloadRequest release_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    if (registry.ReleaseCachePayload(release_request) != ResourceCachePayloadStatus::Success) {
        return Fail("payload release decode result payload release failed");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 0U) {
        return Fail("payload release left active decode plan");
    }

    const ResourceDecodeResultSnapshot snapshot = registry.DecodeResultSnapshot();
    if (snapshot.active_result_count != 0U) {
        return Fail("payload release left active decode result");
    }

    if (snapshot.committed_decoded_byte_count != 0U) {
        return Fail("payload release left decode result bytes");
    }

    ResourceDecodeResultRecord record;
    const ResourceDecodeResultStatus query_status = registry.QueryDecodeResult(result_request, &record);
    if (query_status != ResourceDecodeResultStatus::MissingDecodeResult) {
        return Fail("payload release decode result query returned wrong status");
    }

    return 0;
}

int ResourceDecodeResultFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decode_result_validation");
    if (!result.Succeeded()) {
        return Fail("validation decode result fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("validation decode result fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT)) {
        return Fail("validation decode result fixture plan create failed");
    }

    const ResourceDecodeResultRequest valid_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    if (registry.CommitDecodeResult(valid_request) != ResourceDecodeResultStatus::Success) {
        return Fail("validation decode result commit failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceCachePayloadSnapshot before_cache_snapshot = registry.CachePayloadSnapshot();
    const ResourceDecodePlanSnapshot before_plan_snapshot = registry.DecodePlanSnapshot();
    const ResourceDecodeResultSnapshot before_result_snapshot = registry.DecodeResultSnapshot();
    const ResourceDecodeResultRequest invalid_request = DecodeResultRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        DECODE_PLAN_TWO,
        DECODE_RESULT_TWO,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODE_PLAN_DECODED_BYTE_COUNT);
    const ResourceDecodeResultStatus status = registry.CommitDecodeResult(invalid_request);
    if (status != ResourceDecodeResultStatus::InvalidHandle) {
        return Fail("invalid decode result request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid decode result request changed registered count");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid decode result request changed load commit count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid decode result request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid decode result request changed resident bytes");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("invalid decode result request changed cache payload count");
    }

    const ResourceDecodePlanSnapshot after_plan_snapshot = registry.DecodePlanSnapshot();
    if (after_plan_snapshot.active_plan_count != before_plan_snapshot.active_plan_count) {
        return Fail("invalid decode result request changed decode plan count");
    }

    const ResourceDecodeResultSnapshot after_result_snapshot = registry.DecodeResultSnapshot();
    if (after_result_snapshot.active_result_count != before_result_snapshot.active_result_count) {
        return Fail("invalid decode result request changed active result count");
    }

    if (after_result_snapshot.committed_decoded_byte_count != before_result_snapshot.committed_decoded_byte_count) {
        return Fail("invalid decode result request changed decoded bytes");
    }

    ResourceDecodeResultRecord record;
    if (registry.QueryDecodeResult(valid_request, &record) != ResourceDecodeResultStatus::Success) {
        return Fail("valid decode result was not queryable after invalid request");
    }

    if (record.decode_result_id != DECODE_RESULT_ONE) {
        return Fail("invalid decode result request changed existing result");
    }

    return 0;
}

int ResourceDecodedPayloadStoresReadsQueriesAndReleasesBytes() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_success");
    if (!result.Succeeded()) {
        return Fail("decoded payload success fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("decoded payload success fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x10U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        static_cast<std::uint32_t>(bytes.size()));
    if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("decoded payload store failed");
    }

    ResourceDecodedPayloadRecord record;
    if (registry.QueryDecodedPayload(request, &record) != ResourceDecodedPayloadStatus::Success) {
        return Fail("decoded payload query failed");
    }

    if (record.decoded_payload_id != DECODED_PAYLOAD_ONE) {
        return Fail("decoded payload query returned wrong id");
    }

    if (record.decode_result_id != DECODE_RESULT_ONE) {
        return Fail("decoded payload query returned wrong decode result id");
    }

    if (record.decoded_byte_count != DECODED_PAYLOAD_BYTE_COUNT) {
        return Fail("decoded payload query returned wrong byte count");
    }

    std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> output{};
    std::uint32_t output_byte_count = 0U;
    const ResourceDecodedPayloadStatus read_status = registry.ReadDecodedPayload(
        request,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        &output_byte_count);
    if (read_status != ResourceDecodedPayloadStatus::Success) {
        return Fail("decoded payload read failed");
    }

    if (output_byte_count != DECODED_PAYLOAD_BYTE_COUNT) {
        return Fail("decoded payload read returned wrong byte count");
    }

    if (!BytesMatch(output.data(), bytes.data(), output_byte_count)) {
        return Fail("decoded payload read returned wrong bytes");
    }

    ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 1U) {
        return Fail("decoded payload store did not track active count");
    }

    if (snapshot.stored_decoded_byte_count != DECODED_PAYLOAD_BYTE_COUNT) {
        return Fail("decoded payload store did not track decoded bytes");
    }

    if (snapshot.stored_payload_count != 1U) {
        return Fail("decoded payload store was not counted");
    }

    if (snapshot.queried_payload_count != 1U) {
        return Fail("decoded payload query was not counted");
    }

    if (snapshot.read_payload_count != 1U) {
        return Fail("decoded payload read was not counted");
    }

    if (registry.ReleaseDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("decoded payload release failed");
    }

    snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 0U) {
        return Fail("decoded payload release left active payload");
    }

    if (snapshot.stored_decoded_byte_count != 0U) {
        return Fail("decoded payload release left decoded bytes");
    }

    if (snapshot.released_payload_count != 1U) {
        return Fail("decoded payload release was not counted");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 1U) {
        return Fail("decoded payload release changed decode result count");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 1U) {
        return Fail("decoded payload release changed decode plan count");
    }

    if (registry.CachePayloadSnapshot().cached_payload_count != 1U) {
        return Fail("decoded payload release changed cache payload count");
    }

    const ResourceDecodedPayloadStatus query_status = registry.QueryDecodedPayload(request, &record);
    if (query_status != ResourceDecodedPayloadStatus::MissingDecodedPayload) {
        return Fail("released decoded payload query returned wrong status");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsMissingCachePayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_missing_cache");
    if (!result.Succeeded()) {
        return Fail("missing cache decoded payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("missing cache decoded payload fixture residency failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x18U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::MissingCachePayload) {
        return Fail("missing cache decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("missing cache decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsMissingDecodePlan() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_missing_plan");
    if (!result.Succeeded()) {
        return Fail("missing plan decoded payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("missing plan decoded payload fixture residency failed");
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> plan_payload = DecodePlanPayload(
        ResourceDecodePlanAssetClass::Texture,
        RESOURCE_DECODE_PLAN_HEADER_VERSION,
        DECODED_PAYLOAD_BYTE_COUNT);
    if (!StoreDecodePlanPayload(registry, result.handle, TYPE_TEXTURE, PAYLOAD_ONE, plan_payload)) {
        return Fail("missing plan decoded payload cache store failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x19U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::MissingDecodePlan) {
        return Fail("missing plan decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("missing plan decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsMissingDecodeResult() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_missing");
    if (!result.Succeeded()) {
        return Fail("missing result decoded payload fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("missing result decoded payload fixture residency failed");
    }

    if (!CreateDecodePlanMetadata(
        registry,
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("missing result decoded payload plan create failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x20U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::MissingDecodeResult) {
        return Fail("missing decode result decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("missing decode result decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsNullInputBytes() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_null_input");
    if (!result.Succeeded()) {
        return Fail("null input decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("null input decoded payload fixture chain failed");
    }

    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        nullptr,
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::InvalidArgument) {
        return Fail("null input decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("null input decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsEmptyPayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_empty");
    if (!result.Succeeded()) {
        return Fail("empty decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("empty decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x30U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        0U);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::EmptyPayload) {
        return Fail("empty decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("empty decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadReadRejectsOutputBufferTooSmall() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_small_output");
    if (!result.Succeeded()) {
        return Fail("small output decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("small output decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x40U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("small output decoded payload store failed");
    }

    std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> output{};
    std::uint32_t output_byte_count = 99U;
    const ResourceDecodedPayloadStatus status = registry.ReadDecodedPayload(
        request,
        output.data(),
        DECODED_PAYLOAD_BYTE_COUNT - 1U,
        &output_byte_count);
    if (status != ResourceDecodedPayloadStatus::OutputBufferTooSmall) {
        return Fail("small output decoded payload returned wrong status");
    }

    if (output_byte_count != 0U) {
        return Fail("small output decoded payload changed output byte count");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 1U) {
        return Fail("small output decoded payload changed active count");
    }

    if (snapshot.output_buffer_too_small_count != 1U) {
        return Fail("small output decoded payload was not counted");
    }

    return 0;
}

int ResourceDecodedPayloadStoresWindowMetadataAndReferenceBudget() {
    ResourceRegistry registry;
    if (!ConfigureDecodedPayloadBudget(registry, 256U, 1U)) {
        return Fail("window decoded payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_window");
    if (!result.Succeeded()) {
        return Fail("window decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("window decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x48U);
    const std::uint64_t payload_window_byte_offset = 4096U;
    const std::uint64_t payload_window_byte_size = DECODED_PAYLOAD_BYTE_COUNT;
    const ResourceDecodedPayloadRequest request = DecodedPayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT,
        payload_window_byte_offset,
        payload_window_byte_size);
    if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("window decoded payload store failed");
    }

    ResourceDecodedPayloadRecord record;
    if (registry.QueryDecodedPayload(request, &record) != ResourceDecodedPayloadStatus::Success) {
        return Fail("window decoded payload query failed");
    }

    if (record.payload_window_byte_offset != payload_window_byte_offset) {
        return Fail("window decoded payload query returned wrong offset");
    }

    if (record.payload_window_byte_size != payload_window_byte_size) {
        return Fail("window decoded payload query returned wrong size");
    }

    const ResourceDecodedPayloadSnapshot store_snapshot = registry.DecodedPayloadSnapshot();
    if (store_snapshot.budget_payload_reference_capacity != 1U) {
        return Fail("window decoded payload reference budget was not configured");
    }

    std::array<std::uint8_t, 3U> output{};
    std::uint32_t output_byte_count = 0U;
    const ResourceDecodedPayloadRequest read_request = DecodedPayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        nullptr,
        DECODED_PAYLOAD_BYTE_COUNT,
        payload_window_byte_offset + 2U,
        static_cast<std::uint64_t>(output.size()));
    const ResourceDecodedPayloadStatus read_status = registry.ReadDecodedPayload(
        read_request,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        &output_byte_count);
    if (read_status != ResourceDecodedPayloadStatus::Success) {
        return Fail("window decoded payload sub read failed");
    }

    if (output_byte_count != static_cast<std::uint32_t>(output.size())) {
        return Fail("window decoded payload sub read returned wrong byte count");
    }

    if (output[0U] != bytes[2U]) {
        return Fail("window decoded payload sub read returned wrong first byte");
    }

    if (output[2U] != bytes[4U]) {
        return Fail("window decoded payload sub read returned wrong last byte");
    }

    return 0;
}

int ResourceDecodedPayloadStoresU64LogicalWindowWithU32LocalBytes() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_u64_window");
    if (!result.Succeeded()) {
        return Fail("u64 window decoded payload fixture registration failed");
    }

    const std::uint32_t decoded_byte_count = 4U;
    if (!CreateTextureDecodeResultChain(registry, result.handle, decoded_byte_count)) {
        return Fail("u64 window decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, 4U> bytes{{71U, 72U, 73U, 74U}};
    const std::uint64_t payload_window_byte_size = decoded_byte_count;
    ResourceDecodedPayloadRequest store_request = DecodedPayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        decoded_byte_count,
        U64_PAYLOAD_WINDOW_BYTE_OFFSET,
        payload_window_byte_size);
    store_request.payload_logical_byte_count = U64_LOGICAL_PAYLOAD_BYTE_COUNT;
    if (registry.StoreDecodedPayload(store_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("u64 window decoded payload store failed");
    }

    ResourceDecodedPayloadRecord record;
    if (registry.QueryDecodedPayload(store_request, &record) != ResourceDecodedPayloadStatus::Success) {
        return Fail("u64 window decoded payload query failed");
    }

    if (record.payload_logical_byte_count != U64_LOGICAL_PAYLOAD_BYTE_COUNT) {
        return Fail("u64 window decoded payload logical total was not tracked");
    }

    if (record.payload_window_byte_offset != U64_PAYLOAD_WINDOW_BYTE_OFFSET) {
        return Fail("u64 window decoded payload offset was not tracked");
    }

    if (record.payload_window_byte_size != payload_window_byte_size) {
        return Fail("u64 window decoded payload size was not tracked");
    }

    const ResourceDecodedPayloadSnapshot store_snapshot = registry.DecodedPayloadSnapshot();
    if (store_snapshot.last_payload_logical_byte_count != U64_LOGICAL_PAYLOAD_BYTE_COUNT) {
        return Fail("u64 window decoded payload snapshot logical total was not tracked");
    }

    std::array<std::uint8_t, 2U> output{};
    std::uint32_t output_byte_count = 0U;
    const std::uint64_t read_window_byte_offset = U64_PAYLOAD_WINDOW_BYTE_OFFSET + 1U;
    const std::uint64_t read_window_byte_size = static_cast<std::uint64_t>(output.size());
    ResourceDecodedPayloadRequest read_request = DecodedPayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        nullptr,
        decoded_byte_count,
        read_window_byte_offset,
        read_window_byte_size);
    read_request.payload_logical_byte_count = U64_LOGICAL_PAYLOAD_BYTE_COUNT;
    const std::uint32_t output_byte_capacity = static_cast<std::uint32_t>(output.size());
    const ResourceDecodedPayloadStatus read_status = registry.ReadDecodedPayload(
        read_request,
        output.data(),
        output_byte_capacity,
        &output_byte_count);
    if (read_status != ResourceDecodedPayloadStatus::Success) {
        return Fail("u64 window decoded payload read failed");
    }

    if (output_byte_count != output_byte_capacity) {
        return Fail("u64 window decoded payload read returned wrong byte count");
    }

    if (output[0U] != bytes[1U]) {
        return Fail("u64 window decoded payload read returned wrong first byte");
    }

    if (output[1U] != bytes[2U]) {
        return Fail("u64 window decoded payload read returned wrong second byte");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsWindowMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_window_mismatch");
    if (!result.Succeeded()) {
        return Fail("window mismatch decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("window mismatch decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x58U);
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceDecodedPayloadSnapshot before_decoded_snapshot = registry.DecodedPayloadSnapshot();
    const ResourceDecodedPayloadRequest request = DecodedPayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT,
        8192U,
        DECODED_PAYLOAD_BYTE_COUNT - 1U);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::PayloadWindowOutOfBounds) {
        return Fail("window mismatch decoded payload returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("window mismatch decoded payload changed registered count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("window mismatch decoded payload changed resident count");
    }

    const ResourceDecodedPayloadSnapshot after_decoded_snapshot = registry.DecodedPayloadSnapshot();
    if (after_decoded_snapshot.active_payload_count != before_decoded_snapshot.active_payload_count) {
        return Fail("window mismatch decoded payload changed active count");
    }

    if (after_decoded_snapshot.stored_decoded_byte_count != before_decoded_snapshot.stored_decoded_byte_count) {
        return Fail("window mismatch decoded payload changed decoded bytes");
    }

    if (after_decoded_snapshot.payload_window_rejected_count != 1U) {
        return Fail("window mismatch decoded payload was not counted");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsReferenceBudgetWithoutMutation() {
    ResourceRegistry registry;
    if (!ConfigureDecodedPayloadBudget(registry, 256U, 1U)) {
        return Fail("reference budget decoded payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_reference_budget");
    if (!result.Succeeded()) {
        return Fail("reference budget decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("reference budget decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> first_bytes = DecodedPayloadBytes(0x68U);
    const ResourceDecodedPayloadRequest first_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        first_bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(first_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("reference budget first decoded payload store failed");
    }

    const ResourceDecodedPayloadSnapshot before_snapshot = registry.DecodedPayloadSnapshot();
    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> second_bytes = DecodedPayloadBytes(0x70U);
    const ResourceDecodedPayloadRequest second_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_TWO,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        second_bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(second_request);
    if (status != ResourceDecodedPayloadStatus::ReferenceBudgetExceeded) {
        return Fail("reference budget decoded payload returned wrong status");
    }

    const ResourceDecodedPayloadSnapshot after_snapshot = registry.DecodedPayloadSnapshot();
    if (after_snapshot.active_payload_count != before_snapshot.active_payload_count) {
        return Fail("reference budget decoded payload changed active count");
    }

    if (after_snapshot.stored_decoded_byte_count != before_snapshot.stored_decoded_byte_count) {
        return Fail("reference budget decoded payload changed stored bytes");
    }

    if (after_snapshot.reference_budget_rejected_payload_count != 1U) {
        return Fail("reference budget decoded payload was not counted");
    }

    const std::uint32_t required_payload_reference_count = before_snapshot.active_payload_count + 1U;
    if (after_snapshot.last_required_payload_reference_count != required_payload_reference_count) {
        return Fail("reference budget decoded payload required references were not reported");
    }

    if (after_snapshot.last_required_decoded_byte_count != 0U) {
        return Fail("reference budget decoded payload reported decoded bytes");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsDuplicatePayloadId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_duplicate");
    if (!result.Succeeded()) {
        return Fail("duplicate decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("duplicate decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x50U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("first duplicate decoded payload store failed");
    }

    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::DuplicateDecodedPayloadId) {
        return Fail("duplicate decoded payload id returned wrong status");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 1U) {
        return Fail("duplicate decoded payload changed active count");
    }

    if (snapshot.duplicate_payload_rejected_count != 1U) {
        return Fail("duplicate decoded payload was not counted");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsCapacityOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_capacity");
    if (!result.Succeeded()) {
        return Fail("capacity decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, 1U)) {
        return Fail("capacity decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x60U);
    std::uint32_t payload_index = 0U;
    while (payload_index < MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
            result.handle,
            TYPE_TEXTURE,
            PAYLOAD_ONE,
            DECODE_PLAN_ONE,
            DECODE_RESULT_ONE,
            DECODED_PAYLOAD_ONE + payload_index,
            ResourceDecodePlanAssetClass::Texture,
            ResourceDecodeResultClass::Texture,
            bytes.data(),
            1U);
        if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
            return Fail("capacity decoded payload setup store failed");
        }

        ++payload_index;
    }

    const ResourceDecodedPayloadRequest overflow_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE + MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        1U);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(overflow_request);
    if (status != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity decoded payload returned wrong status");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        return Fail("capacity decoded payload changed active count");
    }

    if (snapshot.capacity_rejected_payload_count != 1U) {
        return Fail("capacity decoded payload was not counted");
    }

    constexpr std::uint32_t REQUIRED_PAYLOAD_REFERENCE_COUNT = MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT + 1U;
    if (snapshot.last_required_payload_reference_count != REQUIRED_PAYLOAD_REFERENCE_COUNT) {
        return Fail("capacity decoded payload required reference count was not reported");
    }

    if (snapshot.last_required_decoded_byte_count != 0U) {
        return Fail("capacity decoded payload reported decoded bytes");
    }

    return 0;
}

int ResourceDecodedPayloadCapacityEntryRecordsRejectedIdentity() {
    ResourceRegistry registry;
    const char *logical_key_text = "texture_decoded_payload_capacity_entry";
    const ResourceLogicalKey logical_key(logical_key_text);
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, logical_key_text);
    if (!result.Succeeded()) {
        return Fail("capacity entry decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, 1U)) {
        return Fail("capacity entry decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x62U);
    std::uint32_t payload_index = 0U;
    while (payload_index < MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT) {
        const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
            result.handle,
            TYPE_TEXTURE,
            PAYLOAD_ONE,
            DECODE_PLAN_ONE,
            DECODE_RESULT_ONE,
            DECODED_PAYLOAD_ONE + payload_index,
            ResourceDecodePlanAssetClass::Texture,
            ResourceDecodeResultClass::Texture,
            bytes.data(),
            1U);
        if (registry.StoreDecodedPayload(request) != ResourceDecodedPayloadStatus::Success) {
            return Fail("capacity entry decoded payload setup store failed");
        }

        ++payload_index;
    }

    const ResourceDecodedPayloadRequest overflow_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE + MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        1U);
    const ResourceDecodedPayloadStatus capacity_status = registry.StoreDecodedPayload(overflow_request);
    if (capacity_status != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity entry decoded payload returned wrong status");
    }

    const ResourceDecodedPayloadSnapshot capacity_snapshot = registry.DecodedPayloadSnapshot();
    constexpr std::uint32_t REQUIRED_PAYLOAD_REFERENCE_COUNT = MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT + 1U;
    const int capacity_entry_status = ExpectDecodedPayloadCapacityEntryMatches(
        capacity_snapshot,
        result.handle,
        logical_key,
        overflow_request,
        0U,
        REQUIRED_PAYLOAD_REFERENCE_COUNT,
        MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    const ResourceDecodedPayloadRequest duplicate_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        1U);
    const ResourceDecodedPayloadStatus duplicate_status = registry.StoreDecodedPayload(duplicate_request);
    if (duplicate_status != ResourceDecodedPayloadStatus::DuplicateDecodedPayloadId) {
        return Fail("capacity entry duplicate decoded payload returned wrong status");
    }

    ResourceDecodedPayloadSnapshot cleared_snapshot = registry.DecodedPayloadSnapshot();
    int cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    if (registry.StoreDecodedPayload(overflow_request) != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity entry decoded payload second overflow failed");
    }

    ResourceDecodedPayloadBudgetDesc budget;
    budget.decoded_byte_capacity = 1U;
    budget.payload_reference_capacity = MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT;
    const ResourceDecodedPayloadStatus budget_status = registry.SetDecodedPayloadBudget(budget);
    if (budget_status != ResourceDecodedPayloadStatus::BudgetExceeded) {
        return Fail("capacity entry configure budget returned wrong status");
    }

    cleared_snapshot = registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    if (registry.StoreDecodedPayload(overflow_request) != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity entry decoded payload third overflow failed");
    }

    const ResourceDecodedPayloadRequest invalid_payload_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        0U,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE + MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT + 1U,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        1U);
    const ResourceDecodedPayloadStatus invalid_payload_status = registry.StoreDecodedPayload(invalid_payload_request);
    if (invalid_payload_status != ResourceDecodedPayloadStatus::InvalidPayloadId) {
        return Fail("capacity entry invalid payload returned wrong status");
    }

    cleared_snapshot = registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    if (registry.StoreDecodedPayload(overflow_request) != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity entry decoded payload fourth overflow failed");
    }

    const ResourceDecodedPayloadRequest missing_resource_request = DecodedPayloadRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE + MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT + 2U,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        1U);
    const ResourceDecodedPayloadStatus missing_resource_status = registry.StoreDecodedPayload(missing_resource_request);
    if (missing_resource_status != ResourceDecodedPayloadStatus::InvalidHandle) {
        return Fail("capacity entry missing resource returned wrong status");
    }

    cleared_snapshot = registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    if (registry.StoreDecodedPayload(overflow_request) != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("capacity entry decoded payload fifth overflow failed");
    }

    if (registry.ReleaseDecodedPayload(duplicate_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("capacity entry decoded payload release failed");
    }

    cleared_snapshot = registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    if (registry.StoreDecodedPayload(overflow_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("capacity entry decoded payload success retry failed");
    }

    cleared_snapshot = registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    return 0;
}

int ResourceDecodedPayloadCapacityEntryRecordsAllCapacityLimits() {
    ResourceRegistry byte_registry;
    const char *byte_key_text = "texture_decoded_payload_entry_byte";
    const ResourceLogicalKey byte_key(byte_key_text);
    const ResourceRegistrationResult byte_result = Register(byte_registry, TYPE_TEXTURE, byte_key_text);
    if (!byte_result.Succeeded()) {
        return Fail("byte capacity entry decoded payload registration failed");
    }

    if (!AdmitUploadedResident(byte_registry, byte_result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("byte capacity entry decoded payload residency failed");
    }

    std::array<std::uint8_t, MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD + 1U> oversized_bytes{};
    oversized_bytes[0U] = 0x72U;
    constexpr std::uint32_t OVERSIZED_BYTE_COUNT = MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD + 1U;
    ResourceDecodedPayloadRequest oversized_request = DecodedPayloadRequest(
        byte_result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        oversized_bytes.data(),
        OVERSIZED_BYTE_COUNT);
    oversized_request.payload_logical_byte_count = OVERSIZED_BYTE_COUNT + 16U;
    oversized_request.payload_window_byte_offset = 8U;
    oversized_request.payload_window_byte_size = OVERSIZED_BYTE_COUNT;
    const ResourceDecodedPayloadStatus oversized_status = byte_registry.StoreDecodedPayload(oversized_request);
    if (oversized_status != ResourceDecodedPayloadStatus::CapacityExceeded) {
        return Fail("byte capacity entry decoded payload returned wrong status");
    }

    ResourceDecodedPayloadSnapshot capacity_snapshot = byte_registry.DecodedPayloadSnapshot();
    int capacity_entry_status = ExpectDecodedPayloadCapacityEntryMatches(
        capacity_snapshot,
        byte_result.handle,
        byte_key,
        oversized_request,
        OVERSIZED_BYTE_COUNT,
        0U,
        MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        0U);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    ResourceDecodedPayloadRequest null_bytes_request = oversized_request;
    null_bytes_request.decoded_bytes = nullptr;
    const ResourceDecodedPayloadStatus null_bytes_status = byte_registry.StoreDecodedPayload(null_bytes_request);
    if (null_bytes_status != ResourceDecodedPayloadStatus::InvalidArgument) {
        return Fail("byte capacity entry null bytes returned wrong status");
    }

    ResourceDecodedPayloadSnapshot cleared_snapshot = byte_registry.DecodedPayloadSnapshot();
    int cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    ResourceRegistry budget_registry;
    if (!ConfigureDecodedPayloadBudget(budget_registry, 1U)) {
        return Fail("budget capacity entry decoded payload budget failed");
    }

    const char *budget_key_text = "texture_decoded_payload_entry_budget";
    const ResourceLogicalKey budget_key(budget_key_text);
    const ResourceRegistrationResult budget_result = Register(budget_registry, TYPE_TEXTURE, budget_key_text);
    if (!budget_result.Succeeded()) {
        return Fail("budget capacity entry decoded payload registration failed");
    }

    if (!CreateTextureDecodeResultChain(budget_registry, budget_result.handle, 2U)) {
        return Fail("budget capacity entry decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> budget_bytes = DecodedPayloadBytes(0x82U);
    const ResourceDecodedPayloadRequest budget_request = DecodedPayloadRequest(
        budget_result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        budget_bytes.data(),
        2U);
    const ResourceDecodedPayloadStatus budget_status = budget_registry.StoreDecodedPayload(budget_request);
    if (budget_status != ResourceDecodedPayloadStatus::BudgetExceeded) {
        return Fail("budget capacity entry decoded payload returned wrong status");
    }

    capacity_snapshot = budget_registry.DecodedPayloadSnapshot();
    capacity_entry_status = ExpectDecodedPayloadCapacityEntryMatches(
        capacity_snapshot,
        budget_result.handle,
        budget_key,
        budget_request,
        2U,
        0U,
        1U,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        0U);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    ResourceDecodedPayloadRecord missing_record{};
    const ResourceDecodedPayloadStatus missing_payload_status =
        budget_registry.QueryDecodedPayload(budget_request, &missing_record);
    if (missing_payload_status != ResourceDecodedPayloadStatus::MissingDecodedPayload) {
        return Fail("budget capacity entry missing payload returned wrong status");
    }

    cleared_snapshot = budget_registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    ResourceRegistry reference_registry;
    if (!ConfigureDecodedPayloadBudget(reference_registry, MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES, 1U)) {
        return Fail("reference capacity entry decoded payload budget failed");
    }

    const char *reference_key_text = "texture_decoded_payload_entry_reference";
    const ResourceLogicalKey reference_key(reference_key_text);
    const ResourceRegistrationResult reference_result = Register(reference_registry, TYPE_TEXTURE, reference_key_text);
    if (!reference_result.Succeeded()) {
        return Fail("reference capacity entry decoded payload registration failed");
    }

    if (!CreateTextureDecodeResultChain(reference_registry, reference_result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("reference capacity entry decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> first_bytes = DecodedPayloadBytes(0x92U);
    const ResourceDecodedPayloadRequest first_request = DecodedPayloadRequest(
        reference_result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        first_bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (reference_registry.StoreDecodedPayload(first_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("reference capacity entry first decoded payload store failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> second_bytes = DecodedPayloadBytes(0xA2U);
    const ResourceDecodedPayloadRequest second_request = DecodedPayloadRequest(
        reference_result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_TWO,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        second_bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus reference_status = reference_registry.StoreDecodedPayload(second_request);
    if (reference_status != ResourceDecodedPayloadStatus::ReferenceBudgetExceeded) {
        return Fail("reference capacity entry decoded payload returned wrong status");
    }

    capacity_snapshot = reference_registry.DecodedPayloadSnapshot();
    capacity_entry_status = ExpectDecodedPayloadCapacityEntryMatches(
        capacity_snapshot,
        reference_result.handle,
        reference_key,
        second_request,
        0U,
        2U,
        MAX_RESOURCE_DECODED_PAYLOAD_TOTAL_BYTES,
        1U,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT,
        1U);
    if (capacity_entry_status != 0) {
        return capacity_entry_status;
    }

    ResourceDecodedPayloadRecord existing_record{};
    const ResourceDecodedPayloadStatus existing_query_status =
        reference_registry.QueryDecodedPayload(first_request, &existing_record);
    if (existing_query_status != ResourceDecodedPayloadStatus::Success) {
        return Fail("reference capacity entry query clear failed");
    }

    cleared_snapshot = reference_registry.DecodedPayloadSnapshot();
    cleared_status = ExpectDecodedPayloadCapacityEntryCleared(cleared_snapshot);
    if (cleared_status != 0) {
        return cleared_status;
    }

    return 0;
}

int ResourceDecodedPayloadRejectsBudgetOverflow() {
    ResourceRegistry registry;
    if (!ConfigureDecodedPayloadBudget(registry, 1U)) {
        return Fail("budget decoded payload budget configuration failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_budget");
    if (!result.Succeeded()) {
        return Fail("budget decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, 2U)) {
        return Fail("budget decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x70U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        2U);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::BudgetExceeded) {
        return Fail("budget decoded payload returned wrong status");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 0U) {
        return Fail("budget decoded payload created a payload");
    }

    if (snapshot.budget_rejected_payload_count != 1U) {
        return Fail("budget decoded payload was not counted");
    }

    if (snapshot.last_required_decoded_byte_count != 2U) {
        return Fail("budget decoded payload required decoded bytes were not reported");
    }

    if (snapshot.last_required_payload_reference_count != 0U) {
        return Fail("budget decoded payload reported reference count");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsAssetClassMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_asset");
    if (!result.Succeeded()) {
        return Fail("asset mismatch decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("asset mismatch decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x80U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Audio,
        ResourceDecodeResultClass::Audio,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::AssetClassMismatch) {
        return Fail("asset mismatch decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("asset mismatch decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsResultClassMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_result_class");
    if (!result.Succeeded()) {
        return Fail("result mismatch decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("result mismatch decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x90U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Audio,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::ResultClassMismatch) {
        return Fail("result mismatch decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("result mismatch decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsDecodedByteCountMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_byte_count");
    if (!result.Succeeded()) {
        return Fail("byte mismatch decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("byte mismatch decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xA0U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT - 1U);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::DecodedByteCountMismatch) {
        return Fail("byte mismatch decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("byte mismatch decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch decoded payload fixture registration failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xB0U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_AUDIO,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::TypeMismatch) {
        return Fail("type mismatch decoded payload returned wrong status");
    }

    if (registry.DecodedPayloadSnapshot().active_payload_count != 0U) {
        return Fail("type mismatch decoded payload created a payload");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsNotResidentWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_not_resident");
    if (!result.Succeeded()) {
        return Fail("not resident decoded payload fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("not resident decoded payload load commit failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xC0U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::NotResident) {
        return Fail("not resident decoded payload returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("not resident decoded payload changed residency state");
    }

    return 0;
}

int ResourceDecodedPayloadRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_failed");
    if (!result.Succeeded()) {
        return Fail("failed load decoded payload fixture registration failed");
    }

    if (!CommitLoad(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("failed load decoded payload load commit failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xD0U);
    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(request);
    if (status != ResourceDecodedPayloadStatus::FailedLoad) {
        return Fail("failed load decoded payload returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load decoded payload changed residency state");
    }

    return 0;
}

int ResourceDecodedPayloadReleasingResultClearsPayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_result_release");
    if (!result.Succeeded()) {
        return Fail("result release decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("result release decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xE0U);
    const ResourceDecodedPayloadRequest decoded_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(decoded_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("result release decoded payload store failed");
    }

    const ResourceDecodeResultRequest result_request = DecodeResultRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.ReleaseDecodeResult(result_request) != ResourceDecodeResultStatus::Success) {
        return Fail("result release decoded payload result release failed");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 0U) {
        return Fail("result release left active decoded payload");
    }

    if (snapshot.stored_decoded_byte_count != 0U) {
        return Fail("result release left decoded payload bytes");
    }

    if (snapshot.dependent_cleared_payload_count != 1U) {
        return Fail("result release dependent clear was not counted");
    }

    ResourceDecodedPayloadRecord record;
    const ResourceDecodedPayloadStatus query_status = registry.QueryDecodedPayload(decoded_request, &record);
    if (query_status != ResourceDecodedPayloadStatus::MissingDecodedPayload) {
        return Fail("result release decoded payload query returned wrong status");
    }

    return 0;
}

int ResourceDecodedPayloadReleasingPlanClearsPayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_plan_release");
    if (!result.Succeeded()) {
        return Fail("plan release decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("plan release decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0xF0U);
    const ResourceDecodedPayloadRequest decoded_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(decoded_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("plan release decoded payload store failed");
    }

    const ResourceDecodePlanRequest plan_request = DecodePlanRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        ResourceDecodePlanAssetClass::Texture,
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.ReleaseDecodePlan(plan_request) != ResourceDecodePlanStatus::Success) {
        return Fail("plan release decoded payload plan release failed");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("plan release left active decode result");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 0U) {
        return Fail("plan release left active decoded payload");
    }

    if (snapshot.stored_decoded_byte_count != 0U) {
        return Fail("plan release left decoded payload bytes");
    }

    if (snapshot.dependent_cleared_payload_count != 1U) {
        return Fail("plan release dependent clear was not counted");
    }

    return 0;
}

int ResourceDecodedPayloadReleasingCachePayloadClearsPayload() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_cache_release");
    if (!result.Succeeded()) {
        return Fail("cache release decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("cache release decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x21U);
    const ResourceDecodedPayloadRequest decoded_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(decoded_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("cache release decoded payload store failed");
    }

    const ResourceCachePayloadRequest cache_request = CachePayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U);
    if (registry.ReleaseCachePayload(cache_request) != ResourceCachePayloadStatus::Success) {
        return Fail("cache release decoded payload cache release failed");
    }

    if (registry.DecodePlanSnapshot().active_plan_count != 0U) {
        return Fail("cache release left active decode plan");
    }

    if (registry.DecodeResultSnapshot().active_result_count != 0U) {
        return Fail("cache release left active decode result");
    }

    const ResourceDecodedPayloadSnapshot snapshot = registry.DecodedPayloadSnapshot();
    if (snapshot.active_payload_count != 0U) {
        return Fail("cache release left active decoded payload");
    }

    if (snapshot.stored_decoded_byte_count != 0U) {
        return Fail("cache release left decoded payload bytes");
    }

    if (snapshot.dependent_cleared_payload_count != 1U) {
        return Fail("cache release dependent clear was not counted");
    }

    return 0;
}

int ResourceDecodedPayloadFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_decoded_payload_validation");
    if (!result.Succeeded()) {
        return Fail("validation decoded payload fixture registration failed");
    }

    if (!CreateTextureDecodeResultChain(registry, result.handle, DECODED_PAYLOAD_BYTE_COUNT)) {
        return Fail("validation decoded payload fixture chain failed");
    }

    const std::array<std::uint8_t, DECODED_PAYLOAD_BYTE_COUNT> bytes = DecodedPayloadBytes(0x31U);
    const ResourceDecodedPayloadRequest valid_request = DecodedPayloadRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        DECODE_PLAN_ONE,
        DECODE_RESULT_ONE,
        DECODED_PAYLOAD_ONE,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    if (registry.StoreDecodedPayload(valid_request) != ResourceDecodedPayloadStatus::Success) {
        return Fail("validation decoded payload store failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceCachePayloadSnapshot before_cache_snapshot = registry.CachePayloadSnapshot();
    const ResourceDecodePlanSnapshot before_plan_snapshot = registry.DecodePlanSnapshot();
    const ResourceDecodeResultSnapshot before_result_snapshot = registry.DecodeResultSnapshot();
    const ResourceDecodedPayloadSnapshot before_payload_snapshot = registry.DecodedPayloadSnapshot();
    const ResourceDecodedPayloadRequest invalid_request = DecodedPayloadRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        PAYLOAD_TWO,
        DECODE_PLAN_TWO,
        DECODE_RESULT_TWO,
        DECODED_PAYLOAD_TWO,
        ResourceDecodePlanAssetClass::Texture,
        ResourceDecodeResultClass::Texture,
        bytes.data(),
        DECODED_PAYLOAD_BYTE_COUNT);
    const ResourceDecodedPayloadStatus status = registry.StoreDecodedPayload(invalid_request);
    if (status != ResourceDecodedPayloadStatus::InvalidHandle) {
        return Fail("invalid decoded payload request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid decoded payload request changed registered count");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid decoded payload request changed load commit count");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid decoded payload request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid decoded payload request changed resident bytes");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("invalid decoded payload request changed cache payload count");
    }

    const ResourceDecodePlanSnapshot after_plan_snapshot = registry.DecodePlanSnapshot();
    if (after_plan_snapshot.active_plan_count != before_plan_snapshot.active_plan_count) {
        return Fail("invalid decoded payload request changed decode plan count");
    }

    const ResourceDecodeResultSnapshot after_result_snapshot = registry.DecodeResultSnapshot();
    if (after_result_snapshot.active_result_count != before_result_snapshot.active_result_count) {
        return Fail("invalid decoded payload request changed decode result count");
    }

    const ResourceDecodedPayloadSnapshot after_payload_snapshot = registry.DecodedPayloadSnapshot();
    if (after_payload_snapshot.active_payload_count != before_payload_snapshot.active_payload_count) {
        return Fail("invalid decoded payload request changed active decoded payload count");
    }

    if (after_payload_snapshot.stored_decoded_byte_count != before_payload_snapshot.stored_decoded_byte_count) {
        return Fail("invalid decoded payload request changed stored decoded bytes");
    }

    ResourceDecodedPayloadRecord record;
    if (registry.QueryDecodedPayload(valid_request, &record) != ResourceDecodedPayloadStatus::Success) {
        return Fail("valid decoded payload was not queryable after invalid request");
    }

    if (record.decoded_payload_id != DECODED_PAYLOAD_ONE) {
        return Fail("invalid decoded payload request changed existing payload");
    }

    return 0;
}

int ResourcePayloadWindowDoesNotChangeResourceReferenceCountOrResidency() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_payload_window_stability");
    if (!result.Succeeded()) {
        return Fail("payload window stability fixture registration failed");
    }

    if (!AdmitUploadedResident(registry, result.handle, TYPE_TEXTURE, COMMIT_ONE, UPLOAD_ONE)) {
        return Fail("payload window stability fixture residency failed");
    }

    const std::array<std::uint8_t, 4U> payload{{57U, 58U, 59U, 60U}};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    const std::uint64_t payload_window_byte_offset = 16384U;
    const ResourceCachePayloadRequest store_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        payload.data(),
        payload_byte_count,
        payload_window_byte_offset,
        payload_byte_count);
    if (registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("payload window stability cache store failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    std::array<std::uint8_t, 2U> output{};
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest read_request = CachePayloadWindowRequest(
        result.handle,
        TYPE_TEXTURE,
        PAYLOAD_ONE,
        nullptr,
        0U,
        payload_window_byte_offset + 1U,
        static_cast<std::uint64_t>(output.size()));
    if (registry.ReadCachePayload(
        read_request,
        output.data(),
        static_cast<std::uint32_t>(output.size()),
        &output_byte_count) != ResourceCachePayloadStatus::Success) {
        return Fail("payload window stability read failed");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.acquired_handle_count != before_resource_snapshot.acquired_handle_count) {
        return Fail("payload window read acquired a resource handle");
    }

    if (after_resource_snapshot.released_handle_count != before_resource_snapshot.released_handle_count) {
        return Fail("payload window read released a resource handle");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("payload window read changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("payload window read changed resident bytes");
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
        {TEST_DESCRIPTOR_BATCH, ResourceDescriptorBatchRegistrationSubmitsRowsAndStopsOnFirstFailure},
        {TEST_DESCRIPTOR_ENUMERATION, ResourceDescriptorEnumerationReportsRegisteredSyntheticDescriptors},
        {TEST_DESCRIPTOR_EXACT_LOOKUP, ResourceDescriptorExactLookupFindsRegisteredSyntheticDescriptor},
        {TEST_DESCRIPTOR_BATCH_EXACT_LOOKUP, ResourceDescriptorBatchExactLookupReturnsAtomicRows},
        {TEST_INVALID_DESCRIPTOR, ResourceRegisterRejectsInvalidDescriptorWithoutMutation},
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
        {TEST_DEPENDENCY_BATCH, ResourceDependencyBatchSubmitsRowsAndStopsOnFirstFailure},
        {TEST_DEPENDENCY_EDGE_EXACT_LOOKUP, ResourceDependencyEdgeExactLookupFindsDirectEdge},
        {TEST_DEPENDENCY_EDGE_COUNT, ResourceDependencyEdgeCountSnapshotMatchesDirectEdges},
        {TEST_DEPENDENCY_EDGE_ENUMERATION, ResourceDependencyEdgeEnumerationReportsCommittedRows},
        {TEST_DEPENDENCY_TRAVERSAL, ResourceDependencyTraversalReturnsExplicitClosureHandles},
        {TEST_DEPENDENCY_MULTIROOT_TRAVERSAL, ResourceDependencyTraversalMultiRootDeduplicatesClosureHandles},
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
        {TEST_LOAD_COMMIT_CAPACITY_ENTRY, ResourceLoadCommitCapacityEntryRecordsRejectedIdentity},
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
        {TEST_CACHE_PAYLOAD_WINDOW_REFERENCE, ResourceCachePayloadStoresWindowMetadataAndReferenceBudget},
        {TEST_CACHE_PAYLOAD_U64_LOGICAL_WINDOW, ResourceCachePayloadStoresU64LogicalWindowWithU32LocalBytes},
        {TEST_CACHE_PAYLOAD_WINDOW_OVERFLOW, ResourceCachePayloadRejectsWindowOverflowWithoutMutation},
        {TEST_CACHE_PAYLOAD_REFERENCE_BUDGET, ResourceCachePayloadRejectsReferenceBudgetWithoutMutation},
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
        {TEST_DECODE_PLAN_FAILED_VALIDATION, ResourceDecodePlanFailedValidationDoesNotMutateResourceState},
        {TEST_DECODE_RESULT_COMMIT_QUERY_RELEASE, ResourceDecodeResultCommitsQueriesAndReleasesMetadata},
        {TEST_DECODE_RESULT_MISSING_PLAN, ResourceDecodeResultRejectsMissingDecodePlan},
        {TEST_DECODE_RESULT_STALE_HANDLE, ResourceDecodeResultRejectsStaleHandleWithoutMutation},
        {TEST_DECODE_RESULT_TYPE_MISMATCH, ResourceDecodeResultRejectsTypeMismatchWithoutMutation},
        {TEST_DECODE_RESULT_NOT_RESIDENT, ResourceDecodeResultRejectsNotResidentWithoutMutation},
        {TEST_DECODE_RESULT_FAILED_LOAD, ResourceDecodeResultRejectsFailedLoadWithoutMutation},
        {TEST_DECODE_RESULT_DUPLICATE, ResourceDecodeResultRejectsDuplicateResultId},
        {TEST_DECODE_RESULT_CAPACITY, ResourceDecodeResultRejectsCapacityOverflow},
        {TEST_DECODE_RESULT_CAPACITY_ENTRY, ResourceDecodeResultCapacityEntryClearsOnNonCapacity},
        {TEST_DECODE_RESULT_BUDGET, ResourceDecodeResultRejectsBudgetOverflow},
        {TEST_DECODE_RESULT_ASSET_CLASS, ResourceDecodeResultRejectsAssetClassMismatch},
        {TEST_DECODE_RESULT_RESULT_CLASS, ResourceDecodeResultRejectsResultClassMismatch},
        {TEST_DECODE_RESULT_DECODED_BYTES, ResourceDecodeResultRejectsDecodedByteCountMismatch},
        {TEST_DECODE_RESULT_PLAN_RELEASE, ResourceDecodeResultReleasingPlanClearsDependentRecords},
        {TEST_DECODE_RESULT_PAYLOAD_RELEASE, ResourceDecodeResultReleasingPayloadClearsDependentRecords},
        {TEST_DECODE_RESULT_FAILED_VALIDATION, ResourceDecodeResultFailedValidationDoesNotMutateResourceState},
        {TEST_DECODED_PAYLOAD_STORE_READ, ResourceDecodedPayloadStoresReadsQueriesAndReleasesBytes},
        {TEST_DECODED_PAYLOAD_MISSING_CACHE, ResourceDecodedPayloadRejectsMissingCachePayload},
        {TEST_DECODED_PAYLOAD_MISSING_PLAN, ResourceDecodedPayloadRejectsMissingDecodePlan},
        {TEST_DECODED_PAYLOAD_MISSING_RESULT, ResourceDecodedPayloadRejectsMissingDecodeResult},
        {TEST_DECODED_PAYLOAD_NULL_INPUT, ResourceDecodedPayloadRejectsNullInputBytes},
        {TEST_DECODED_PAYLOAD_EMPTY, ResourceDecodedPayloadRejectsEmptyPayload},
        {TEST_DECODED_PAYLOAD_OUTPUT_SMALL, ResourceDecodedPayloadReadRejectsOutputBufferTooSmall},
        {TEST_DECODED_PAYLOAD_WINDOW_REFERENCE, ResourceDecodedPayloadStoresWindowMetadataAndReferenceBudget},
        {TEST_DECODED_PAYLOAD_U64_LOGICAL_WINDOW, ResourceDecodedPayloadStoresU64LogicalWindowWithU32LocalBytes},
        {TEST_DECODED_PAYLOAD_WINDOW_MISMATCH, ResourceDecodedPayloadRejectsWindowMismatchWithoutMutation},
        {TEST_DECODED_PAYLOAD_REFERENCE_BUDGET, ResourceDecodedPayloadRejectsReferenceBudgetWithoutMutation},
        {TEST_DECODED_PAYLOAD_DUPLICATE, ResourceDecodedPayloadRejectsDuplicatePayloadId},
        {TEST_DECODED_PAYLOAD_CAPACITY, ResourceDecodedPayloadRejectsCapacityOverflow},
        {TEST_DECODED_PAYLOAD_CAPACITY_ENTRY, ResourceDecodedPayloadCapacityEntryRecordsRejectedIdentity},
        {TEST_DECODED_PAYLOAD_CAPACITY_ENTRY_LIMITS, ResourceDecodedPayloadCapacityEntryRecordsAllCapacityLimits},
        {TEST_DECODED_PAYLOAD_BUDGET, ResourceDecodedPayloadRejectsBudgetOverflow},
        {TEST_DECODED_PAYLOAD_ASSET_CLASS, ResourceDecodedPayloadRejectsAssetClassMismatch},
        {TEST_DECODED_PAYLOAD_RESULT_CLASS, ResourceDecodedPayloadRejectsResultClassMismatch},
        {TEST_DECODED_PAYLOAD_DECODED_BYTES, ResourceDecodedPayloadRejectsDecodedByteCountMismatch},
        {TEST_DECODED_PAYLOAD_TYPE_MISMATCH, ResourceDecodedPayloadRejectsTypeMismatchWithoutMutation},
        {TEST_DECODED_PAYLOAD_NOT_RESIDENT, ResourceDecodedPayloadRejectsNotResidentWithoutMutation},
        {TEST_DECODED_PAYLOAD_FAILED_LOAD, ResourceDecodedPayloadRejectsFailedLoadWithoutMutation},
        {TEST_DECODED_PAYLOAD_RESULT_RELEASE, ResourceDecodedPayloadReleasingResultClearsPayload},
        {TEST_DECODED_PAYLOAD_PLAN_RELEASE, ResourceDecodedPayloadReleasingPlanClearsPayload},
        {TEST_DECODED_PAYLOAD_CACHE_RELEASE, ResourceDecodedPayloadReleasingCachePayloadClearsPayload},
        {TEST_DECODED_PAYLOAD_FAILED_VALIDATION, ResourceDecodedPayloadFailedValidationDoesNotMutateResourceState},
        {TEST_PAYLOAD_WINDOW_REFERENCE_STABILITY, ResourcePayloadWindowDoesNotChangeResourceReferenceCountOrResidency}};

    const std::string_view test_name(argv[1]);
    const auto test_entry = test_registry.find(test_name);
    if (test_entry == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_entry->second();
}
