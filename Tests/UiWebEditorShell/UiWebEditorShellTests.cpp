// 模块: Tests UiWebEditorShell
// 文件: Tests/UiWebEditorShell/UiWebEditorShellTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiStackDirection.h"
#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"
#include "YuEngine/UiWebEditorShell/UiWebEditorShell.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaHeader;
using yuengine::uicore::UiFileSchemaIssueKind;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiLayoutContainerType;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStackDirection;
using yuengine::uicore::UI_FILE_SCHEMA_ID;
using yuengine::uicore::UI_FILE_SCHEMA_VERSION;
using yuengine::ui_web_editor_service::UiWebEditorLoadRequest;
using yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord;
using yuengine::ui_web_editor_service::UiWebEditorLocalService;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus;
using yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord;
using yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorShell;
using yuengine::ui_web_editor_shell::UiWebEditorShellRequest;
using yuengine::ui_web_editor_shell::UiWebEditorShellSnapshot;
using yuengine::ui_web_editor_shell::UiWebEditorShellStatus;

namespace {
constexpr const char *TEST_BUILD_SNAPSHOT =
    "UiWebEditorShell_Shell_BuildsFourPanelSnapshot";
constexpr const char *TEST_ROOT_INSPECTOR =
    "UiWebEditorShell_Shell_UsesRootInspectorWithoutSelection";
constexpr const char *TEST_MISSING_SELECTION =
    "UiWebEditorShell_Shell_RejectsMissingSelectionWithoutMutation";
constexpr const char *TEST_COUNT_MISMATCH =
    "UiWebEditorShell_Shell_RejectsDocumentCountMismatchWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiWebEditorShell_Shell_RejectsSmallOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 9001U;
constexpr std::uint32_t SENTINEL_PARENT_ID = 9002U;
constexpr std::uint32_t SENTINEL_RESOURCE_KEY = 9003U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform Transform(float anchor_max_x) {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {anchor_max_x, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.dpi_scale = 1.0F;
    return transform;
}

UiFileSchemaHeader Header(std::uint32_t layout_id, std::uint32_t root_node_id) {
    UiFileSchemaHeader header{};
    header.schema_id = UI_FILE_SCHEMA_ID;
    header.schema_version = UI_FILE_SCHEMA_VERSION;
    header.layout_id = layout_id;
    header.root_node_id = NodeId(root_node_id);
    return header;
}

UiFileNodeRecord Node(
    std::uint32_t node_id,
    std::uint32_t parent_id,
    std::uint32_t order,
    float anchor_max_x) {
    UiFileNodeRecord record{};
    record.node_id = NodeId(node_id);
    record.parent_id = NodeId(parent_id);
    record.rect_transform = Transform(anchor_max_x);
    record.sibling_order = order;
    record.layer = static_cast<std::int32_t>(order);
    return record;
}

UiFileNodeRecord RootNode(std::uint32_t node_id) {
    UiFileNodeRecord record = Node(node_id, 0U, 0U, 1.0F);
    record.parent_id = UiNodeId{};
    return record;
}

UiFileLayoutRecord Layout(std::uint32_t container_id) {
    UiFileLayoutRecord record{};
    record.container.container_id = NodeId(container_id);
    record.container.type = UiLayoutContainerType::Stack;
    record.container.stack_direction = UiStackDirection::Vertical;
    record.container.grid_column_count = 1U;
    return record;
}

UiFileStyleRef StyleRef(std::uint32_t node_id, std::uint32_t style_key) {
    UiFileStyleRef record{};
    record.node_id = NodeId(node_id);
    record.style_key = style_key;
    return record;
}

UiFileResourceRef ResourceRef(
    std::uint32_t node_id,
    UiFileResourceKind kind,
    std::uint32_t resource_key) {
    UiFileResourceRef record{};
    record.node_id = NodeId(node_id);
    record.kind = kind;
    record.resource_key = resource_key;
    return record;
}

UiFileEventBinding EventBinding(
    std::uint32_t node_id,
    std::uint32_t binding_key,
    std::uint32_t event_key) {
    UiFileEventBinding record{};
    record.node_id = NodeId(node_id);
    record.binding_key = binding_key;
    record.event_key = event_key;
    return record;
}

UiFileSchemaIssueRecord SentinelIssue() {
    UiFileSchemaIssueRecord record{};
    record.issue_kind = UiFileSchemaIssueKind::MissingRootNode;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    return record;
}

UiWebEditorHierarchyItemRecord SentinelHierarchy() {
    UiWebEditorHierarchyItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.parent_id = NodeId(SENTINEL_PARENT_ID);
    record.sibling_order = 77U;
    record.is_visible = true;
    return record;
}

UiWebEditorInspectorRecord SentinelInspector() {
    UiWebEditorInspectorRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.resource_ref_count = 77U;
    record.is_visible = true;
    return record;
}

UiWebEditorCanvasItemRecord SentinelCanvas() {
    UiWebEditorCanvasItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.rect_transform = Transform(0.25F);
    record.sibling_order = 77U;
    record.is_visible = true;
    return record;
}

UiWebEditorResourceItemRecord SentinelResource() {
    UiWebEditorResourceItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.resource_kind = UiFileResourceKind::Custom;
    record.resource_key = SENTINEL_RESOURCE_KEY;
    return record;
}

bool HierarchyMatchesSentinel(const UiWebEditorHierarchyItemRecord &record) {
    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.parent_id.value != SENTINEL_PARENT_ID) {
        return false;
    }

    return record.sibling_order == 77U;
}

bool InspectorMatchesSentinel(const UiWebEditorInspectorRecord &record) {
    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.resource_ref_count == 77U;
}

bool CanvasMatchesSentinel(const UiWebEditorCanvasItemRecord &record) {
    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.sibling_order == 77U;
}

bool ResourceMatchesSentinel(const UiWebEditorResourceItemRecord &record) {
    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.resource_key == SENTINEL_RESOURCE_KEY;
}

struct ValidSchemaFixture final {
    std::array<UiFileNodeRecord, 3U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U, 0.5F),
        Node(3U, 1U, 1U, 0.75F)};
    std::array<UiFileLayoutRecord, 1U> layouts{
        Layout(1U)};
    std::array<UiFileStyleRef, 3U> style_refs{
        StyleRef(1U, 11U),
        StyleRef(2U, 12U),
        StyleRef(3U, 13U)};
    std::array<UiFileResourceRef, 2U> resource_refs{
        ResourceRef(2U, UiFileResourceKind::Sprite, 21U),
        ResourceRef(3U, UiFileResourceKind::Font, 22U)};
    std::array<UiFileEventBinding, 2U> event_bindings{
        EventBinding(2U, 31U, 41U),
        EventBinding(3U, 32U, 42U)};
};

