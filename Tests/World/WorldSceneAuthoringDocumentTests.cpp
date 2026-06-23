// 模块: Tests World
// 文件: Tests/World/WorldSceneAuthoringDocumentTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneAuthoringDocument.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectHandle;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceTypeId;
using yuengine::world::WorldComponentAttachmentSnapshotRecord;
using yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using yuengine::world::WorldComponentSlotId;
using yuengine::world::WorldComponentTypeId;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldSceneAuthoringDependencyRecord;
using yuengine::world::WorldSceneAuthoringDocument;
using yuengine::world::WorldSceneAuthoringDocumentDesc;
using yuengine::world::WorldSceneAuthoringDocumentResult;
using yuengine::world::WorldSceneAuthoringDocumentSnapshot;
using yuengine::world::WorldSceneAuthoringDocumentStatus;
using yuengine::world::WorldSceneAuthoringDocumentValidator;
using yuengine::world::WorldSceneAuthoringRuntimeExport;
using yuengine::world::WorldSceneEditorSidecarExportPolicy;
using yuengine::world::WorldSceneEditorSidecarKind;
using yuengine::world::WorldSceneEditorSidecarRecord;
using yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord;
using yuengine::world::WorldSceneObjectTransformRestoreTransformRecord;
using yuengine::world::WorldTransformState;

namespace {
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *TEST_EXPORTS_RUNTIME_ONLY =
    "WorldSceneAuthoringDocument_ExportsRuntimeRecordsWithoutSidecar";
constexpr const char *TEST_REJECTS_SIDECAR_RUNTIME_EXPORT =
    "WorldSceneAuthoringDocument_RejectsSidecarRuntimeExportWithoutMutation";
constexpr const char *TEST_REJECTS_DUPLICATE_OBJECT_IDS =
    "WorldSceneAuthoringDocument_RejectsDuplicateObjectIdsWithoutMutation";
constexpr const char *TEST_REJECTS_MISSING_REFS =
    "WorldSceneAuthoringDocument_RejectsMissingReferencesWithoutMutation";
constexpr const char *TEST_REJECTS_UNSUPPORTED_FIELDS =
    "WorldSceneAuthoringDocument_RejectsUnsupportedRuntimeFieldsWithoutMutation";
constexpr const char *TEST_REJECTS_CAPACITY =
    "WorldSceneAuthoringDocument_RejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_REJECTS_MISSING_DEPENDENCY =
    "WorldSceneAuthoringDocument_RejectsMissingCookDependencyWithoutMutation";
constexpr const char *TEST_REJECTS_SIDECAR_MISSING_OBJECT =
    "WorldSceneAuthoringDocument_RejectsSidecarMissingObjectWithoutMutation";
constexpr const char *TEST_SNAPSHOT =
    "WorldSceneAuthoringDocument_SnapshotReportsCountsAndSidecarRejection";

constexpr std::uint32_t SENTINEL_COUNT = 99U;

int Fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

WorldObjectId ObjectId(std::uint32_t value) {
    return WorldObjectId{value};
}

ObjectHandle MakeObjectHandle(std::uint32_t slot, std::uint32_t generation) {
    ObjectHandle handle{};
    handle.slot = slot;
    handle.generation = generation;
    return handle;
}

ResourceHandle MakeResourceHandle(std::uint32_t slot, std::uint32_t generation) {
    ResourceHandle handle{};
    handle.slot = slot;
    handle.generation = generation;
    return handle;
}

ResourceTypeId ResourceType(std::uint32_t value) {
    return ResourceTypeId{value};
}

WorldTransformState Transform(float seed) {
    WorldTransformState transform{};
    transform.translation_x = seed;
    transform.translation_y = seed + 1.0F;
    transform.translation_z = seed + 2.0F;
    transform.rotation_z = seed * 0.01F;
    transform.scale_x = 1.0F + seed * 0.001F;
    transform.scale_y = 1.0F + seed * 0.001F;
    transform.scale_z = 1.0F + seed * 0.001F;
    return transform;
}

bool IdentityMatches(
    const WorldSceneObjectTransformRestoreIdentityRecord &left,
    const WorldSceneObjectTransformRestoreIdentityRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.object_handle.slot != right.object_handle.slot) {
        return false;
    }

    return left.object_handle.generation == right.object_handle.generation;
}

bool TransformMatches(
    const WorldTransformState &left,
    const WorldTransformState &right) {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

bool AttachmentMatches(
    const WorldComponentAttachmentSnapshotRecord &left,
    const WorldComponentAttachmentSnapshotRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.component_type_id.value != right.component_type_id.value) {
        return false;
    }

    return left.component_slot_id.value == right.component_slot_id.value;
}

