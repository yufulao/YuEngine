// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorIdEventValidatorTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiEditor/UiEditorIdEventValidator.h"

using yuengine::uieditor::UiEditorEventBinding;
using yuengine::uieditor::UiEditorIdEventIssueKind;
using yuengine::uieditor::UiEditorIdEventValidationRecord;
using yuengine::uieditor::UiEditorIdEventValidationResult;
using yuengine::uieditor::UiEditorIdEventValidationStatus;
using yuengine::uieditor::UiEditorIdEventValidator;
using yuengine::uieditor::UiEditorIdValidationNode;

namespace {
constexpr const char *TEST_REPORTS_ISSUES =
    "UiEditor_IdEventValidator_ReportsDuplicateIdsAndMissingEventNames";
constexpr const char *TEST_ACCEPTS_CLEAN =
    "UiEditor_IdEventValidator_AcceptsUniqueIdsAndNamedEvents";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiEditor_IdEventValidator_RejectsSmallReportOutputWithoutMutation";
constexpr const char *TEST_INVALID_NODE =
    "UiEditor_IdEventValidator_RejectsInvalidNodeWithoutMutation";
constexpr const char *TEST_INVALID_EVENT =
    "UiEditor_IdEventValidator_RejectsInvalidEventWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;
constexpr std::uint32_t SENTINEL_CONTEXT_ID = 888U;
constexpr std::uint32_t SENTINEL_DUPLICATE_COUNT = 9U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiEditorIdValidationNode MakeNode(std::uint32_t node_id) {
    UiEditorIdValidationNode node{};
    node.node_id = node_id;
    return node;
}

UiEditorEventBinding MakeEvent(
    std::uint32_t node_id,
    std::uint32_t event_id,
    std::string_view event_name) {
    UiEditorEventBinding event_binding{};
    event_binding.node_id = node_id;
    event_binding.event_id = event_id;
    event_binding.event_name = event_name;
    return event_binding;
}

UiEditorIdEventValidationRecord SentinelReport() {
    UiEditorIdEventValidationRecord record{};
    record.issue_kind = UiEditorIdEventIssueKind::MissingEventName;
    record.node_id = SENTINEL_NODE_ID;
    record.context_id = SENTINEL_CONTEXT_ID;
    record.duplicate_count = SENTINEL_DUPLICATE_COUNT;
    return record;
}

bool ReportMatchesSentinel(const UiEditorIdEventValidationRecord &record) {
    if (record.issue_kind != UiEditorIdEventIssueKind::MissingEventName) {
        return false;
    }

    if (record.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.context_id != SENTINEL_CONTEXT_ID) {
        return false;
    }

    return record.duplicate_count == SENTINEL_DUPLICATE_COUNT;
}

int UiEditorIdEventValidatorReportsDuplicateIdsAndMissingEventNames() {
    const std::array<UiEditorIdValidationNode, 5U> nodes{
        MakeNode(1U),
        MakeNode(2U),
        MakeNode(1U),
        MakeNode(3U),
        MakeNode(2U)};
    const std::array<UiEditorEventBinding, 4U> events{
        MakeEvent(1U, 101U, "OnOpen"),
        MakeEvent(2U, 102U, ""),
        MakeEvent(3U, 103U, "OnClose"),
        MakeEvent(2U, 104U, "")};

    std::array<UiEditorIdEventValidationRecord, 4U> reports{};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::IssuesFound || result.Succeeded()) {
        return Fail("id event validator did not report issues");
    }

    if (result.checked_node_count != 5U ||
        result.checked_event_count != 4U ||
        result.duplicate_node_id_count != 2U ||
        result.missing_event_name_count != 2U ||
        result.report_count != 4U) {
        return Fail("id event issue counts mismatch");
    }

    if (reports[0U].issue_kind != UiEditorIdEventIssueKind::DuplicateNodeId ||
        reports[0U].node_id != 1U ||
        reports[0U].context_id != 1U ||
        reports[0U].duplicate_count != 2U) {
        return Fail("first duplicate id report mismatch");
    }

    if (reports[1U].issue_kind != UiEditorIdEventIssueKind::DuplicateNodeId ||
        reports[1U].node_id != 2U ||
        reports[1U].context_id != 2U ||
        reports[1U].duplicate_count != 2U) {
        return Fail("second duplicate id report mismatch");
    }

