// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiPanelRegistryTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"
#include "YuEngine/UiRuntime/UiPanelRegistryDesc.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"
#include "YuEngine/UiRuntime/UiPanelRegistrySnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistryStatus.h"

using yuengine::uiruntime::UiPanelControllerRef;
using yuengine::uiruntime::UiPanelId;
using yuengine::uiruntime::UiPanelLayoutRef;
using yuengine::uiruntime::UiPanelManifestRecord;
using yuengine::uiruntime::UiPanelRegistry;
using yuengine::uiruntime::UiPanelRegistryDesc;
using yuengine::uiruntime::UiPanelRegistryResult;
using yuengine::uiruntime::UiPanelRegistrySnapshot;
using yuengine::uiruntime::UiPanelRegistryStatus;
using yuengine::uiruntime::UiPanelResourceRef;
using yuengine::uiruntime::UiPanelTestManifest;

namespace {
constexpr const char *TEST_RESOLVE_REFS =
    "UiRuntime_PanelRegistry_RegistersAndResolvesRefs";
constexpr const char *TEST_DUPLICATE_MISSING =
    "UiRuntime_PanelRegistry_RejectsDuplicateAndMissingPanels";
constexpr const char *TEST_TEST_MANIFEST =
    "UiRuntime_PanelRegistry_LoadsExplicitTestManifestAtomically";
constexpr const char *TEST_CAPACITY_ENTRY =
    "UiRuntime_PanelRegistry_CapacityFailureReportsEntry";
constexpr const char *TEST_EXPORT_READ_ONLY =
    "UiRuntime_PanelRegistry_ExportsManifestAsReadOnlyCopy";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiPanelId PanelId(std::uint32_t value) {
    return UiPanelId{value};
}

UiPanelLayoutRef LayoutRef(std::uint32_t asset_key, std::uint32_t variant_key) {
    UiPanelLayoutRef ref{};
    ref.layout_asset_key = asset_key;
    ref.layout_variant_key = variant_key;
    return ref;
}

UiPanelControllerRef ControllerRef(std::uint32_t type_key) {
    UiPanelControllerRef ref{};
    ref.controller_type_key = type_key;
    return ref;
}

UiPanelResourceRef ResourceRef(std::uint32_t resource_key, std::uint32_t type_key) {
    UiPanelResourceRef ref{};
    ref.resource_key = resource_key;
    ref.resource_type_key = type_key;
    return ref;
}

UiPanelManifestRecord MakePanelRecord(
    std::uint32_t panel_id,
    std::uint32_t layout_key,
    std::uint32_t controller_key,
    std::uint32_t resource_key) {
    UiPanelManifestRecord record{};
    record.panel_id = PanelId(panel_id);
    record.layout_ref = LayoutRef(layout_key, layout_key + 10U);
    record.controller_ref = ControllerRef(controller_key);
    record.resource_refs[0U] = ResourceRef(resource_key, 700U);
    record.resource_refs[1U] = ResourceRef(resource_key + 1U, 701U);
    record.resource_ref_count = 2U;
    return record;
}

int RequireStatus(UiPanelRegistryStatus actual, UiPanelRegistryStatus expected, std::string_view message) {
    if (actual == expected) {
        return 0;
    }

    return Fail(message);
}

int RequireRecordRefs(const UiPanelManifestRecord &record, const UiPanelManifestRecord &expected) {
    if (record.panel_id.value != expected.panel_id.value) {
        return Fail("panel id mismatch");
    }

    if (record.layout_ref.layout_asset_key != expected.layout_ref.layout_asset_key ||
        record.layout_ref.layout_variant_key != expected.layout_ref.layout_variant_key) {
        return Fail("layout ref mismatch");
    }

    if (record.controller_ref.controller_type_key != expected.controller_ref.controller_type_key) {
        return Fail("controller ref mismatch");
    }

    if (record.resource_ref_count != expected.resource_ref_count) {
        return Fail("resource ref count mismatch");
    }

    for (std::uint32_t index = 0U; index < record.resource_ref_count; ++index) {
        if (record.resource_refs[index].resource_key != expected.resource_refs[index].resource_key ||
            record.resource_refs[index].resource_type_key != expected.resource_refs[index].resource_type_key) {
            return Fail("resource ref mismatch");
        }
    }

    return 0;
}

int UiRuntimePanelRegistryRegistersAndResolvesRefs() {
    UiPanelRegistry registry;
    const UiPanelManifestRecord expected = MakePanelRecord(10U, 100U, 200U, 300U);
    const UiPanelRegistryResult register_result = registry.RegisterPanel(expected);
    if (!register_result.Succeeded()) {
        return Fail("panel register failed");
    }

    UiPanelManifestRecord resolved{};
    const UiPanelRegistryStatus resolve_status = registry.ResolvePanel(PanelId(10U), &resolved);
    if (RequireStatus(resolve_status, UiPanelRegistryStatus::Success, "panel resolve failed") != 0) {
        return 1;
    }

    if (RequireRecordRefs(resolved, expected) != 0) {
        return 1;
    }

    const UiPanelRegistrySnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_panel_count != 1U || snapshot.accepted_operation_count != 1U) {
        return Fail("panel registry snapshot mismatch");
    }

