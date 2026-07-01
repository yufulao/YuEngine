// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiNodeTree.cpp

#include "YuEngine/UiCore/UiNodeTree.h"

#include "YuEngine/UiCore/UiRectMath.h"

namespace yuengine::uicore {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

UiNodeTree::UiNodeTree()
    : UiNodeTree(UiNodeTreeDesc{}) {
}

UiNodeTree::UiNodeTree(UiNodeTreeDesc desc)
    : records_{},
      viewport_rect_(desc.viewport_rect),
      snapshot_{
          ClampCapacity(desc.node_capacity, MAX_UI_NODE_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          UiNodeTreeStatus::Success} {
    if (snapshot_.node_capacity == 0U) {
        snapshot_.last_status = UiNodeTreeStatus::InvalidCapacity;
    }
}

UiNodeTreeResult UiNodeTree::CreateNode(const UiNodeDesc &desc) {
    UiNodeRecord new_record;
    const UiNodeTreeStatus validate_status = ValidateDesc(desc, new_record);
    if (validate_status != UiNodeTreeStatus::Success) {
        if (validate_status == UiNodeTreeStatus::CapacityExceeded) {
            return RecordNodeCapacityFailure(desc);
        }

        return UiNodeTreeResult::Failure(RecordFailure(validate_status));
    }

    UiNodeRecord *free_record = FindFreeRecord();
    if (free_record == nullptr) {
        return RecordNodeCapacityFailure(desc);
    }

    *free_record = new_record;
    ++snapshot_.active_node_count;
    ++snapshot_.created_node_count;
    RecordSuccess();
    return UiNodeTreeResult::Success(*free_record);
}

UiNodeTreeResult UiNodeTree::DestroyNode(UiNodeId node_id) {
    UiNodeRecord *record = FindRecord(node_id);
    if (record == nullptr) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::NodeNotFound));
    }

    const std::uint32_t destroyed_count = DestroyRecordAndDescendants(*record);
    snapshot_.destroyed_node_count += destroyed_count;
    RecountActiveNodes();
    RecordSuccess();
    return UiNodeTreeResult::Success(UiNodeRecord{});
}

UiNodeTreeResult UiNodeTree::AttachNode(UiNodeId node_id, UiNodeId parent_id, std::uint32_t sibling_order) {
    if (!parent_id.IsValid()) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::ParentNotFound));
    }

    UiNodeRecord *record = FindRecord(node_id);
    if (record == nullptr) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::NodeNotFound));
    }

    const UiNodeRecord *parent_record = FindRecord(parent_id);
    if (parent_record == nullptr) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::ParentNotFound));
    }

    if (node_id.value == parent_id.value) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::SelfParent));
    }

    if (WouldCreateCycle(node_id, parent_id)) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::CycleRejected));
    }

    record->parent_id = parent_id;
    record->sibling_order = sibling_order;
    const UiNodeTreeStatus rect_status = ResolveRecordInPlace(*record);
    if (rect_status != UiNodeTreeStatus::Success) {
        return UiNodeTreeResult::Failure(RecordFailure(rect_status));
    }

    const UiNodeTreeStatus descendant_status = ResolveDescendants(node_id);
    if (descendant_status != UiNodeTreeStatus::Success) {
        return UiNodeTreeResult::Failure(RecordFailure(descendant_status));
    }

    RecordSuccess();
    return UiNodeTreeResult::Success(*record);
}

UiNodeTreeResult UiNodeTree::DetachNode(UiNodeId node_id, std::uint32_t sibling_order) {
    UiNodeRecord *record = FindRecord(node_id);
    if (record == nullptr) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::NodeNotFound));
    }

    record->parent_id = UiNodeId{};
    record->sibling_order = sibling_order;
    const UiNodeTreeStatus rect_status = ResolveRecordInPlace(*record);
    if (rect_status != UiNodeTreeStatus::Success) {
        return UiNodeTreeResult::Failure(RecordFailure(rect_status));
    }

    const UiNodeTreeStatus descendant_status = ResolveDescendants(node_id);
    if (descendant_status != UiNodeTreeStatus::Success) {
        return UiNodeTreeResult::Failure(RecordFailure(descendant_status));
    }

    RecordSuccess();
    return UiNodeTreeResult::Success(*record);
}

