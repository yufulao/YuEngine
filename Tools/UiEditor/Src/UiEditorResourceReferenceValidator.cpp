// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorResourceReferenceValidator.cpp

#include "YuEngine/UiEditor/UiEditorResourceReferenceValidator.h"

#include <cstddef>

namespace yuengine::uieditor {
namespace {
bool IsReferenceStorageValid(std::span<const UiEditorResourceReference> references) {
    if (references.empty()) {
        return true;
    }

    return references.data() != nullptr;
}

bool IsReportStorageValid(
    std::span<UiEditorResourceValidationRecord> out_reports,
    std::uint32_t report_count) {
    if (report_count == 0U) {
        return true;
    }

    if (out_reports.size() < static_cast<std::size_t>(report_count)) {
        return false;
    }

    return out_reports.data() != nullptr;
}

UiEditorResourceIssueKind MissingIssueForKind(UiEditorResourceReferenceKind kind) {
    if (kind == UiEditorResourceReferenceKind::Sprite) {
        return UiEditorResourceIssueKind::MissingSprite;
    }

    if (kind == UiEditorResourceReferenceKind::Font) {
        return UiEditorResourceIssueKind::MissingFont;
    }

    if (kind == UiEditorResourceReferenceKind::Style) {
        return UiEditorResourceIssueKind::MissingStyle;
    }

    if (kind == UiEditorResourceReferenceKind::Localization) {
        return UiEditorResourceIssueKind::MissingLocalization;
    }

    return UiEditorResourceIssueKind::None;
}
}

UiEditorResourceValidationStatus UiEditorResourceReferenceValidator::Validate(
    std::span<const UiEditorResourceReference> references,
    const UiEditorResourceCatalog &catalog,
    std::span<UiEditorResourceValidationRecord> out_reports,
    UiEditorResourceValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorResourceValidationStatus::InvalidInput;
    }

    *out_result = UiEditorResourceValidationResult{};
    out_result->checked_reference_count = static_cast<std::uint32_t>(references.size());

    if (!IsReferenceStorageValid(references)) {
        out_result->status = UiEditorResourceValidationStatus::InvalidInput;
        return UiEditorResourceValidationStatus::InvalidInput;
    }

    std::uint32_t missing_count = 0U;
    for (std::size_t index = 0U; index < references.size(); ++index) {
        const UiEditorResourceReference &reference = references[index];
        const UiEditorResourceValidationStatus reference_status =
            ValidateReference(reference, static_cast<std::uint32_t>(index), out_result);
        if (reference_status != UiEditorResourceValidationStatus::Success) {
            out_result->status = reference_status;
            return reference_status;
        }

        UiEditorResourceIssueKind issue_kind = UiEditorResourceIssueKind::None;
        const UiEditorResourceValidationStatus issue_status = ResolveIssue(catalog, reference, &issue_kind);
        if (issue_status != UiEditorResourceValidationStatus::Success) {
            out_result->failed_reference_index = static_cast<std::uint32_t>(index);
            out_result->failed_node_id = reference.node_id;
            out_result->status = issue_status;
            return issue_status;
        }

        if (issue_kind != UiEditorResourceIssueKind::None) {
            ++missing_count;
        }
    }

    out_result->missing_reference_count = missing_count;
    out_result->report_count = missing_count;
    if (!IsReportStorageValid(out_reports, missing_count)) {
        out_result->status = UiEditorResourceValidationStatus::OutputCapacityExceeded;
        return UiEditorResourceValidationStatus::OutputCapacityExceeded;
    }

    std::uint32_t report_index = 0U;
    for (const UiEditorResourceReference &reference : references) {
        UiEditorResourceIssueKind issue_kind = UiEditorResourceIssueKind::None;
        const UiEditorResourceValidationStatus issue_status = ResolveIssue(catalog, reference, &issue_kind);
        if (issue_status != UiEditorResourceValidationStatus::Success) {
            out_result->status = issue_status;
            return issue_status;
        }

        if (issue_kind == UiEditorResourceIssueKind::None) {
            continue;
        }

        out_reports[report_index].node_id = reference.node_id;
        out_reports[report_index].kind = reference.kind;
        out_reports[report_index].key = reference.key;
        out_reports[report_index].issue_kind = issue_kind;
        ++report_index;
    }

