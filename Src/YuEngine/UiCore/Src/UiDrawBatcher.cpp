// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiDrawBatcher.cpp

#include "YuEngine/UiCore/UiDrawBatcher.h"

#include <cstddef>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
namespace {
bool IsKnownElementType(UiDrawElementType type) {
    if (type == UiDrawElementType::Rect) {
        return true;
    }

    if (type == UiDrawElementType::TexturedQuad) {
        return true;
    }

    return type == UiDrawElementType::Text;
}

bool IsElementSpanStorageValid(std::span<const UiDrawElement> elements) {
    if (elements.size() == 0U) {
        return true;
    }

    return elements.data() != nullptr;
}

bool IsBatchOutputStorageValid(std::span<UiDrawBatch> out_batches, std::uint32_t batch_count) {
    if (batch_count == 0U) {
        return true;
    }

    if (out_batches.size() < static_cast<std::size_t>(batch_count)) {
        return false;
    }

    return out_batches.data() != nullptr;
}
}

UiDrawBatchStatus UiDrawBatcher::Build(
    std::span<const UiDrawElement> elements,
    std::span<UiDrawBatch> out_batches,
    UiDrawBatchResult *out_result) const {
    snapshot_.draw_element_count = static_cast<std::uint32_t>(elements.size());
    snapshot_.batch_count = 0U;
    if (out_result == nullptr) {
        return RecordFailure(UiDrawBatchStatus::InvalidOutputBuffer);
    }

    *out_result = UiDrawBatchResult{};
    out_result->draw_element_count = static_cast<std::uint32_t>(elements.size());
    const UiDrawBatchStatus validate_status = ValidateElements(elements, out_result);
    if (validate_status != UiDrawBatchStatus::Success) {
        out_result->status = validate_status;
        return RecordFailure(validate_status);
    }

    out_result->failed_element_index = 0U;
    out_result->failed_node_id = UiNodeId{};
    const std::uint32_t batch_count = CountBatches(elements);
    out_result->batch_count = batch_count;
    snapshot_.batch_count = batch_count;
    if (!IsBatchOutputStorageValid(out_batches, batch_count)) {
        if (out_batches.size() < static_cast<std::size_t>(batch_count)) {
            RecordOutputCapacityFailure(elements, out_batches.size(), batch_count, out_result);
        }

        if (out_batches.size() >= static_cast<std::size_t>(batch_count)) {
            RecordFailure(UiDrawBatchStatus::OutputCapacityExceeded);
        }

        out_result->status = UiDrawBatchStatus::OutputCapacityExceeded;
        return UiDrawBatchStatus::OutputCapacityExceeded;
    }

    WriteBatches(elements, out_batches);
    out_result->status = UiDrawBatchStatus::Success;
    return RecordSuccess();
}

UiDrawBatcherSnapshot UiDrawBatcher::Snapshot() const {
    return snapshot_;
}