UiNodeTreeResult UiNodeTree::SetNodeRect(UiNodeId node_id, const UiRectTransform &rect_transform) {
    UiNodeRecord *record = FindRecord(node_id);
    if (record == nullptr) {
        return UiNodeTreeResult::Failure(RecordFailure(UiNodeTreeStatus::NodeNotFound));
    }

    const UiRectTransform previous_transform = record->rect_transform;
    record->rect_transform = rect_transform;
    const UiNodeTreeStatus rect_status = ResolveRecordInPlace(*record);
    if (rect_status != UiNodeTreeStatus::Success) {
        record->rect_transform = previous_transform;
        const UiNodeTreeStatus rollback_status = ResolveRecordInPlace(*record);
        (void)rollback_status;
        return UiNodeTreeResult::Failure(RecordFailure(rect_status));
    }

    const UiNodeTreeStatus descendant_status = ResolveDescendants(node_id);
    if (descendant_status != UiNodeTreeStatus::Success) {
        return UiNodeTreeResult::Failure(RecordFailure(descendant_status));
    }

    RecordSuccess();
    return UiNodeTreeResult::Success(*record);
}

UiNodeTreeStatus UiNodeTree::SetViewportRect(UiRect viewport_rect) {
    if ((viewport_rect.width < 0.0F) || (viewport_rect.height < 0.0F)) {
        return RecordFailure(UiNodeTreeStatus::InvalidRect);
    }

    viewport_rect_ = viewport_rect;
    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        UiNodeRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.parent_id.IsValid()) {
            continue;
        }

        const UiNodeTreeStatus rect_status = ResolveRecordInPlace(record);
        if (rect_status != UiNodeTreeStatus::Success) {
            return RecordFailure(rect_status);
        }

        const UiNodeTreeStatus descendant_status = ResolveDescendants(record.node_id);
        if (descendant_status != UiNodeTreeStatus::Success) {
            return RecordFailure(descendant_status);
        }
    }

    RecordSuccess();
    return UiNodeTreeStatus::Success;
}

UiNodeTreeResult UiNodeTree::QueryNode(UiNodeId node_id) const {
    const UiNodeRecord *record = FindRecord(node_id);
    if (record == nullptr) {
        return UiNodeTreeResult::Failure(UiNodeTreeStatus::NodeNotFound);
    }

    return UiNodeTreeResult::Success(*record);
}

std::uint32_t UiNodeTree::ExportChildren(
    UiNodeId parent_id,
    UiNodeRecord *output_records,
    std::uint32_t output_capacity) const {
    std::array<bool, MAX_UI_NODE_COUNT> exported_flags{};
    std::uint32_t exported_count = 0U;

    while (true) {
        bool found_child = false;
        std::uint32_t best_index = 0U;
        UiNodeRecord best_record;

        for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
            const UiNodeRecord &record = records_[index];
            if (exported_flags[index]) {
                continue;
            }

            if (!ChildMatchesParent(record, parent_id)) {
                continue;
            }

            const bool is_better_order = record.sibling_order < best_record.sibling_order;
            const bool is_same_order = record.sibling_order == best_record.sibling_order;
            const bool is_better_id = record.node_id.value < best_record.node_id.value;
            if (!found_child || is_better_order || (is_same_order && is_better_id)) {
                found_child = true;
                best_index = index;
                best_record = record;
            }
        }

        if (!found_child) {
            break;
        }

        if ((output_records != nullptr) && (exported_count < output_capacity)) {
            output_records[exported_count] = best_record;
        }

        exported_flags[best_index] = true;
        ++exported_count;
    }

    return exported_count;
}

UiNodeTreeStatus UiNodeTree::ExportChildrenChecked(
    UiNodeId parent_id,
    UiNodeRecord *output_records,
    std::uint32_t output_capacity,
    UiNodeTreeExportChildrenResult *out_result) const {
    if (out_result == nullptr) {
        return UiNodeTreeStatus::InvalidCapacity;
    }

    UiNodeTreeExportChildrenResult result{};
    *out_result = result;
    if (output_capacity > 0U && output_records == nullptr) {
        result.status = UiNodeTreeStatus::InvalidCapacity;
        *out_result = result;
        return result.status;
    }

    if (parent_id.IsValid() && FindRecord(parent_id) == nullptr) {
        result.status = UiNodeTreeStatus::NodeNotFound;
        *out_result = result;
        return result.status;
    }

    const std::uint32_t required_child_count = ExportChildren(parent_id, nullptr, 0U);
    result.required_child_count = required_child_count;
    if (output_capacity < required_child_count) {
        result.status = UiNodeTreeStatus::CapacityExceeded;
        *out_result = result;
        return result.status;
    }

    const std::uint32_t copied_child_count =
        ExportChildren(parent_id, output_records, output_capacity);
    result.status = UiNodeTreeStatus::Success;
    result.copied_child_count = copied_child_count;
    result.required_child_count = required_child_count;
    *out_result = result;
    return result.status;
}

