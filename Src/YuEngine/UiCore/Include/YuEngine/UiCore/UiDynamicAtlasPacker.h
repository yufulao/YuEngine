// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDynamicAtlasPacker.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
enum class UiDynamicAtlasPackPhase {
    SafePoint,
    PaintPath
};

enum class UiDynamicAtlasStatus {
    Success,
    InvalidOutputBuffer,
    InvalidPackPhase,
    InvalidAtlasPage,
    InvalidSpriteRequest,
    DuplicateSprite,
    OutputCapacityExceeded,
    SpriteTooLarge,
    AtlasCapacityExceeded
};

struct UiDynamicAtlasPackDesc final {
    std::span<const UiStaticAtlasPageDesc> pages;
    UiDynamicAtlasPackPhase phase = UiDynamicAtlasPackPhase::SafePoint;
    std::uint32_t padding = 0U;
};

struct UiDynamicAtlasSpriteRequest final {
    std::uint32_t sprite_key = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    bool nine_slice_enabled = false;
    std::uint32_t border_left = 0U;
    std::uint32_t border_top = 0U;
    std::uint32_t border_right = 0U;
    std::uint32_t border_bottom = 0U;
};

struct UiDynamicAtlasPackResult final {
    UiDynamicAtlasStatus status = UiDynamicAtlasStatus::InvalidOutputBuffer;
    std::uint32_t request_count = 0U;
    std::uint32_t allocation_count = 0U;
    std::uint32_t required_allocation_count = 0U;
    std::uint32_t used_page_count = 0U;
    std::uint32_t failed_sprite_key = 0U;

    /**
     * @comment 检查 dynamic atlas pack 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiDynamicAtlasStatus::Success;
    }
};

class UiDynamicAtlasPacker final {
public:
    /**
     * @comment 在 safe point 执行动态 atlas metadata pack。
     * @param desc pack 描述。
     * @param requests sprite pack 请求。
     * @param out_sprites 调用方持有的 sprite metadata 输出 buffer。
     * @param out_result 输出 pack result。
     * @return 显式 pack 状态。
     */
    UiDynamicAtlasStatus Pack(
        const UiDynamicAtlasPackDesc &desc,
        std::span<const UiDynamicAtlasSpriteRequest> requests,
        std::span<UiStaticAtlasSpriteDesc> out_sprites,
        UiDynamicAtlasPackResult *out_result) const;

private:
    UiDynamicAtlasStatus ValidateDesc(const UiDynamicAtlasPackDesc &desc) const;
    UiDynamicAtlasStatus ValidateRequests(std::span<const UiDynamicAtlasSpriteRequest> requests) const;
    UiDynamicAtlasStatus ValidateRequest(const UiDynamicAtlasSpriteRequest &request) const;
    bool RequestFitsAnyPage(
        const UiDynamicAtlasPackDesc &desc,
        const UiDynamicAtlasSpriteRequest &request) const;
};
}