bool BindingMatches(
    const WorldComponentResourceBindingSnapshotRecord &left,
    const WorldComponentResourceBindingSnapshotRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.component_type_id.value != right.component_type_id.value) {
        return false;
    }

    if (left.component_slot_id.value != right.component_slot_id.value) {
        return false;
    }

    if (left.resource_handle.slot != right.resource_handle.slot) {
        return false;
    }

    if (left.resource_handle.generation != right.resource_handle.generation) {
        return false;
    }

    return left.expected_resource_type.value == right.expected_resource_type.value;
}

bool DependencyMatches(
    const WorldSceneAuthoringDependencyRecord &left,
    const WorldSceneAuthoringDependencyRecord &right) {
    if (left.stable_resource_id != right.stable_resource_id) {
        return false;
    }

    if (left.resource_handle.slot != right.resource_handle.slot) {
        return false;
    }

    if (left.resource_handle.generation != right.resource_handle.generation) {
        return false;
    }

    return left.expected_resource_type.value == right.expected_resource_type.value;
}

WorldSceneAuthoringDocument MakeDocument(
    const WorldSceneObjectTransformRestoreIdentityRecord *identities,
    std::uint32_t identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transforms,
    std::uint32_t transform_count,
    const WorldComponentAttachmentSnapshotRecord *attachments,
    std::uint32_t attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *bindings,
    std::uint32_t binding_count,
    const WorldSceneAuthoringDependencyRecord *dependencies,
    std::uint32_t dependency_count,
    const WorldSceneEditorSidecarRecord *sidecars,
    std::uint32_t sidecar_count) {
    WorldSceneAuthoringDocument document{};
    document.header.scene_document_id = 0x1001U;
    document.header.deterministic_document_hash = 0x55AAU;
    document.header.identity_record_count = identity_count;
    document.header.transform_record_count = transform_count;
    document.header.attachment_record_count = attachment_count;
    document.header.binding_record_count = binding_count;
    document.header.dependency_record_count = dependency_count;
    document.header.sidecar_record_count = sidecar_count;
    document.identity_records = identities;
    document.transform_records = transforms;
    document.attachment_records = attachments;
    document.binding_records = bindings;
    document.dependency_records = dependencies;
    document.sidecar_records = sidecars;
    return document;
}

WorldSceneAuthoringRuntimeExport MakeRuntimeExport(
    WorldSceneObjectTransformRestoreIdentityRecord *identities,
    std::uint32_t identity_capacity,
    std::uint32_t *identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *transforms,
    std::uint32_t transform_capacity,
    std::uint32_t *transform_count,
    WorldComponentAttachmentSnapshotRecord *attachments,
    std::uint32_t attachment_capacity,
    std::uint32_t *attachment_count,
    WorldComponentResourceBindingSnapshotRecord *bindings,
    std::uint32_t binding_capacity,
    std::uint32_t *binding_count,
    WorldSceneAuthoringDependencyRecord *dependencies,
    std::uint32_t dependency_capacity,
    std::uint32_t *dependency_count) {
    WorldSceneAuthoringRuntimeExport runtime_output{};
    runtime_output.identity_records = identities;
    runtime_output.identity_capacity = identity_capacity;
    runtime_output.identity_count = identity_count;
    runtime_output.transform_records = transforms;
    runtime_output.transform_capacity = transform_capacity;
    runtime_output.transform_count = transform_count;
    runtime_output.attachment_records = attachments;
    runtime_output.attachment_capacity = attachment_capacity;
    runtime_output.attachment_count = attachment_count;
    runtime_output.binding_records = bindings;
    runtime_output.binding_capacity = binding_capacity;
    runtime_output.binding_count = binding_count;
    runtime_output.dependency_records = dependencies;
    runtime_output.dependency_capacity = dependency_capacity;
    runtime_output.dependency_count = dependency_count;
    return runtime_output;
}

