// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiDynamicAtlasPacker.cpp

#include "YuEngine/UiCore/UiDynamicAtlasPacker.h"

#include <array>
#include <cstddef>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
namespace {
struct UiDynamicAtlasPageCursor final {
    std::uint32_t next_x = 0U;
    std::uint32_t next_y = 0U;
    std::uint32_t row_height = 0U;
    bool used = false;
};

bool IsPageValid(const UiStaticAtlasPageDesc &page) {
    if (page.page_key == 0U || page.texture_key == 0U) {
        return false;
    }

    if (page.width == 0U || page.height == 0U) {
        return false;
    }

    return true;
}

bool IsRequestNineSliceValid(const UiDynamicAtlasSpriteRequest &request) {
    if (!request.nine_slice_enabled) {
        return true;
    }

    if (request.border_left > request.width) {
        return false;
    }

    const std::uint32_t remaining_width = request.width - request.border_left;
    if (request.border_right > remaining_width) {
        return false;
    }

    if (request.border_top > request.height) {
        return false;
    }

    const std::uint32_t remaining_height = request.height - request.border_top;
    if (request.border_bottom > remaining_height) {
        return false;
    }

    return true;
}

bool TryAddPadding(std::uint32_t value, std::uint32_t padding, std::uint32_t limit, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (padding == 0U) {
        *out_value = value;
        return true;
    }

    if (value > limit) {
        return false;
    }

    const std::uint32_t remaining = limit - value;
    if (padding > remaining) {
        return false;
    }

    *out_value = value + padding;
    return true;
}

bool TryWriteAtCursor(
    const UiStaticAtlasPageDesc &page,
    const UiDynamicAtlasSpriteRequest &request,
    std::uint32_t padding,
    UiDynamicAtlasPageCursor *cursor,
    UiStaticAtlasSpriteDesc *out_sprite) {
    if (cursor == nullptr || out_sprite == nullptr) {
        return false;
    }

    if (request.width > page.width || request.height > page.height) {
        return false;
    }

    std::uint32_t target_x = cursor->next_x;
    if (cursor->next_x > 0U) {
        if (!TryAddPadding(cursor->next_x, padding, page.width, &target_x)) {
            return false;
        }
    }

    if (target_x > page.width - request.width) {
        return false;
    }

    if (cursor->next_y > page.height - request.height) {
        return false;
    }

    out_sprite->sprite_key = request.sprite_key;
    out_sprite->page_key = page.page_key;
    out_sprite->x = target_x;
    out_sprite->y = cursor->next_y;
    out_sprite->width = request.width;
    out_sprite->height = request.height;
    out_sprite->nine_slice_enabled = request.nine_slice_enabled;
    out_sprite->border_left = request.border_left;
    out_sprite->border_top = request.border_top;
    out_sprite->border_right = request.border_right;
    out_sprite->border_bottom = request.border_bottom;

    cursor->next_x = target_x + request.width;
    if (request.height > cursor->row_height) {
        cursor->row_height = request.height;
    }

    cursor->used = true;
    return true;
}

bool TryPackInPage(
    const UiStaticAtlasPageDesc &page,
    const UiDynamicAtlasSpriteRequest &request,
    std::uint32_t padding,
    UiDynamicAtlasPageCursor *cursor,
    UiStaticAtlasSpriteDesc *out_sprite) {
    UiDynamicAtlasPageCursor current = *cursor;
    if (TryWriteAtCursor(page, request, padding, &current, out_sprite)) {
        *cursor = current;
        return true;
    }

    if (cursor->row_height == 0U) {
        return false;
    }

    const std::uint32_t row_bottom = cursor->next_y + cursor->row_height;
    if (row_bottom > page.height) {
        return false;
    }

    UiDynamicAtlasPageCursor next_row;
    next_row.next_x = 0U;
    next_row.row_height = 0U;
    if (!TryAddPadding(row_bottom, padding, page.height, &next_row.next_y)) {
        return false;
    }

    if (TryWriteAtCursor(page, request, padding, &next_row, out_sprite)) {
        *cursor = next_row;
        return true;
    }

    return false;
}

std::uint32_t CountUsedPages(std::span<const UiDynamicAtlasPageCursor> cursors) {
    std::uint32_t used_page_count = 0U;
    for (const UiDynamicAtlasPageCursor &cursor : cursors) {
        if (cursor.used) {
            ++used_page_count;
        }
    }

    return used_page_count;
}

void SetResult(
    UiDynamicAtlasPackResult *out_result,
    UiDynamicAtlasStatus status,
    std::uint32_t required_allocation_count) {
    out_result->status = status;
    out_result->required_allocation_count = required_allocation_count;
}

bool IsSpriteRequestValid(const UiDynamicAtlasSpriteRequest &request) {
    if (request.sprite_key == 0U) {
        return false;
    }

    if (request.width == 0U || request.height == 0U) {
        return false;
    }

    return IsRequestNineSliceValid(request);
}

std::uint32_t FindFailedSpriteKey(
    std::span<const UiDynamicAtlasSpriteRequest> requests,
    UiDynamicAtlasStatus status) {
    if (status == UiDynamicAtlasStatus::InvalidSpriteRequest) {
        for (const UiDynamicAtlasSpriteRequest &request : requests) {
            if (!IsSpriteRequestValid(request)) {
                return request.sprite_key;
            }
        }
    }

    if (status == UiDynamicAtlasStatus::DuplicateSprite) {
        const std::size_t request_count = requests.size();
        for (std::size_t left_index = 0U; left_index < request_count; ++left_index) {
            for (std::size_t right_index = left_index + 1U; right_index < request_count; ++right_index) {
                if (requests[left_index].sprite_key == requests[right_index].sprite_key) {
                    return requests[right_index].sprite_key;
                }
            }
        }
    }

    return 0U;
}
}

