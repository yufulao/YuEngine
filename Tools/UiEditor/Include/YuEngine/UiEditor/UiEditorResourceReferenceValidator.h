// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Include/YuEngine/UiEditor/UiEditorResourceReferenceValidator.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uieditor {
enum class UiEditorResourceReferenceKind {
    Invalid = 0,
    Sprite,
    Font,
    Style,
    Localization
};

enum class UiEditorResourceIssueKind {
    None = 0,
    MissingSprite,
    MissingFont,
    MissingStyle,
    MissingLocalization
};

enum class UiEditorResourceValidationStatus {
    Success = 0,
    MissingReferences,
    InvalidInput,
    InvalidReference,
    InvalidAtlasMetadata,
    OutputCapacityExceeded
};

struct UiEditorResourceReference final {
    std::uint32_t node_id = 0U;
    UiEditorResourceReferenceKind kind = UiEditorResourceReferenceKind::Invalid;
    std::uint32_t key = 0U;
};

struct UiEditorResourceCatalog final {
    yuengine::uicore::UiStaticAtlasMetadataDesc static_atlas{};
    std::span<const std::uint32_t> font_keys{};
    std::span<const std::uint32_t> style_keys{};
    std::span<const std::uint32_t> localization_keys{};
};

struct UiEditorResourceValidationRecord final {
    std::uint32_t node_id = 0U;
    UiEditorResourceReferenceKind kind = UiEditorResourceReferenceKind::Invalid;
    std::uint32_t key = 0U;
    UiEditorResourceIssueKind issue_kind = UiEditorResourceIssueKind::None;
};

struct UiEditorResourceValidationResult final {
    UiEditorResourceValidationStatus status = UiEditorResourceValidationStatus::Success;
    std::uint32_t checked_reference_count = 0U;
    std::uint32_t missing_reference_count = 0U;
    std::uint32_t report_count = 0U;
    std::uint32_t failed_reference_index = 0U;
    std::uint32_t failed_node_id = 0U;

    /**
     * @comment 检查 resource reference validation 是否没有发现缺失或错误。
     * @return 没有缺失引用且校验成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiEditorResourceValidationStatus::Success;
    }
};

class UiEditorResourceReferenceValidator final {
public:
    /**
     * @comment 校验 editor layout resource references 并输出缺失引用报告。
     * @param references layout 中提取的 resource references。
     * @param catalog editor 已知 resource catalog。
     * @param out_reports 调用方持有的 report buffer。
     * @param out_result 输出 validation result。
     * @return 显式校验状态。
     */
    UiEditorResourceValidationStatus Validate(
        std::span<const UiEditorResourceReference> references,
        const UiEditorResourceCatalog &catalog,
        std::span<UiEditorResourceValidationRecord> out_reports,
        UiEditorResourceValidationResult *out_result) const;

private:
    UiEditorResourceValidationStatus ValidateReference(
        const UiEditorResourceReference &reference,
        std::uint32_t reference_index,
        UiEditorResourceValidationResult *out_result) const;
    UiEditorResourceValidationStatus ResolveIssue(
        const UiEditorResourceCatalog &catalog,
        const UiEditorResourceReference &reference,
        UiEditorResourceIssueKind *out_issue_kind) const;
    bool ContainsKey(std::span<const std::uint32_t> keys, std::uint32_t key) const;
    bool IsKnownKind(UiEditorResourceReferenceKind kind) const;
};
}