UiFileSchemaDesc MakeValidSchema(ValidSchemaFixture *fixture, std::uint32_t layout_id) {
    UiFileSchemaDesc desc{};
    desc.header = Header(layout_id, 1U);
    desc.nodes = fixture->nodes;
    desc.layouts = fixture->layouts;
    desc.style_refs = fixture->style_refs;
    desc.resource_refs = fixture->resource_refs;
    desc.event_bindings = fixture->event_bindings;
    return desc;
}

UiWebEditorLocalServiceStatus LoadDocument(
    const UiFileSchemaDesc &schema,
    std::uint32_t document_id,
    UiWebEditorLocalDocumentRecord *out_document) {
    UiWebEditorLoadRequest request{};
    request.document_id = document_id;
    request.schema = schema;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    return service.LoadUiFile(request, out_document, issues, &result);
}

UiWebEditorShellRequest ShellRequest(
    const UiWebEditorLocalDocumentRecord &document,
    const UiFileSchemaDesc &schema,
    std::uint32_t selected_node_id,
    bool has_selection) {
    UiWebEditorShellRequest request{};
    request.document = document;
    request.schema = schema;
    request.selected_node_id = NodeId(selected_node_id);
    request.has_selection = has_selection;
    return request;
}

int UiWebEditorShellShellBuildsFourPanelSnapshot() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 7001U);
    UiWebEditorLocalDocumentRecord document{};
    const UiWebEditorLocalServiceStatus load_status = LoadDocument(schema, 1001U, &document);
    if (load_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("web shell fixture document did not load");
    }

    const UiWebEditorShellRequest request = ShellRequest(document, schema, 2U, true);
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{};
    UiWebEditorShellSnapshot snapshot{};
    const UiWebEditorShell shell{};
    const UiWebEditorShellStatus status =
        shell.BuildSnapshot(request, hierarchy, &inspector, canvas, resources, &snapshot);
    if (status != UiWebEditorShellStatus::Success || !snapshot.Succeeded()) {
        return Fail("web shell snapshot did not succeed");
    }

    if (snapshot.hierarchy_item_count != 3U ||
        snapshot.canvas_item_count != 3U ||
        snapshot.resource_item_count != 2U) {
        return Fail("web shell snapshot panel counts mismatch");
    }

    if (!snapshot.hierarchy_ready ||
        !snapshot.inspector_ready ||
        !snapshot.canvas_ready ||
        !snapshot.resource_panel_ready) {
        return Fail("web shell snapshot panel readiness mismatch");
    }

    if (!hierarchy[0U].is_root || !hierarchy[1U].is_selected || hierarchy[2U].sibling_order != 1U) {
        return Fail("web shell hierarchy panel mismatch");
    }

    if (inspector.node_id.value != 2U ||
        inspector.style_ref_count != 1U ||
        inspector.resource_ref_count != 1U ||
        inspector.event_binding_count != 1U ||
        inspector.layout_container_count != 0U) {
        return Fail("web shell inspector panel mismatch");
    }

    if (!canvas[1U].is_selected || canvas[1U].rect_transform.anchor_max.x != 0.5F) {
        return Fail("web shell canvas panel mismatch");
    }

    if (!resources[0U].is_selected_node ||
        resources[0U].resource_kind != UiFileResourceKind::Sprite ||
        resources[1U].resource_kind != UiFileResourceKind::Font) {
        return Fail("web shell resource panel mismatch");
    }

    return 0;
}