int ExpectFailureWithoutMutation(
    WorldSceneAuthoringDocumentValidator &validator,
    const WorldSceneAuthoringDocument &document,
    WorldSceneAuthoringDocumentStatus expected_status) {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> output_identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(42U), MakeObjectHandle(42U, 42U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(43U), MakeObjectHandle(43U, 43U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> output_transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(42U), Transform(420.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(43U), Transform(430.0F)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> output_attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(42U), WorldComponentTypeId{42U}, WorldComponentSlotId{42U}},
        WorldComponentAttachmentSnapshotRecord{ObjectId(43U), WorldComponentTypeId{43U}, WorldComponentSlotId{43U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> output_bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(42U),
            WorldComponentTypeId{42U},
            WorldComponentSlotId{42U},
            MakeResourceHandle(42U, 42U),
            ResourceType(42U)},
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(43U),
            WorldComponentTypeId{43U},
            WorldComponentSlotId{43U},
            MakeResourceHandle(43U, 43U),
            ResourceType(43U)}};
    std::array<WorldSceneAuthoringDependencyRecord, 2U> output_dependencies{
        WorldSceneAuthoringDependencyRecord{4200U, MakeResourceHandle(42U, 42U), ResourceType(42U)},
        WorldSceneAuthoringDependencyRecord{4300U, MakeResourceHandle(43U, 43U), ResourceType(43U)}};
    const WorldSceneObjectTransformRestoreIdentityRecord first_identity = output_identities[0];
    const WorldSceneObjectTransformRestoreTransformRecord first_transform = output_transforms[0];
    const WorldComponentAttachmentSnapshotRecord first_attachment = output_attachments[0];
    const WorldComponentResourceBindingSnapshotRecord first_binding = output_bindings[0];
    const WorldSceneAuthoringDependencyRecord first_dependency = output_dependencies[0];
    std::uint32_t identity_count = SENTINEL_COUNT;
    std::uint32_t transform_count = SENTINEL_COUNT;
    std::uint32_t attachment_count = SENTINEL_COUNT;
    std::uint32_t binding_count = SENTINEL_COUNT;
    std::uint32_t dependency_count = SENTINEL_COUNT;
    WorldSceneAuthoringRuntimeExport runtime_output = MakeRuntimeExport(
        output_identities.data(),
        static_cast<std::uint32_t>(output_identities.size()),
        &identity_count,
        output_transforms.data(),
        static_cast<std::uint32_t>(output_transforms.size()),
        &transform_count,
        output_attachments.data(),
        static_cast<std::uint32_t>(output_attachments.size()),
        &attachment_count,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count,
        output_dependencies.data(),
        static_cast<std::uint32_t>(output_dependencies.size()),
        &dependency_count);

    const WorldSceneAuthoringDocumentResult result = validator.ValidateAndExport(document, &runtime_output);
    if (result.status != expected_status) {
        return Fail("authoring document failure returned wrong status");
    }

    if (identity_count != SENTINEL_COUNT) {
        return Fail("authoring document failure mutated identity count");
    }

    if (transform_count != SENTINEL_COUNT) {
        return Fail("authoring document failure mutated transform count");
    }

    if (attachment_count != SENTINEL_COUNT) {
        return Fail("authoring document failure mutated attachment count");
    }

    if (binding_count != SENTINEL_COUNT) {
        return Fail("authoring document failure mutated binding count");
    }

    if (dependency_count != SENTINEL_COUNT) {
        return Fail("authoring document failure mutated dependency count");
    }

    if (!IdentityMatches(output_identities[0], first_identity)) {
        return Fail("authoring document failure mutated identity output");
    }

    if (!TransformMatches(output_transforms[0].transform_state, first_transform.transform_state)) {
        return Fail("authoring document failure mutated transform output");
    }

    if (!AttachmentMatches(output_attachments[0], first_attachment)) {
        return Fail("authoring document failure mutated attachment output");
    }

    if (!BindingMatches(output_bindings[0], first_binding)) {
        return Fail("authoring document failure mutated binding output");
    }

    if (!DependencyMatches(output_dependencies[0], first_dependency)) {
        return Fail("authoring document failure mutated dependency output");
    }

    return 0;
}

int WorldSceneAuthoringDocumentExportsRuntimeRecordsWithoutSidecar() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 9U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)},
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(2U), Transform(20.0F)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{7U}, WorldComponentSlotId{3U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(1U),
            WorldComponentTypeId{7U},
            WorldComponentSlotId{3U},
            MakeResourceHandle(5U, 11U),
            ResourceType(17U)}};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> dependencies{
        WorldSceneAuthoringDependencyRecord{5001U, MakeResourceHandle(5U, 11U), ResourceType(17U)}};
    std::array<WorldSceneEditorSidecarRecord, 2U> sidecars{
        WorldSceneEditorSidecarRecord{
            WorldSceneEditorSidecarKind::Selection,
            WorldSceneEditorSidecarExportPolicy::EditorOnly,
            ObjectId(1U),
            0U,
            1U},
        WorldSceneEditorSidecarRecord{
            WorldSceneEditorSidecarKind::ViewportCameraBookmark,
            WorldSceneEditorSidecarExportPolicy::EditorOnly,
            WorldObjectId{},
            1U,
            300U}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        static_cast<std::uint32_t>(identities.size()),
        transforms.data(),
        static_cast<std::uint32_t>(transforms.size()),
        attachments.data(),
        static_cast<std::uint32_t>(attachments.size()),
        bindings.data(),
        static_cast<std::uint32_t>(bindings.size()),
        dependencies.data(),
        static_cast<std::uint32_t>(dependencies.size()),
        sidecars.data(),
        static_cast<std::uint32_t>(sidecars.size()));

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> output_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 2U> output_transforms{};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> output_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> output_bindings{};
    std::array<WorldSceneAuthoringDependencyRecord, 1U> output_dependencies{};
    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
    std::uint32_t dependency_count = 0U;
    WorldSceneAuthoringRuntimeExport runtime_output = MakeRuntimeExport(
        output_identities.data(),
        static_cast<std::uint32_t>(output_identities.size()),
        &identity_count,
        output_transforms.data(),
        static_cast<std::uint32_t>(output_transforms.size()),
        &transform_count,
        output_attachments.data(),
        static_cast<std::uint32_t>(output_attachments.size()),
        &attachment_count,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count,
        output_dependencies.data(),
        static_cast<std::uint32_t>(output_dependencies.size()),
        &dependency_count);
    WorldSceneAuthoringDocumentValidator validator;
    const WorldSceneAuthoringDocumentResult result = validator.ValidateAndExport(document, &runtime_output);
    if (!result.Succeeded()) {
        return Fail("authoring document export failed");
    }

    if (identity_count != identities.size()) {
        return Fail("authoring document identity count wrong");
    }

    if (transform_count != transforms.size()) {
        return Fail("authoring document transform count wrong");
    }

    if (attachment_count != attachments.size()) {
        return Fail("authoring document attachment count wrong");
    }

    if (binding_count != bindings.size()) {
        return Fail("authoring document binding count wrong");
    }

    if (dependency_count != dependencies.size()) {
        return Fail("authoring document dependency count wrong");
    }

    if (!IdentityMatches(output_identities[0], identities[0])) {
        return Fail("authoring document identity output wrong");
    }

    if (!TransformMatches(output_transforms[1].transform_state, transforms[1].transform_state)) {
        return Fail("authoring document transform output wrong");
    }

    if (!AttachmentMatches(output_attachments[0], attachments[0])) {
        return Fail("authoring document attachment output wrong");
    }

    if (!BindingMatches(output_bindings[0], bindings[0])) {
        return Fail("authoring document binding output wrong");
    }

    if (!DependencyMatches(output_dependencies[0], dependencies[0])) {
        return Fail("authoring document dependency output wrong");
    }

    if (result.state.validated_sidecar_count != sidecars.size()) {
        return Fail("authoring document sidecar count wrong");
    }

    const WorldSceneAuthoringDocumentSnapshot snapshot = validator.Snapshot();
    if (snapshot.runtime_export_count != 1U) {
        return Fail("authoring document snapshot export count wrong");
    }

    return 0;
}

