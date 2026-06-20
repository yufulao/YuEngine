// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorResourceReferenceValidatorTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"
#include "YuEngine/UiEditor/UiEditorResourceReferenceValidator.h"

using yuengine::uicore::UiStaticAtlasMetadataDesc;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uieditor::UiEditorResourceCatalog;
using yuengine::uieditor::UiEditorResourceIssueKind;
using yuengine::uieditor::UiEditorResourceReference;
using yuengine::uieditor::UiEditorResourceReferenceKind;
using yuengine::uieditor::UiEditorResourceReferenceValidator;
using yuengine::uieditor::UiEditorResourceValidationRecord;
using yuengine::uieditor::UiEditorResourceValidationResult;
using yuengine::uieditor::UiEditorResourceValidationStatus;

namespace {
constexpr const char *TEST_REPORTS_MISSING =
    "UiEditor_ResourceValidator_ReportsMissingReferences";
constexpr const char *TEST_ACCEPTS_EXISTING =
    "UiEditor_ResourceValidator_AcceptsExistingReferences";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiEditor_ResourceValidator_RejectsSmallReportOutputWithoutMutation";
constexpr const char *TEST_INVALID_REFERENCE =
    "UiEditor_ResourceValidator_RejectsInvalidReferenceWithoutMutation";
constexpr const char *TEST_INVALID_ATLAS =
    "UiEditor_ResourceValidator_RejectsInvalidAtlasMetadata";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;
constexpr std::uint32_t SENTINEL_KEY = 888U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiStaticAtlasPageDesc MakeAtlasPage(
    std::uint32_t page_key,
    std::uint32_t texture_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasPageDesc desc{};
    desc.page_key = page_key;
    desc.texture_key = texture_key;
    desc.width = width;
    desc.height = height;
    return desc;
}

UiStaticAtlasSpriteDesc MakeAtlasSprite(
    std::uint32_t sprite_key,
    std::uint32_t page_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasSpriteDesc desc{};
    desc.sprite_key = sprite_key;
    desc.page_key = page_key;
    desc.width = width;
    desc.height = height;
    return desc;
}

UiEditorResourceReference MakeReference(
    std::uint32_t node_id,
    UiEditorResourceReferenceKind kind,
    std::uint32_t key) {
    UiEditorResourceReference reference{};
    reference.node_id = node_id;
    reference.kind = kind;
    reference.key = key;
    return reference;
}

UiEditorResourceValidationRecord SentinelReport() {
    UiEditorResourceValidationRecord record{};
    record.node_id = SENTINEL_NODE_ID;
    record.kind = UiEditorResourceReferenceKind::Style;
    record.key = SENTINEL_KEY;
    record.issue_kind = UiEditorResourceIssueKind::MissingStyle;
    return record;
}

bool ReportMatchesSentinel(const UiEditorResourceValidationRecord &record) {
    if (record.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.key != SENTINEL_KEY) {
        return false;
    }

    return record.issue_kind == UiEditorResourceIssueKind::MissingStyle;
}

UiEditorResourceCatalog MakeCatalog(
    const std::array<UiStaticAtlasPageDesc, 1U> &pages,
    const std::array<UiStaticAtlasSpriteDesc, 1U> &sprites,
    const std::array<std::uint32_t, 1U> &font_keys,
    const std::array<std::uint32_t, 1U> &style_keys,
    const std::array<std::uint32_t, 1U> &localization_keys) {
    UiEditorResourceCatalog catalog{};
    catalog.static_atlas = UiStaticAtlasMetadataDesc{pages, sprites};
    catalog.font_keys = font_keys;
    catalog.style_keys = style_keys;
    catalog.localization_keys = localization_keys;
    return catalog;
}

int UiEditorResourceValidatorReportsMissingReferences() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 32U, 32U)};
    const std::array<std::uint32_t, 1U> font_keys{31U};
    const std::array<std::uint32_t, 1U> style_keys{41U};
    const std::array<std::uint32_t, 1U> localization_keys{51U};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    const std::array<UiEditorResourceReference, 4U> references{
        MakeReference(1U, UiEditorResourceReferenceKind::Sprite, 99U),
        MakeReference(2U, UiEditorResourceReferenceKind::Font, 39U),
        MakeReference(3U, UiEditorResourceReferenceKind::Style, 49U),
        MakeReference(4U, UiEditorResourceReferenceKind::Localization, 59U)};

    std::array<UiEditorResourceValidationRecord, 4U> reports{};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(references, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::MissingReferences || result.Succeeded()) {
        return Fail("missing references did not produce explicit status");
    }

    if (result.checked_reference_count != 4U || result.missing_reference_count != 4U || result.report_count != 4U) {
        return Fail("missing reference counts mismatch");
    }

    if (reports[0U].issue_kind != UiEditorResourceIssueKind::MissingSprite || reports[0U].node_id != 1U) {
        return Fail("missing sprite report mismatch");
    }

    if (reports[1U].issue_kind != UiEditorResourceIssueKind::MissingFont || reports[1U].node_id != 2U) {
        return Fail("missing font report mismatch");
    }

    if (reports[2U].issue_kind != UiEditorResourceIssueKind::MissingStyle || reports[2U].node_id != 3U) {
        return Fail("missing style report mismatch");
    }

    if (reports[3U].issue_kind != UiEditorResourceIssueKind::MissingLocalization || reports[3U].node_id != 4U) {
        return Fail("missing localization report mismatch");
    }

    return 0;
}