int UiWebEditorShellShellUsesRootInspectorWithoutSelection() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 7002U);
    UiWebEditorLocalDocumentRecord document{};
    const UiWebEditorLocalServiceStatus load_status = LoadDocument(schema, 1002U, &document);
    if (load_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("web shell root fixture document did not load");
    }

    const UiWebEditorShellRequest request = ShellRequest(document, schema, 0U, false);
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{};
    UiWebEditorShellSnapshot snapshot{};
    const UiWebEditorShell shell{};
    const UiWebEditorShellStatus status =
        shell.BuildSnapshot(request, hierarchy, &inspector, canvas, resources, &snapshot);
    if (status != UiWebEditorShellStatus::Success) {
        return Fail("web shell root inspector did not succeed");
    }

    if (snapshot.has_selection || snapshot.inspector_node_id.value != 1U) {
        return Fail("web shell root inspector snapshot mismatch");
    }

    if (inspector.node_id.value != 1U ||
        inspector.layout_container_count != 1U ||
        inspector.style_ref_count != 1U ||
        !inspector.is_root) {
        return Fail("web shell root inspector record mismatch");
    }

    return 0;
}

int UiWebEditorShellShellRejectsMissingSelectionWithoutMutation() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 7003U);
    UiWebEditorLocalDocumentRecord document{};
    const UiWebEditorLocalServiceStatus load_status = LoadDocument(schema, 1003U, &document);
    if (load_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("web shell missing selection fixture document did not load");
    }

    const UiWebEditorShellRequest request = ShellRequest(document, schema, 99U, true);
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{
        SentinelHierarchy(),
        SentinelHierarchy(),
        SentinelHierarchy()};
    UiWebEditorInspectorRecord inspector = SentinelInspector();
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{
        SentinelCanvas(),
        SentinelCanvas(),
        SentinelCanvas()};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{
        SentinelResource(),
        SentinelResource()};
    UiWebEditorShellSnapshot snapshot{};
    const UiWebEditorShell shell{};
    const UiWebEditorShellStatus status =
        shell.BuildSnapshot(request, hierarchy, &inspector, canvas, resources, &snapshot);
    if (status != UiWebEditorShellStatus::MissingSelectedNode) {
        return Fail("web shell missing selection was not rejected");
    }

    if (!HierarchyMatchesSentinel(hierarchy[0U]) ||
        !InspectorMatchesSentinel(inspector) ||
        !CanvasMatchesSentinel(canvas[0U]) ||
        !ResourceMatchesSentinel(resources[0U])) {
        return Fail("web shell missing selection mutated output panels");
    }

    return 0;
}