int WorldSceneAuthoringDocumentRejectsSidecarRuntimeExportWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(1U), Transform(10.0F)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        WorldSceneEditorSidecarRecord{
            WorldSceneEditorSidecarKind::Selection,
            WorldSceneEditorSidecarExportPolicy::RuntimeData,
            ObjectId(1U),
            0U,
            1U}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::SidecarMarkedForRuntimeExport);
}

int WorldSceneAuthoringDocumentRejectsDuplicateObjectIdsWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(2U, 9U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        2U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::DuplicateIdentityWorldObjectId);
}

int WorldSceneAuthoringDocumentRejectsMissingReferencesWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, 1U> transforms{
        WorldSceneObjectTransformRestoreTransformRecord{ObjectId(3U), Transform(10.0F)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        transforms.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::MissingIdentityForTransform);
}

int WorldSceneAuthoringDocumentRejectsUnsupportedRuntimeFieldsWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    document.header.unsupported_runtime_field_count = 1U;
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::UnsupportedRuntimeField);
}

int WorldSceneAuthoringDocumentRejectsCapacityOverflowWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 2U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)},
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(2U), MakeObjectHandle(2U, 9U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        2U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U);
    WorldSceneAuthoringDocumentDesc desc{};
    desc.identity_capacity = 1U;
    WorldSceneAuthoringDocumentValidator validator(desc);
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::InputCountExceeded);
}