    return 0;
}

int UiRuntimePanelRegistryRejectsDuplicateAndMissingPanels() {
    UiPanelRegistry registry;
    const UiPanelManifestRecord record = MakePanelRecord(20U, 101U, 201U, 301U);
    UiPanelRegistryResult result = registry.RegisterPanel(record);
    if (!result.Succeeded()) {
        return Fail("first panel register failed");
    }

    result = registry.RegisterPanel(record);
    if (result.status != UiPanelRegistryStatus::DuplicatePanelId) {
        return Fail("duplicate panel was not rejected");
    }

    UiPanelManifestRecord resolved{};
    UiPanelRegistryStatus status = registry.ResolvePanel(PanelId(999U), &resolved);
    if (RequireStatus(status, UiPanelRegistryStatus::PanelNotFound, "missing panel status mismatch") != 0) {
        return 1;
    }

    status = registry.ResolvePanel(UiPanelId{}, &resolved);
    if (RequireStatus(status, UiPanelRegistryStatus::InvalidPanelId, "invalid panel status mismatch") != 0) {
        return 1;
    }

    status = registry.ResolvePanel(PanelId(20U), nullptr);
    if (RequireStatus(status, UiPanelRegistryStatus::InvalidOutputBuffer, "null output status mismatch") != 0) {
        return 1;
    }

    const UiPanelManifestRecord second_record = MakePanelRecord(21U, 102U, 202U, 302U);
    result = registry.RegisterPanel(second_record);
    if (!result.Succeeded()) {
        return Fail("second panel register after failure failed");
    }

    const UiPanelRegistrySnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_panel_count != 2U ||
        snapshot.accepted_operation_count != 2U ||
        snapshot.rejected_operation_count != 1U ||
        snapshot.failed_operation_count != 1U ||
        snapshot.duplicate_panel_rejected_count != 1U ||
        snapshot.last_status != UiPanelRegistryStatus::Success) {
        return Fail("duplicate snapshot mismatch");
    }

    return 0;
}

int UiRuntimePanelRegistryLoadsExplicitTestManifestAtomically() {
    UiPanelRegistry registry;
    std::array<UiPanelManifestRecord, 2U> records{};
    records[0U] = MakePanelRecord(31U, 110U, 210U, 310U);
    records[1U] = MakePanelRecord(32U, 111U, 211U, 311U);

    UiPanelTestManifest manifest{};
    manifest.records = std::span<const UiPanelManifestRecord>(records.data(), records.size());
    UiPanelRegistryResult result = registry.RegisterManifest(manifest);
    if (!result.Succeeded()) {
        return Fail("test manifest register failed");
    }

    UiPanelManifestRecord resolved{};
    UiPanelRegistryStatus status = registry.ResolvePanel(PanelId(32U), &resolved);
    if (RequireStatus(status, UiPanelRegistryStatus::Success, "manifest panel resolve failed") != 0) {
        return 1;
    }

    if (RequireRecordRefs(resolved, records[1U]) != 0) {
        return 1;
    }

    std::array<UiPanelManifestRecord, 2U> duplicate_records{};
    duplicate_records[0U] = MakePanelRecord(41U, 120U, 220U, 320U);
    duplicate_records[1U] = MakePanelRecord(41U, 121U, 221U, 321U);
    manifest.records = std::span<const UiPanelManifestRecord>(duplicate_records.data(), duplicate_records.size());
    result = registry.RegisterManifest(manifest);
    if (result.status != UiPanelRegistryStatus::DuplicatePanelId) {
        return Fail("duplicate manifest status mismatch");
    }

    status = registry.ResolvePanel(PanelId(41U), &resolved);
    if (RequireStatus(status, UiPanelRegistryStatus::PanelNotFound, "duplicate manifest mutated registry") != 0) {
        return 1;
    }

    const UiPanelRegistrySnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_panel_count != 2U ||
        snapshot.rejected_operation_count != 1U ||
        snapshot.failed_operation_count != 1U) {
        return Fail("manifest atomic snapshot mismatch");
    }

    return 0;
}