UiNodeTreeSnapshot UiNodeTree::Snapshot() const {
    return snapshot_;
}

UiNodeTreeStatus UiNodeTree::RecordFailure(UiNodeTreeStatus status) {
    ClearNodeCapacityEntry();
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void UiNodeTree::RecordSuccess() {
    ++snapshot_.accepted_operation_count;
    ClearNodeCapacityEntry();
    snapshot_.last_status = UiNodeTreeStatus::Success;
}

UiNodeTreeResult UiNodeTree::RecordNodeCapacityFailure(const UiNodeDesc &desc) {
    const std::uint32_t required_node_count = snapshot_.active_node_count + 1U;
    snapshot_.last_required_node_count = required_node_count;
    snapshot_.last_node_capacity_entry_node_id = desc.node_id;
    snapshot_.last_node_capacity_entry_parent_id = desc.parent_id;
    snapshot_.last_node_capacity_entry_sibling_order = desc.sibling_order;
    snapshot_.last_node_capacity_entry_capacity = snapshot_.node_capacity;
    snapshot_.last_node_capacity_entry_active_count = snapshot_.active_node_count;
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = UiNodeTreeStatus::CapacityExceeded;

    UiNodeTreeResult result = UiNodeTreeResult::Failure(UiNodeTreeStatus::CapacityExceeded);
    result.capacity_entry_node_id = desc.node_id;
    result.capacity_entry_parent_id = desc.parent_id;
    result.capacity_entry_sibling_order = desc.sibling_order;
    result.capacity_entry_node_capacity = snapshot_.node_capacity;
    result.capacity_entry_active_node_count = snapshot_.active_node_count;
    result.required_node_count = required_node_count;
    return result;
}

void UiNodeTree::ClearNodeCapacityEntry() {
    snapshot_.last_required_node_count = 0U;
    snapshot_.last_node_capacity_entry_node_id = UiNodeId{};
    snapshot_.last_node_capacity_entry_parent_id = UiNodeId{};
    snapshot_.last_node_capacity_entry_sibling_order = 0U;
    snapshot_.last_node_capacity_entry_capacity = 0U;
    snapshot_.last_node_capacity_entry_active_count = 0U;
}

UiNodeTreeStatus UiNodeTree::ValidateDesc(const UiNodeDesc &desc, UiNodeRecord &out_record) const {
    if (snapshot_.node_capacity == 0U) {
        return UiNodeTreeStatus::InvalidCapacity;
    }

    if (!desc.node_id.IsValid()) {
        return UiNodeTreeStatus::InvalidNodeId;
    }

    if (FindRecord(desc.node_id) != nullptr) {
        return UiNodeTreeStatus::DuplicateNodeId;
    }

    if (snapshot_.active_node_count >= snapshot_.node_capacity) {
        return UiNodeTreeStatus::CapacityExceeded;
    }

    return ResolveRecord(desc, out_record);
}

UiNodeTreeStatus UiNodeTree::ResolveRecord(const UiNodeDesc &desc, UiNodeRecord &out_record) const {
    UiRect parent_rect = viewport_rect_;
    if (desc.parent_id.IsValid()) {
        const UiNodeRecord *parent_record = FindRecord(desc.parent_id);
        if (parent_record == nullptr) {
            return UiNodeTreeStatus::ParentNotFound;
        }

        parent_rect = parent_record->world_rect;
    }

    const UiRectMathResult rect_result = UiRectMath::Resolve(parent_rect, desc.rect_transform);
    if (!rect_result.Succeeded()) {
        return UiNodeTreeStatus::InvalidRect;
    }

    out_record = UiNodeRecord{};
    out_record.node_id = desc.node_id;
    out_record.parent_id = desc.parent_id;
    out_record.rect_transform = desc.rect_transform;
    out_record.world_rect = rect_result.rect;
    out_record.content_rect = rect_result.content_rect;
    out_record.pivot_point = rect_result.pivot_point;
    out_record.sibling_order = desc.sibling_order;
    out_record.layer = desc.layer;
    out_record.is_visible = desc.is_visible;
    out_record.is_enabled = desc.is_enabled;
    out_record.is_hit_testable = desc.is_hit_testable;
    out_record.is_active = true;
    return UiNodeTreeStatus::Success;
}

UiNodeTreeStatus UiNodeTree::ResolveRecordInPlace(UiNodeRecord &record) {
    UiNodeDesc desc;
    desc.node_id = record.node_id;
    desc.parent_id = record.parent_id;
    desc.rect_transform = record.rect_transform;
    desc.sibling_order = record.sibling_order;
    desc.layer = record.layer;
    desc.is_visible = record.is_visible;
    desc.is_enabled = record.is_enabled;
    desc.is_hit_testable = record.is_hit_testable;

    UiNodeRecord resolved_record;
    const UiNodeTreeStatus status = ResolveRecord(desc, resolved_record);
    if (status != UiNodeTreeStatus::Success) {
        return status;
    }

    record = resolved_record;
    return UiNodeTreeStatus::Success;
}

UiNodeTreeStatus UiNodeTree::ResolveDescendants(UiNodeId parent_id) {
    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        UiNodeRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.parent_id.value != parent_id.value) {
            continue;
        }

        const UiNodeTreeStatus rect_status = ResolveRecordInPlace(record);
        if (rect_status != UiNodeTreeStatus::Success) {
            return rect_status;
        }

        const UiNodeTreeStatus descendant_status = ResolveDescendants(record.node_id);
        if (descendant_status != UiNodeTreeStatus::Success) {
            return descendant_status;
        }
    }

    return UiNodeTreeStatus::Success;
}

