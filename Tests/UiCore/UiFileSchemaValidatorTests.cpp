// 模块: Tests UiCore
// 文件: Tests/UiCore/UiFileSchemaValidatorTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiStackDirection.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaHeader;
using yuengine::uicore::UiFileSchemaIssueKind;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileSchemaStatus;
using yuengine::uicore::UiFileSchemaValidationResult;
using yuengine::uicore::UiFileSchemaValidator;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiLayoutContainerType;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStackDirection;
using yuengine::uicore::UI_FILE_SCHEMA_ID;
using yuengine::uicore::UI_FILE_SCHEMA_VERSION;

namespace {
constexpr const char *TEST_VALID =
    "UiCore_UiFileSchema_ValidatesGenericRuntimeData";
constexpr const char *TEST_ISSUES =
    "UiCore_UiFileSchema_ReportsMissingDuplicateAndBindingIssues";
constexpr const char *TEST_INVALID_HEADER =
    "UiCore_UiFileSchema_RejectsInvalidHeaderWithoutMutation";
constexpr const char *TEST_INVALID_RECORDS =
    "UiCore_UiFileSchema_RejectsInvalidRecordsWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_UiFileSchema_RejectsSmallIssueOutputWithoutMutation";
constexpr const char *TEST_CAPACITY_ENTRY =
    "UiCore_UiFileSchema_OutputCapacityReportsFirstUnreportedIssue";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;
constexpr std::uint32_t SENTINEL_CONTEXT_KEY = 888U;
constexpr std::uint32_t SENTINEL_RECORD_INDEX = 9U;
constexpr std::uint32_t SENTINEL_DUPLICATE_COUNT = 3U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform DefaultTransform() {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
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

UiFileNodeRecord Node(std::uint32_t node_id, std::uint32_t parent_id, std::uint32_t order) {
    UiFileNodeRecord record{};
    record.node_id = NodeId(node_id);
    record.parent_id = NodeId(parent_id);
    record.rect_transform = DefaultTransform();
    record.sibling_order = order;
    record.layer = 1;
    return record;
}

UiFileNodeRecord RootNode(std::uint32_t node_id) {
    UiFileNodeRecord record = Node(node_id, 0U, 0U);
    record.parent_id = UiNodeId{};
    return record;
}

UiFileLayoutRecord Layout(std::uint32_t container_id, UiLayoutContainerType type) {
    UiFileLayoutRecord record{};
    record.container.container_id = NodeId(container_id);
    record.container.type = type;
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
    record.context_key = SENTINEL_CONTEXT_KEY;
    record.record_index = SENTINEL_RECORD_INDEX;
    record.duplicate_count = SENTINEL_DUPLICATE_COUNT;
    return record;
}

bool IssueMatchesSentinel(const UiFileSchemaIssueRecord &record) {
    if (record.issue_kind != UiFileSchemaIssueKind::MissingRootNode) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.context_key != SENTINEL_CONTEXT_KEY) {
        return false;
    }

    if (record.record_index != SENTINEL_RECORD_INDEX) {
        return false;
    }

    return record.duplicate_count == SENTINEL_DUPLICATE_COUNT;
}

bool CapacityEntryIsClear(const UiFileSchemaValidationResult &result) {
    if (result.required_issue_count != 0U) {
        return false;
    }

    if (result.capacity_entry_issue_capacity != 0U) {
        return false;
    }

    if (result.capacity_entry_current_issue_count != 0U) {
        return false;
    }

    if (result.capacity_entry_required_issue_count != 0U) {
        return false;
    }

    if (result.failed_issue_kind != UiFileSchemaIssueKind::None) {
        return false;
    }

    if (result.failed_context_key != 0U) {
        return false;
    }

    return result.failed_duplicate_count == 0U;
}

int UiCoreUiFileSchemaValidatesGenericRuntimeData() {
    const std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U)};
    const std::array<UiFileLayoutRecord, 1U> layouts{
        Layout(1U, UiLayoutContainerType::Stack)};
    const std::array<UiFileStyleRef, 2U> style_refs{
        StyleRef(1U, 11U),
        StyleRef(2U, 12U)};
    const std::array<UiFileResourceRef, 2U> resource_refs{
        ResourceRef(1U, UiFileResourceKind::Sprite, 21U),
        ResourceRef(2U, UiFileResourceKind::Font, 22U)};
    const std::array<UiFileEventBinding, 1U> event_bindings{
        EventBinding(2U, 31U, 41U)};

    UiFileSchemaDesc desc{};
    desc.header = Header(9001U, 1U);
    desc.nodes = nodes;
    desc.layouts = layouts;
    desc.style_refs = style_refs;
    desc.resource_refs = resource_refs;
    desc.event_bindings = event_bindings;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    const UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::Success || !result.Succeeded()) {
        return Fail("valid ui file schema did not pass");
    }

    if (result.checked_node_count != 2U ||
        result.checked_layout_count != 1U ||
        result.checked_style_ref_count != 2U ||
        result.checked_resource_ref_count != 2U ||
        result.checked_event_binding_count != 1U ||
        result.issue_count != 0U) {
        return Fail("valid ui file schema counts mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("valid ui file schema mutated issue buffer");
    }

    return 0;
}