int UiRuntimePanelRegistryCapacityFailureReportsEntry() {
    UiPanelRegistryDesc desc{};
    desc.panel_capacity = 1U;
    UiPanelRegistry registry(desc);
    const UiPanelManifestRecord first = MakePanelRecord(51U, 130U, 230U, 330U);
    const UiPanelManifestRecord overflow = MakePanelRecord(52U, 131U, 231U, 331U);
    if (!registry.RegisterPanel(first).Succeeded()) {
        return Fail("capacity fixture register failed");
    }

    UiPanelRegistryResult result = registry.RegisterPanel(overflow);
    if (result.status != UiPanelRegistryStatus::CapacityExceeded) {
        return Fail("panel capacity status mismatch");
    }

    if (result.required_record_count != 2U ||
        result.failed_panel_id.value != overflow.panel_id.value ||
        result.failed_record_index != 1U) {
        return Fail("panel capacity result entry mismatch");
    }

    UiPanelRegistrySnapshot snapshot = registry.Snapshot();
    if (snapshot.required_record_count != 2U ||
        snapshot.failed_panel_id.value != overflow.panel_id.value ||
        snapshot.failed_record_index != 1U) {
        return Fail("panel capacity snapshot entry mismatch");
    }

    result = registry.RegisterPanel(first);
    if (result.status != UiPanelRegistryStatus::DuplicatePanelId) {
        return Fail("duplicate did not clear capacity entry");
    }

    snapshot = registry.Snapshot();
    if (snapshot.failed_panel_id.value != 0U || snapshot.required_record_count != 0U) {
        return Fail("duplicate left stale capacity entry");
    }

    UiPanelRegistryDesc manifest_desc{};
    manifest_desc.panel_capacity = 2U;
    UiPanelRegistry manifest_registry(manifest_desc);
    const UiPanelManifestRecord existing = MakePanelRecord(61U, 140U, 240U, 340U);
    const UiPanelManifestRecord fit_record = MakePanelRecord(62U, 141U, 241U, 341U);
    const UiPanelManifestRecord failed_record = MakePanelRecord(63U, 142U, 242U, 342U);
    if (!manifest_registry.RegisterPanel(existing).Succeeded()) {
        return Fail("manifest capacity fixture register failed");
    }

    std::array<UiPanelManifestRecord, 2U> manifest_records{fit_record, failed_record};
    UiPanelTestManifest manifest{};
    manifest.records = std::span<const UiPanelManifestRecord>(manifest_records.data(), manifest_records.size());
    result = manifest_registry.RegisterManifest(manifest);
    if (result.status != UiPanelRegistryStatus::CapacityExceeded) {
        return Fail("manifest capacity status mismatch");
    }

    if (result.required_record_count != 3U ||
        result.failed_panel_id.value != failed_record.panel_id.value ||
        result.failed_record_index != 1U) {
        return Fail("manifest capacity result entry mismatch");
    }

    UiPanelManifestRecord resolved{};
    UiPanelRegistryStatus status = manifest_registry.ResolvePanel(fit_record.panel_id, &resolved);
    if (status != UiPanelRegistryStatus::PanelNotFound) {
        return Fail("manifest capacity mutated registry");
    }

    result = manifest_registry.RegisterPanel(fit_record);
    if (!result.Succeeded()) {
        return Fail("success after manifest capacity failed");
    }

    snapshot = manifest_registry.Snapshot();
    if (snapshot.failed_panel_id.value != 0U || snapshot.required_record_count != 0U) {
        return Fail("success left stale capacity entry");
    }

    return 0;
}

int UiRuntimePanelRegistryExportsManifestAsReadOnlyCopy() {
    UiPanelRegistryDesc desc{};
    desc.panel_capacity = 2U;
    UiPanelRegistry registry(desc);
    const UiPanelManifestRecord first = MakePanelRecord(51U, 130U, 230U, 330U);
    const UiPanelManifestRecord second = MakePanelRecord(52U, 131U, 231U, 331U);

    if (!registry.RegisterPanel(first).Succeeded()) {
        return Fail("first export fixture register failed");
    }

    if (!registry.RegisterPanel(second).Succeeded()) {
        return Fail("second export fixture register failed");
    }

    std::array<UiPanelManifestRecord, 2U> output_records{};
    UiPanelRegistryResult export_result = registry.ExportManifest(output_records.data(), 1U);
    if (export_result.status != UiPanelRegistryStatus::InvalidOutputBuffer ||
        export_result.required_record_count != 2U) {
        return Fail("small manifest export status mismatch");
    }

    export_result = registry.ExportManifest(output_records.data(), static_cast<std::uint32_t>(output_records.size()));
    if (!export_result.Succeeded() || export_result.required_record_count != 2U) {
        return Fail("manifest export failed");
    }

    output_records[0U].layout_ref.layout_asset_key = 9999U;
    UiPanelManifestRecord resolved{};
    const UiPanelRegistryStatus status = registry.ResolvePanel(PanelId(51U), &resolved);
    if (RequireStatus(status, UiPanelRegistryStatus::Success, "readonly copy resolve failed") != 0) {
        return 1;
    }

    if (resolved.layout_ref.layout_asset_key != first.layout_ref.layout_asset_key) {
        return Fail("exported manifest mutated registry storage");
    }

    return 0;
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_RESOLVE_REFS) {
        return UiRuntimePanelRegistryRegistersAndResolvesRefs();
    }

    if (test_name == TEST_DUPLICATE_MISSING) {
        return UiRuntimePanelRegistryRejectsDuplicateAndMissingPanels();
    }

    if (test_name == TEST_TEST_MANIFEST) {
        return UiRuntimePanelRegistryLoadsExplicitTestManifestAtomically();
    }

    if (test_name == TEST_CAPACITY_ENTRY) {
        return UiRuntimePanelRegistryCapacityFailureReportsEntry();
    }

    if (test_name == TEST_EXPORT_READ_ONLY) {
        return UiRuntimePanelRegistryExportsManifestAsReadOnlyCopy();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