UiNodeRecord *UiNodeTree::FindRecord(UiNodeId node_id) {
    if (!node_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        UiNodeRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.node_id.value == node_id.value) {
            return &record;
        }
    }

    return nullptr;
}

const UiNodeRecord *UiNodeTree::FindRecord(UiNodeId node_id) const {
    if (!node_id.IsValid()) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        const UiNodeRecord &record = records_[index];
        if (!record.is_active) {
            continue;
        }

        if (record.node_id.value == node_id.value) {
            return &record;
        }
    }

    return nullptr;
}

UiNodeRecord *UiNodeTree::FindFreeRecord() {
    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        UiNodeRecord &record = records_[index];
        if (record.is_active) {
            continue;
        }

        return &record;
    }

    return nullptr;
}

bool UiNodeTree::WouldCreateCycle(UiNodeId node_id, UiNodeId parent_id) const {
    UiNodeId current_parent_id = parent_id;
    while (current_parent_id.IsValid()) {
        if (current_parent_id.value == node_id.value) {
            return true;
        }

        const UiNodeRecord *parent_record = FindRecord(current_parent_id);
        if (parent_record == nullptr) {
            return false;
        }

        current_parent_id = parent_record->parent_id;
    }

    return false;
}

bool UiNodeTree::ChildMatchesParent(const UiNodeRecord &record, UiNodeId parent_id) const {
    if (!record.is_active) {
        return false;
    }

    if (!parent_id.IsValid()) {
        return !record.parent_id.IsValid();
    }

    return record.parent_id.value == parent_id.value;
}

std::uint32_t UiNodeTree::DestroyRecordAndDescendants(UiNodeRecord &record) {
    const UiNodeId parent_id = record.node_id;
    std::uint32_t destroyed_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        UiNodeRecord &child_record = records_[index];
        if (!child_record.is_active) {
            continue;
        }

        if (child_record.parent_id.value != parent_id.value) {
            continue;
        }

        destroyed_count += DestroyRecordAndDescendants(child_record);
    }

    record = UiNodeRecord{};
    return destroyed_count + 1U;
}

void UiNodeTree::RecountActiveNodes() {
    std::uint32_t active_node_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.node_capacity; ++index) {
        if (!records_[index].is_active) {
            continue;
        }

        ++active_node_count;
    }

    snapshot_.active_node_count = active_node_count;
}
}