UiDynamicAtlasStatus UiDynamicAtlasPacker::Pack(
    const UiDynamicAtlasPackDesc &desc,
    std::span<const UiDynamicAtlasSpriteRequest> requests,
    std::span<UiStaticAtlasSpriteDesc> out_sprites,
    UiDynamicAtlasPackResult *out_result) const {
    if (out_result == nullptr) {
        return UiDynamicAtlasStatus::InvalidOutputBuffer;
    }

    *out_result = UiDynamicAtlasPackResult{};
    out_result->request_count = static_cast<std::uint32_t>(requests.size());
    out_result->required_allocation_count = out_result->request_count;

    UiDynamicAtlasStatus status = ValidateDesc(desc);
    if (status != UiDynamicAtlasStatus::Success) {
        SetResult(out_result, status, out_result->required_allocation_count);
        return status;
    }

    status = ValidateRequests(requests);
    if (status != UiDynamicAtlasStatus::Success) {
        out_result->failed_sprite_key = FindFailedSpriteKey(requests, status);
        SetResult(out_result, status, out_result->required_allocation_count);
        return status;
    }

    if (out_sprites.size() < requests.size()) {
        SetResult(out_result, UiDynamicAtlasStatus::OutputCapacityExceeded, out_result->required_allocation_count);
        return UiDynamicAtlasStatus::OutputCapacityExceeded;
    }

    if ((requests.size() > 0U) && (out_sprites.data() == nullptr)) {
        SetResult(out_result, UiDynamicAtlasStatus::InvalidOutputBuffer, out_result->required_allocation_count);
        return UiDynamicAtlasStatus::InvalidOutputBuffer;
    }

    std::array<UiDynamicAtlasPageCursor, MAX_UI_NODE_COUNT> cursors{};
    std::array<UiStaticAtlasSpriteDesc, MAX_UI_NODE_COUNT> packed_sprites{};
    std::uint32_t allocation_count = 0U;
    for (const UiDynamicAtlasSpriteRequest &request : requests) {
        if (!RequestFitsAnyPage(desc, request)) {
            out_result->failed_sprite_key = request.sprite_key;
            SetResult(out_result, UiDynamicAtlasStatus::SpriteTooLarge, out_result->required_allocation_count);
            return UiDynamicAtlasStatus::SpriteTooLarge;
        }

        bool packed = false;
        UiStaticAtlasSpriteDesc packed_sprite;
        for (std::size_t page_index = 0U; page_index < desc.pages.size(); ++page_index) {
            if (TryPackInPage(desc.pages[page_index], request, desc.padding, &cursors[page_index], &packed_sprite)) {
                packed = true;
                break;
            }
        }

        if (!packed) {
            out_result->failed_sprite_key = request.sprite_key;
            SetResult(out_result, UiDynamicAtlasStatus::AtlasCapacityExceeded, out_result->required_allocation_count);
            return UiDynamicAtlasStatus::AtlasCapacityExceeded;
        }

        packed_sprites[allocation_count] = packed_sprite;
        ++allocation_count;
    }

    for (std::uint32_t index = 0U; index < allocation_count; ++index) {
        out_sprites[index] = packed_sprites[index];
    }

    const std::span<const UiDynamicAtlasPageCursor> used_cursor_span(cursors.data(), desc.pages.size());
    out_result->status = UiDynamicAtlasStatus::Success;
    out_result->allocation_count = allocation_count;
    out_result->used_page_count = CountUsedPages(used_cursor_span);
    return UiDynamicAtlasStatus::Success;
}