int UiWebEditorShellShellRejectsDocumentCountMismatchWithoutMutation() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 7004U);
    UiWebEditorLocalDocumentRecord document{};
    const UiWebEditorLocalServiceStatus load_status = LoadDocument(schema, 1004U, &document);
    if (load_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("web shell count mismatch fixture document did not load");
    }

    document.node_count = 99U;
    const UiWebEditorShellRequest request = ShellRequest(document, schema, 2U, true);
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{
        SentinelHierarchy(),
        SentinelHierarchy(),
        SentinelHierarchy()};
    UiWebEditorInspectorRecord inspector = SentinelInspector();
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{
        SentinelCanvas(),
        SentinelCanvas(),
        SentinelCanvas()};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{
        SentinelResource(),
        SentinelResource()};
    UiWebEditorShellSnapshot snapshot{};
    const UiWebEditorShell shell{};
    const UiWebEditorShellStatus status =
        shell.BuildSnapshot(request, hierarchy, &inspector, canvas, resources, &snapshot);
    if (status != UiWebEditorShellStatus::SchemaCountMismatch) {
        return Fail("web shell document count mismatch was not rejected");
    }

    if (!HierarchyMatchesSentinel(hierarchy[0U]) ||
        !InspectorMatchesSentinel(inspector) ||
        !CanvasMatchesSentinel(canvas[0U]) ||
        !ResourceMatchesSentinel(resources[0U])) {
        return Fail("web shell document count mismatch mutated output panels");
    }

    return 0;
}

int UiWebEditorShellShellRejectsSmallOutputWithoutMutation() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 7005U);
    UiWebEditorLocalDocumentRecord document{};
    const UiWebEditorLocalServiceStatus load_status = LoadDocument(schema, 1005U, &document);
    if (load_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("web shell small output fixture document did not load");
    }

    const UiWebEditorShellRequest request = ShellRequest(document, schema, 2U, true);
    std::array<UiWebEditorHierarchyItemRecord, 2U> hierarchy{
        SentinelHierarchy(),
        SentinelHierarchy()};
    UiWebEditorInspectorRecord inspector = SentinelInspector();
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{
        SentinelCanvas(),
        SentinelCanvas(),
        SentinelCanvas()};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{
        SentinelResource(),
        SentinelResource()};
    UiWebEditorShellSnapshot snapshot{};
    const UiWebEditorShell shell{};
    const UiWebEditorShellStatus status =
        shell.BuildSnapshot(request, hierarchy, &inspector, canvas, resources, &snapshot);
    if (status != UiWebEditorShellStatus::OutputCapacityExceeded) {
        return Fail("web shell small output was not rejected");
    }

    if (!HierarchyMatchesSentinel(hierarchy[0U]) ||
        !InspectorMatchesSentinel(inspector) ||
        !CanvasMatchesSentinel(canvas[0U]) ||
        !ResourceMatchesSentinel(resources[0U])) {
        return Fail("web shell small output mutated output panels");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILD_SNAPSHOT) {
        return UiWebEditorShellShellBuildsFourPanelSnapshot();
    }

    if (name == TEST_ROOT_INSPECTOR) {
        return UiWebEditorShellShellUsesRootInspectorWithoutSelection();
    }

    if (name == TEST_MISSING_SELECTION) {
        return UiWebEditorShellShellRejectsMissingSelectionWithoutMutation();
    }

    if (name == TEST_COUNT_MISMATCH) {
        return UiWebEditorShellShellRejectsDocumentCountMismatchWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiWebEditorShellShellRejectsSmallOutputWithoutMutation();
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