int WorldSceneAuthoringDocumentRejectsMissingCookDependencyWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> attachments{
        WorldComponentAttachmentSnapshotRecord{ObjectId(1U), WorldComponentTypeId{7U}, WorldComponentSlotId{3U}}};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> bindings{
        WorldComponentResourceBindingSnapshotRecord{
            ObjectId(1U),
            WorldComponentTypeId{7U},
            WorldComponentSlotId{3U},
            MakeResourceHandle(5U, 11U),
            ResourceType(17U)}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        attachments.data(),
        1U,
        bindings.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U);
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::MissingDependencyForBinding);
}

int WorldSceneAuthoringDocumentRejectsSidecarMissingObjectWithoutMutation() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        WorldSceneEditorSidecarRecord{
            WorldSceneEditorSidecarKind::Selection,
            WorldSceneEditorSidecarExportPolicy::EditorOnly,
            ObjectId(2U),
            0U,
            1U}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);
    WorldSceneAuthoringDocumentValidator validator;
    return ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::SidecarReferencesMissingObject);
}

int WorldSceneAuthoringDocumentSnapshotReportsCountsAndSidecarRejection() {
    std::array<WorldSceneObjectTransformRestoreIdentityRecord, 1U> identities{
        WorldSceneObjectTransformRestoreIdentityRecord{ObjectId(1U), MakeObjectHandle(1U, 9U)}};
    std::array<WorldSceneEditorSidecarRecord, 1U> sidecars{
        WorldSceneEditorSidecarRecord{
            WorldSceneEditorSidecarKind::Selection,
            WorldSceneEditorSidecarExportPolicy::RuntimeData,
            ObjectId(1U),
            0U,
            1U}};
    const WorldSceneAuthoringDocument document = MakeDocument(
        identities.data(),
        1U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        nullptr,
        0U,
        sidecars.data(),
        1U);
    WorldSceneAuthoringDocumentValidator validator;
    if (ExpectFailureWithoutMutation(
        validator,
        document,
        WorldSceneAuthoringDocumentStatus::SidecarMarkedForRuntimeExport) != 0) {
        return 1;
    }

    const WorldSceneAuthoringDocumentSnapshot snapshot = validator.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("authoring document snapshot failure count wrong");
    }

    if (snapshot.rejected_document_count != 1U) {
        return Fail("authoring document snapshot rejected document count wrong");
    }

    if (snapshot.rejected_sidecar_count != 1U) {
        return Fail("authoring document snapshot rejected sidecar count wrong");
    }

    if (snapshot.last_status != WorldSceneAuthoringDocumentStatus::SidecarMarkedForRuntimeExport) {
        return Fail("authoring document snapshot last status wrong");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("authoring document snapshot allocation status wrong");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_EXPORTS_RUNTIME_ONLY) {
        return WorldSceneAuthoringDocumentExportsRuntimeRecordsWithoutSidecar();
    }

    if (test_name == TEST_REJECTS_SIDECAR_RUNTIME_EXPORT) {
        return WorldSceneAuthoringDocumentRejectsSidecarRuntimeExportWithoutMutation();
    }

    if (test_name == TEST_REJECTS_DUPLICATE_OBJECT_IDS) {
        return WorldSceneAuthoringDocumentRejectsDuplicateObjectIdsWithoutMutation();
    }

    if (test_name == TEST_REJECTS_MISSING_REFS) {
        return WorldSceneAuthoringDocumentRejectsMissingReferencesWithoutMutation();
    }

    if (test_name == TEST_REJECTS_UNSUPPORTED_FIELDS) {
        return WorldSceneAuthoringDocumentRejectsUnsupportedRuntimeFieldsWithoutMutation();
    }

    if (test_name == TEST_REJECTS_CAPACITY) {
        return WorldSceneAuthoringDocumentRejectsCapacityOverflowWithoutMutation();
    }

    if (test_name == TEST_REJECTS_MISSING_DEPENDENCY) {
        return WorldSceneAuthoringDocumentRejectsMissingCookDependencyWithoutMutation();
    }

    if (test_name == TEST_REJECTS_SIDECAR_MISSING_OBJECT) {
        return WorldSceneAuthoringDocumentRejectsSidecarMissingObjectWithoutMutation();
    }

    if (test_name == TEST_SNAPSHOT) {
        return WorldSceneAuthoringDocumentSnapshotReportsCountsAndSidecarRejection();
    }

    return Fail("unknown test name");
}