UiDrawBatchStatus UiDrawBatcher::RecordFailure(UiDrawBatchStatus status) const {
    ClearOutputCapacityEntry();
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

UiDrawBatchStatus UiDrawBatcher::RecordSuccess() const {
    ClearOutputCapacityEntry();
    ++snapshot_.accepted_operation_count;
    snapshot_.last_status = UiDrawBatchStatus::Success;
    return UiDrawBatchStatus::Success;
}

void UiDrawBatcher::ClearOutputCapacityEntry() const {
    snapshot_.last_capacity_entry_output_index = 0U;
    snapshot_.last_capacity_entry_node_id = UiNodeId{};
    snapshot_.last_capacity_entry_key = UiDrawBatchKey{};
    snapshot_.last_capacity_entry_output_capacity = 0U;
    snapshot_.last_capacity_entry_written_batch_count = 0U;
    snapshot_.last_required_output_batch_count = 0U;
}

UiDrawBatchStatus UiDrawBatcher::ValidateElements(
    std::span<const UiDrawElement> elements,
    UiDrawBatchResult *out_result) const {
    if (out_result == nullptr) {
        return UiDrawBatchStatus::InvalidOutputBuffer;
    }

    if (elements.size() > static_cast<std::size_t>(MAX_UI_NODE_COUNT)) {
        return UiDrawBatchStatus::OutputCapacityExceeded;
    }

    if (!IsElementSpanStorageValid(elements)) {
        return UiDrawBatchStatus::InvalidOutputBuffer;
    }

    for (std::size_t index = 0U; index < elements.size(); ++index) {
        const UiDrawBatchStatus status = ValidateElement(
            elements[index],
            static_cast<std::uint32_t>(index),
            out_result);
        if (status != UiDrawBatchStatus::Success) {
            return status;
        }
    }

    return UiDrawBatchStatus::Success;
}

UiDrawBatchStatus UiDrawBatcher::ValidateElement(
    const UiDrawElement &element,
    std::uint32_t index,
    UiDrawBatchResult *out_result) const {
    if (out_result == nullptr) {
        return UiDrawBatchStatus::InvalidOutputBuffer;
    }

    out_result->failed_element_index = index;
    out_result->failed_node_id = element.node_id;
    if (element.node_id.value == INVALID_UI_NODE_ID_VALUE) {
        return UiDrawBatchStatus::InvalidDrawElement;
    }

    if (!IsKnownElementType(element.type)) {
        return UiDrawBatchStatus::InvalidDrawElement;
    }

    return UiDrawBatchStatus::Success;
}

std::uint32_t UiDrawBatcher::CountBatches(std::span<const UiDrawElement> elements) const {
    std::uint32_t batch_count = 0U;
    UiDrawBatchKey last_key{};
    bool has_last_key = false;

    for (const UiDrawElement &element : elements) {
        const UiDrawBatchKey key = BuildKey(element);
        if (!has_last_key) {
            last_key = key;
            has_last_key = true;
            ++batch_count;
            continue;
        }

        if (KeyMatches(last_key, key)) {
            continue;
        }

        last_key = key;
        ++batch_count;
    }

    return batch_count;
}

void UiDrawBatcher::RecordOutputCapacityFailure(
    std::span<const UiDrawElement> elements,
    std::size_t output_batch_capacity,
    std::uint32_t required_batch_count,
    UiDrawBatchResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    const std::uint32_t output_capacity = static_cast<std::uint32_t>(output_batch_capacity);
    out_result->capacity_entry_output_capacity = output_capacity;
    out_result->capacity_entry_written_batch_count = output_capacity;
    out_result->required_output_batch_count = required_batch_count;
    snapshot_.last_capacity_entry_output_capacity = output_capacity;
    snapshot_.last_capacity_entry_written_batch_count = output_capacity;
    snapshot_.last_required_output_batch_count = required_batch_count;
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = UiDrawBatchStatus::OutputCapacityExceeded;

    if (elements.size() == 0U) {
        return;
    }

    UiDrawBatchKey last_key = BuildKey(elements[0U]);
    if (output_batch_capacity == 0U) {
        out_result->failed_batch_element_index = 0U;
        out_result->failed_batch_node_id = elements[0U].node_id;
        out_result->failed_batch_key = last_key;
        out_result->capacity_entry_output_index = 0U;
        snapshot_.last_capacity_entry_output_index = 0U;
        snapshot_.last_capacity_entry_node_id = elements[0U].node_id;
        snapshot_.last_capacity_entry_key = last_key;
        return;
    }

    std::uint32_t batch_index = 0U;
    for (std::size_t index = 1U; index < elements.size(); ++index) {
        const UiDrawBatchKey key = BuildKey(elements[index]);
        if (KeyMatches(last_key, key)) {
            continue;
        }

        ++batch_index;
        if (static_cast<std::size_t>(batch_index) >= output_batch_capacity) {
            out_result->failed_batch_element_index = static_cast<std::uint32_t>(index);
            out_result->failed_batch_node_id = elements[index].node_id;
            out_result->failed_batch_key = key;
            out_result->capacity_entry_output_index = batch_index;
            snapshot_.last_capacity_entry_output_index = batch_index;
            snapshot_.last_capacity_entry_node_id = elements[index].node_id;
            snapshot_.last_capacity_entry_key = key;
            return;
        }

        last_key = key;
    }
}

void UiDrawBatcher::WriteBatches(
    std::span<const UiDrawElement> elements,
    std::span<UiDrawBatch> out_batches) const {
    if (elements.size() == 0U) {
        return;
    }

    std::uint32_t batch_index = 0U;
    out_batches[batch_index].key = BuildKey(elements[0U]);
    out_batches[batch_index].first_element_index = 0U;
    out_batches[batch_index].element_count = 1U;

    for (std::size_t index = 1U; index < elements.size(); ++index) {
        const UiDrawBatchKey key = BuildKey(elements[index]);
        UiDrawBatch &current_batch = out_batches[batch_index];
        if (KeyMatches(current_batch.key, key)) {
            ++current_batch.element_count;
            continue;
        }

        ++batch_index;
        out_batches[batch_index].key = key;
        out_batches[batch_index].first_element_index = static_cast<std::uint32_t>(index);
        out_batches[batch_index].element_count = 1U;
    }
}

UiDrawBatchKey UiDrawBatcher::BuildKey(const UiDrawElement &element) const {
    UiDrawBatchKey key{};
    key.type = element.type;
    key.clip_rect = element.clip_rect;
    key.layer = element.layer;
    key.style_key = element.style_key;
    key.material_key = element.material_key;
    key.texture_key = element.texture_key;
    key.text_key = element.text_key;
    key.scissor_enabled = element.scissor_enabled;
    return key;
}

bool UiDrawBatcher::KeyMatches(const UiDrawBatchKey &left, const UiDrawBatchKey &right) const {
    if (left.type != right.type) {
        return false;
    }

    if (left.layer != right.layer) {
        return false;
    }

    if (left.style_key != right.style_key) {
        return false;
    }

    if (left.material_key != right.material_key) {
        return false;
    }

    if (left.texture_key != right.texture_key) {
        return false;
    }

    if (left.text_key != right.text_key) {
        return false;
    }

    if (left.scissor_enabled != right.scissor_enabled) {
        return false;
    }

    return RectMatches(left.clip_rect, right.clip_rect);
}

bool UiDrawBatcher::RectMatches(const UiRect &left, const UiRect &right) const {
    if (left.x != right.x) {
        return false;
    }

    if (left.y != right.y) {
        return false;
    }

    if (left.width != right.width) {
        return false;
    }

    return left.height == right.height;
}
}