    if (reports[2U].issue_kind != UiEditorIdEventIssueKind::MissingEventName ||
        reports[2U].node_id != 2U ||
        reports[2U].context_id != 102U ||
        reports[2U].duplicate_count != 0U) {
        return Fail("first missing event report mismatch");
    }

    if (reports[3U].issue_kind != UiEditorIdEventIssueKind::MissingEventName ||
        reports[3U].node_id != 2U ||
        reports[3U].context_id != 104U ||
        reports[3U].duplicate_count != 0U) {
        return Fail("second missing event report mismatch");
    }

    return 0;
}

int UiEditorIdEventValidatorAcceptsUniqueIdsAndNamedEvents() {
    const std::array<UiEditorIdValidationNode, 3U> nodes{
        MakeNode(10U),
        MakeNode(11U),
        MakeNode(12U)};
    const std::array<UiEditorEventBinding, 2U> events{
        MakeEvent(10U, 201U, "OnClick"),
        MakeEvent(11U, 202U, "OnHover")};

    std::array<UiEditorIdEventValidationRecord, 1U> reports{SentinelReport()};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::Success || !result.Succeeded()) {
        return Fail("clean id event data did not validate");
    }

    if (result.duplicate_node_id_count != 0U ||
        result.missing_event_name_count != 0U ||
        result.report_count != 0U) {
        return Fail("clean id event data produced reports");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("clean id event validation mutated reports");
    }

    return 0;
}

int UiEditorIdEventValidatorRejectsSmallReportOutputWithoutMutation() {
    const std::array<UiEditorIdValidationNode, 2U> nodes{
        MakeNode(21U),
        MakeNode(21U)};
    const std::array<UiEditorEventBinding, 1U> events{
        MakeEvent(21U, 301U, "")};

    std::array<UiEditorIdEventValidationRecord, 1U> reports{SentinelReport()};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::OutputCapacityExceeded) {
        return Fail("small id event output was not rejected");
    }

    if (result.duplicate_node_id_count != 1U ||
        result.missing_event_name_count != 1U ||
        result.report_count != 2U) {
        return Fail("small id event output did not report required count");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("small id event output mutated report storage");
    }

    return 0;
}

int UiEditorIdEventValidatorRejectsInvalidNodeWithoutMutation() {
    const std::array<UiEditorIdValidationNode, 1U> nodes{MakeNode(0U)};
    const std::array<UiEditorEventBinding, 1U> events{
        MakeEvent(1U, 401U, "OnSubmit")};

    std::array<UiEditorIdEventValidationRecord, 1U> reports{SentinelReport()};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::InvalidNode) {
        return Fail("invalid node was not rejected");
    }

    if (result.failed_node_index != 0U || result.failed_node_id != 0U) {
        return Fail("invalid node failure location mismatch");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("invalid node mutated report storage");
    }

    return 0;
}

int UiEditorIdEventValidatorRejectsInvalidEventWithoutMutation() {
    const std::array<UiEditorIdValidationNode, 1U> nodes{MakeNode(31U)};
    const std::array<UiEditorEventBinding, 1U> events{
        MakeEvent(0U, 501U, "OnConfirm")};

    std::array<UiEditorIdEventValidationRecord, 1U> reports{SentinelReport()};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::InvalidEvent) {
        return Fail("invalid event was not rejected");
    }

    if (result.failed_event_index != 0U || result.failed_node_id != 0U) {
        return Fail("invalid event failure location mismatch");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("invalid event mutated report storage");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_REPORTS_ISSUES) {
        return UiEditorIdEventValidatorReportsDuplicateIdsAndMissingEventNames();
    }

    if (name == TEST_ACCEPTS_CLEAN) {
        return UiEditorIdEventValidatorAcceptsUniqueIdsAndNamedEvents();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiEditorIdEventValidatorRejectsSmallReportOutputWithoutMutation();
    }

    if (name == TEST_INVALID_NODE) {
        return UiEditorIdEventValidatorRejectsInvalidNodeWithoutMutation();
    }

    if (name == TEST_INVALID_EVENT) {
        return UiEditorIdEventValidatorRejectsInvalidEventWithoutMutation();
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