int UiCoreUiFileSchemaReportsMissingDuplicateAndBindingIssues() {
    const std::array<UiFileNodeRecord, 4U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U),
        Node(2U, 1U, 1U),
        Node(3U, 99U, 2U)};
    const std::array<UiFileLayoutRecord, 1U> layouts{
        Layout(44U, UiLayoutContainerType::Grid)};
    const std::array<UiFileStyleRef, 1U> style_refs{
        StyleRef(55U, 71U)};
    const std::array<UiFileResourceRef, 1U> resource_refs{
        ResourceRef(66U, UiFileResourceKind::Sprite, 81U)};
    const std::array<UiFileEventBinding, 2U> event_bindings{
        EventBinding(77U, 91U, 92U),
        EventBinding(2U, 0U, 93U)};

    UiFileSchemaDesc desc{};
    desc.header = Header(9002U, 9U);
    desc.nodes = nodes;
    desc.layouts = layouts;
    desc.style_refs = style_refs;
    desc.resource_refs = resource_refs;
    desc.event_bindings = event_bindings;

    std::array<UiFileSchemaIssueRecord, 8U> issues{};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    const UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::IssuesFound || result.Succeeded()) {
        return Fail("ui file schema issues were not reported");
    }

    if (result.issue_count != 8U) {
        return Fail("ui file schema issue count mismatch");
    }

    if (issues[0U].issue_kind != UiFileSchemaIssueKind::MissingRootNode ||
        issues[0U].node_id.value != 9U ||
        issues[0U].context_key != 9002U) {
        return Fail("missing root issue mismatch");
    }

    if (issues[1U].issue_kind != UiFileSchemaIssueKind::DuplicateNodeId ||
        issues[1U].node_id.value != 2U ||
        issues[1U].duplicate_count != 2U) {
        return Fail("duplicate node issue mismatch");
    }

    if (issues[2U].issue_kind != UiFileSchemaIssueKind::MissingParentNode ||
        issues[2U].node_id.value != 3U ||
        issues[2U].context_key != 99U) {
        return Fail("missing parent issue mismatch");
    }

    if (issues[3U].issue_kind != UiFileSchemaIssueKind::MissingLayoutContainerNode ||
        issues[3U].node_id.value != 44U) {
        return Fail("missing layout issue mismatch");
    }

    if (issues[4U].issue_kind != UiFileSchemaIssueKind::MissingStyleRefNode ||
        issues[4U].node_id.value != 55U ||
        issues[4U].context_key != 71U) {
        return Fail("missing style issue mismatch");
    }

    if (issues[5U].issue_kind != UiFileSchemaIssueKind::MissingResourceRefNode ||
        issues[5U].node_id.value != 66U ||
        issues[5U].context_key != 81U) {
        return Fail("missing resource issue mismatch");
    }

    if (issues[6U].issue_kind != UiFileSchemaIssueKind::MissingEventBindingNode ||
        issues[6U].node_id.value != 77U ||
        issues[6U].context_key != 91U) {
        return Fail("missing event node issue mismatch");
    }

    if (issues[7U].issue_kind != UiFileSchemaIssueKind::MissingEventBindingKey ||
        issues[7U].node_id.value != 2U ||
        issues[7U].context_key != 0U) {
        return Fail("missing event key issue mismatch");
    }

    return 0;
}

int UiCoreUiFileSchemaRejectsInvalidHeaderWithoutMutation() {
    const std::array<UiFileNodeRecord, 1U> nodes{RootNode(1U)};
    UiFileSchemaDesc desc{};
    desc.header = Header(9003U, 1U);
    desc.header.schema_version = 99U;
    desc.nodes = nodes;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    const UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::InvalidHeader) {
        return Fail("invalid header was not rejected");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("invalid header mutated issue buffer");
    }

    return 0;
}