    if (missing_count > 0U) {
        out_result->status = UiEditorResourceValidationStatus::MissingReferences;
        return UiEditorResourceValidationStatus::MissingReferences;
    }

    out_result->status = UiEditorResourceValidationStatus::Success;
    return UiEditorResourceValidationStatus::Success;
}

UiEditorResourceValidationStatus UiEditorResourceReferenceValidator::ValidateReference(
    const UiEditorResourceReference &reference,
    std::uint32_t reference_index,
    UiEditorResourceValidationResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorResourceValidationStatus::InvalidInput;
    }

    out_result->failed_reference_index = reference_index;
    out_result->failed_node_id = reference.node_id;
    if (reference.node_id == 0U) {
        return UiEditorResourceValidationStatus::InvalidReference;
    }

    if (reference.key == 0U) {
        return UiEditorResourceValidationStatus::InvalidReference;
    }

    if (!IsKnownKind(reference.kind)) {
        return UiEditorResourceValidationStatus::InvalidReference;
    }

    return UiEditorResourceValidationStatus::Success;
}

UiEditorResourceValidationStatus UiEditorResourceReferenceValidator::ResolveIssue(
    const UiEditorResourceCatalog &catalog,
    const UiEditorResourceReference &reference,
    UiEditorResourceIssueKind *out_issue_kind) const {
    if (out_issue_kind == nullptr) {
        return UiEditorResourceValidationStatus::InvalidInput;
    }

    *out_issue_kind = UiEditorResourceIssueKind::None;
    if (reference.kind == UiEditorResourceReferenceKind::Sprite) {
        const yuengine::uicore::UiStaticAtlasMetadata metadata;
        const yuengine::uicore::UiStaticAtlasResolveResult result =
            metadata.ResolveSprite(catalog.static_atlas, reference.key);
        if (result.status == yuengine::uicore::UiStaticAtlasStatus::Success) {
            return UiEditorResourceValidationStatus::Success;
        }

        if (result.status == yuengine::uicore::UiStaticAtlasStatus::SpriteNotFound) {
            *out_issue_kind = UiEditorResourceIssueKind::MissingSprite;
            return UiEditorResourceValidationStatus::Success;
        }

        return UiEditorResourceValidationStatus::InvalidAtlasMetadata;
    }

    if (reference.kind == UiEditorResourceReferenceKind::Font) {
        if (ContainsKey(catalog.font_keys, reference.key)) {
            return UiEditorResourceValidationStatus::Success;
        }

        *out_issue_kind = UiEditorResourceIssueKind::MissingFont;
        return UiEditorResourceValidationStatus::Success;
    }

    if (reference.kind == UiEditorResourceReferenceKind::Style) {
        if (ContainsKey(catalog.style_keys, reference.key)) {
            return UiEditorResourceValidationStatus::Success;
        }

        *out_issue_kind = UiEditorResourceIssueKind::MissingStyle;
        return UiEditorResourceValidationStatus::Success;
    }

    if (reference.kind == UiEditorResourceReferenceKind::Localization) {
        if (ContainsKey(catalog.localization_keys, reference.key)) {
            return UiEditorResourceValidationStatus::Success;
        }

        *out_issue_kind = UiEditorResourceIssueKind::MissingLocalization;
        return UiEditorResourceValidationStatus::Success;
    }

    *out_issue_kind = MissingIssueForKind(reference.kind);
    return UiEditorResourceValidationStatus::InvalidReference;
}

bool UiEditorResourceReferenceValidator::ContainsKey(
    std::span<const std::uint32_t> keys,
    std::uint32_t key) const {
    for (const std::uint32_t value : keys) {
        if (value == key) {
            return true;
        }
    }

    return false;
}

bool UiEditorResourceReferenceValidator::IsKnownKind(UiEditorResourceReferenceKind kind) const {
    if (kind == UiEditorResourceReferenceKind::Sprite) {
        return true;
    }

    if (kind == UiEditorResourceReferenceKind::Font) {
        return true;
    }

    if (kind == UiEditorResourceReferenceKind::Style) {
        return true;
    }

    return kind == UiEditorResourceReferenceKind::Localization;
}
}