UiDynamicAtlasStatus UiDynamicAtlasPacker::ValidateDesc(const UiDynamicAtlasPackDesc &desc) const {
    if (desc.phase == UiDynamicAtlasPackPhase::PaintPath) {
        return UiDynamicAtlasStatus::InvalidPackPhase;
    }

    if (desc.pages.size() > static_cast<std::size_t>(MAX_UI_NODE_COUNT)) {
        return UiDynamicAtlasStatus::OutputCapacityExceeded;
    }

    for (const UiStaticAtlasPageDesc &page : desc.pages) {
        if (!IsPageValid(page)) {
            return UiDynamicAtlasStatus::InvalidAtlasPage;
        }
    }

    const std::size_t page_count = desc.pages.size();
    for (std::size_t left_index = 0U; left_index < page_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < page_count; ++right_index) {
            if (desc.pages[left_index].page_key == desc.pages[right_index].page_key) {
                return UiDynamicAtlasStatus::InvalidAtlasPage;
            }
        }
    }

    return UiDynamicAtlasStatus::Success;
}

UiDynamicAtlasStatus UiDynamicAtlasPacker::ValidateRequests(
    std::span<const UiDynamicAtlasSpriteRequest> requests) const {
    if (requests.size() > static_cast<std::size_t>(MAX_UI_NODE_COUNT)) {
        return UiDynamicAtlasStatus::OutputCapacityExceeded;
    }

    for (const UiDynamicAtlasSpriteRequest &request : requests) {
        const UiDynamicAtlasStatus status = ValidateRequest(request);
        if (status != UiDynamicAtlasStatus::Success) {
            return status;
        }
    }

    const std::size_t request_count = requests.size();
    for (std::size_t left_index = 0U; left_index < request_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < request_count; ++right_index) {
            if (requests[left_index].sprite_key == requests[right_index].sprite_key) {
                return UiDynamicAtlasStatus::DuplicateSprite;
            }
        }
    }

    return UiDynamicAtlasStatus::Success;
}

UiDynamicAtlasStatus UiDynamicAtlasPacker::ValidateRequest(const UiDynamicAtlasSpriteRequest &request) const {
    if (request.sprite_key == 0U) {
        return UiDynamicAtlasStatus::InvalidSpriteRequest;
    }

    if (request.width == 0U || request.height == 0U) {
        return UiDynamicAtlasStatus::InvalidSpriteRequest;
    }

    if (!IsRequestNineSliceValid(request)) {
        return UiDynamicAtlasStatus::InvalidSpriteRequest;
    }

    return UiDynamicAtlasStatus::Success;
}

bool UiDynamicAtlasPacker::RequestFitsAnyPage(
    const UiDynamicAtlasPackDesc &desc,
    const UiDynamicAtlasSpriteRequest &request) const {
    for (const UiStaticAtlasPageDesc &page : desc.pages) {
        if (request.width > page.width || request.height > page.height) {
            continue;
        }

        return true;
    }

    return false;
}
}