int UiCoreUiFileSchemaRejectsInvalidRecordsWithoutMutation() {
    std::array<UiFileNodeRecord, 1U> nodes{RootNode(1U)};
    nodes[0U].rect_transform.dpi_scale = 0.0F;
    UiFileSchemaDesc desc{};
    desc.header = Header(9004U, 1U);
    desc.nodes = nodes;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::InvalidNodeRecord) {
        return Fail("invalid node record was not rejected");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("invalid node record mutated issue buffer");
    }

    nodes[0U].rect_transform = DefaultTransform();
    const std::array<UiFileResourceRef, 1U> resource_refs{
        ResourceRef(1U, UiFileResourceKind::Invalid, 41U)};
    desc.resource_refs = resource_refs;
    issues[0U] = SentinelIssue();
    result = UiFileSchemaValidationResult{};
    status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::InvalidResourceRef) {
        return Fail("invalid resource ref was not rejected");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("invalid resource ref mutated issue buffer");
    }

    return 0;
}

int UiCoreUiFileSchemaRejectsSmallIssueOutputWithoutMutation() {
    const std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(1U, 99U, 0U)};
    UiFileSchemaDesc desc{};
    desc.header = Header(9005U, 8U);
    desc.nodes = nodes;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    const UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::OutputCapacityExceeded) {
        return Fail("small issue output was not rejected");
    }

    if (result.issue_count != 3U) {
        return Fail("small issue output count mismatch");
    }

    if (result.required_issue_count != 3U ||
        result.capacity_entry_issue_capacity != 1U ||
        result.capacity_entry_current_issue_count != 1U ||
        result.capacity_entry_required_issue_count != 3U ||
        result.failed_issue_kind != UiFileSchemaIssueKind::DuplicateNodeId ||
        result.failed_node_id.value != 1U ||
        result.failed_context_key != 1U ||
        result.failed_record_index != 0U ||
        result.failed_duplicate_count != 2U) {
        return Fail("small issue output capacity entry mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("small issue output mutated issue buffer");
    }

    return 0;
}

int UiCoreUiFileSchemaOutputCapacityReportsFirstUnreportedIssue() {
    const std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(1U, 99U, 0U)};
    UiFileSchemaDesc desc{};
    desc.header = Header(9006U, 8U);
    desc.nodes = nodes;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiFileSchemaValidationResult result{};
    const UiFileSchemaValidator validator{};
    UiFileSchemaStatus status = validator.Validate(desc, issues, &result);
    if (status != UiFileSchemaStatus::OutputCapacityExceeded) {
        return Fail("capacity entry output was not rejected");
    }

    if (result.issue_count != 3U || result.required_issue_count != 3U) {
        return Fail("capacity entry required issue count mismatch");
    }

    if (result.capacity_entry_issue_capacity != 1U ||
        result.capacity_entry_current_issue_count != 1U ||
        result.capacity_entry_required_issue_count != 3U) {
        return Fail("capacity entry issue count mismatch");
    }

    if (result.failed_issue_kind != UiFileSchemaIssueKind::DuplicateNodeId ||
        result.failed_record_index != 0U ||
        result.failed_node_id.value != 1U ||
        result.failed_context_key != 1U ||
        result.failed_duplicate_count != 2U) {
        return Fail("capacity entry first unreported issue mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("capacity entry mutated issue buffer");
    }

    UiFileSchemaDesc invalid_desc = desc;
    invalid_desc.header.schema_version = 99U;
    issues[0U] = SentinelIssue();
    status = validator.Validate(invalid_desc, issues, &result);
    if (status != UiFileSchemaStatus::InvalidHeader) {
        return Fail("capacity entry invalid header was not rejected");
    }

    if (!CapacityEntryIsClear(result)) {
        return Fail("capacity entry invalid header was stale");
    }

    const std::array<UiFileNodeRecord, 1U> valid_nodes{RootNode(1U)};
    UiFileSchemaDesc valid_desc{};
    valid_desc.header = Header(9007U, 1U);
    valid_desc.nodes = valid_nodes;
    issues[0U] = SentinelIssue();
    status = validator.Validate(valid_desc, issues, &result);
    if (status != UiFileSchemaStatus::Success) {
        return Fail("capacity entry valid schema did not pass");
    }

    if (!CapacityEntryIsClear(result)) {
        return Fail("capacity entry success was stale");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("capacity entry cleanup mutated issue buffer");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_VALID) {
        return UiCoreUiFileSchemaValidatesGenericRuntimeData();
    }

    if (name == TEST_ISSUES) {
        return UiCoreUiFileSchemaReportsMissingDuplicateAndBindingIssues();
    }

    if (name == TEST_INVALID_HEADER) {
        return UiCoreUiFileSchemaRejectsInvalidHeaderWithoutMutation();
    }

    if (name == TEST_INVALID_RECORDS) {
        return UiCoreUiFileSchemaRejectsInvalidRecordsWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreUiFileSchemaRejectsSmallIssueOutputWithoutMutation();
    }

    if (name == TEST_CAPACITY_ENTRY) {
        return UiCoreUiFileSchemaOutputCapacityReportsFirstUnreportedIssue();
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