int UiEditorResourceValidatorAcceptsExistingReferences() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 32U, 32U)};
    const std::array<std::uint32_t, 1U> font_keys{31U};
    const std::array<std::uint32_t, 1U> style_keys{41U};
    const std::array<std::uint32_t, 1U> localization_keys{51U};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    const std::array<UiEditorResourceReference, 4U> references{
        MakeReference(1U, UiEditorResourceReferenceKind::Sprite, 11U),
        MakeReference(2U, UiEditorResourceReferenceKind::Font, 31U),
        MakeReference(3U, UiEditorResourceReferenceKind::Style, 41U),
        MakeReference(4U, UiEditorResourceReferenceKind::Localization, 51U)};

    std::array<UiEditorResourceValidationRecord, 1U> reports{SentinelReport()};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(references, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::Success || !result.Succeeded()) {
        return Fail("existing references did not validate");
    }

    if (result.missing_reference_count != 0U || result.report_count != 0U) {
        return Fail("existing references produced reports");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("validator mutated report buffer for clean references");
    }

    return 0;
}

int UiEditorResourceValidatorRejectsSmallReportOutputWithoutMutation() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 32U, 32U)};
    const std::array<std::uint32_t, 1U> font_keys{31U};
    const std::array<std::uint32_t, 1U> style_keys{41U};
    const std::array<std::uint32_t, 1U> localization_keys{51U};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    const std::array<UiEditorResourceReference, 2U> references{
        MakeReference(1U, UiEditorResourceReferenceKind::Sprite, 99U),
        MakeReference(2U, UiEditorResourceReferenceKind::Font, 39U)};

    std::array<UiEditorResourceValidationRecord, 1U> reports{SentinelReport()};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(references, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::OutputCapacityExceeded) {
        return Fail("small report output was not rejected");
    }

    if (result.report_count != 2U || result.missing_reference_count != 2U) {
        return Fail("small output did not report required count");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("small output mutated report storage");
    }

    return 0;
}

int UiEditorResourceValidatorRejectsInvalidReferenceWithoutMutation() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 256U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 32U, 32U)};
    const std::array<std::uint32_t, 1U> font_keys{31U};
    const std::array<std::uint32_t, 1U> style_keys{41U};
    const std::array<std::uint32_t, 1U> localization_keys{51U};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    const std::array<UiEditorResourceReference, 1U> references{
        MakeReference(0U, UiEditorResourceReferenceKind::Sprite, 11U)};

    std::array<UiEditorResourceValidationRecord, 1U> reports{SentinelReport()};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(references, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::InvalidReference) {
        return Fail("invalid reference was not rejected");
    }

    if (result.failed_reference_index != 0U || result.failed_node_id != 0U) {
        return Fail("invalid reference failure location mismatch");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("invalid reference mutated report storage");
    }

    return 0;
}

int UiEditorResourceValidatorRejectsInvalidAtlasMetadata() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakeAtlasPage(7U, 77U, 0U, 128U)};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{
        MakeAtlasSprite(11U, 7U, 32U, 32U)};
    const std::array<std::uint32_t, 1U> font_keys{31U};
    const std::array<std::uint32_t, 1U> style_keys{41U};
    const std::array<std::uint32_t, 1U> localization_keys{51U};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    const std::array<UiEditorResourceReference, 1U> references{
        MakeReference(9U, UiEditorResourceReferenceKind::Sprite, 11U)};

    std::array<UiEditorResourceValidationRecord, 1U> reports{SentinelReport()};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(references, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::InvalidAtlasMetadata) {
        return Fail("invalid atlas metadata was not rejected");
    }

    if (result.failed_reference_index != 0U || result.failed_node_id != 9U) {
        return Fail("invalid atlas failure location mismatch");
    }

    if (!ReportMatchesSentinel(reports[0U])) {
        return Fail("invalid atlas mutated report storage");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_REPORTS_MISSING) {
        return UiEditorResourceValidatorReportsMissingReferences();
    }

    if (name == TEST_ACCEPTS_EXISTING) {
        return UiEditorResourceValidatorAcceptsExistingReferences();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiEditorResourceValidatorRejectsSmallReportOutputWithoutMutation();
    }

    if (name == TEST_INVALID_REFERENCE) {
        return UiEditorResourceValidatorRejectsInvalidReferenceWithoutMutation();
    }

    if (name == TEST_INVALID_ATLAS) {
        return UiEditorResourceValidatorRejectsInvalidAtlasMetadata();
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
