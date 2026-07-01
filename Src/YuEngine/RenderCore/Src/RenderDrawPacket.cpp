// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderDrawPacket.cpp

#include "YuEngine/RenderCore/RenderDrawPacket.h"

#include <cstddef>

#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::rendercore {
namespace {
RenderDrawPacketDesc NormalizeDesc(RenderDrawPacketDesc desc) {
    if (desc.draw_record_capacity > MAX_RENDER_DRAW_PACKET_RECORDS) {
        desc.draw_record_capacity = MAX_RENDER_DRAW_PACKET_RECORDS;
    }

    return desc;
}

bool IsBufferHandleSet(yuengine::rhi::RhiBufferHandle handle) {
    return handle.generation != 0U;
}

bool IsVertexBufferViewValid(const yuengine::rhi::RhiVertexBufferView &view) {
    if (!IsBufferHandleSet(view.buffer)) {
        return false;
    }

    if (view.stride_bytes == 0U) {
        return false;
    }

    return view.size_bytes != 0U;
}

std::size_t IndexSizeBytes(yuengine::rhi::RhiIndexFormat format) {
    if (format == yuengine::rhi::RhiIndexFormat::Uint16) {
        return sizeof(std::uint16_t);
    }

    if (format == yuengine::rhi::RhiIndexFormat::Uint32) {
        return sizeof(std::uint32_t);
    }

    return 0U;
}

bool IsIndexBufferViewValid(const yuengine::rhi::RhiIndexBufferView &view) {
    if (!IsBufferHandleSet(view.buffer)) {
        return false;
    }

    const std::size_t index_size = IndexSizeBytes(view.format);
    if (index_size == 0U) {
        return false;
    }

    return view.size_bytes >= index_size;
}

bool IsDrawRangeValid(
    const yuengine::rhi::RhiIndexBufferView &index_buffer,
    const yuengine::rhi::RhiDrawIndexedDesc &draw) {
    const std::size_t index_size = IndexSizeBytes(index_buffer.format);
    if (index_size == 0U) {
        return false;
    }

    const std::size_t available_count = index_buffer.size_bytes / index_size;
    if (draw.first_index >= available_count) {
        return false;
    }

    const std::size_t remaining_count = available_count - draw.first_index;
    return draw.index_count <= remaining_count;
}

bool IsDrawValid(
    const yuengine::rhi::RhiIndexBufferView &index_buffer,
    const yuengine::rhi::RhiDrawIndexedDesc &draw) {
    if (draw.topology == yuengine::rhi::RhiPrimitiveTopology::Unsupported) {
        return false;
    }

    if (draw.index_count == 0U) {
        return false;
    }

    return IsDrawRangeValid(index_buffer, draw);
}

void ClearDrawCapacityFailure(RenderDrawPacketSnapshot &snapshot) {
    snapshot.last_capacity_entry_draw_record_capacity = 0U;
    snapshot.last_capacity_entry_current_draw_record_count = 0U;
    snapshot.last_capacity_entry_required_draw_record_count = 0U;
    snapshot.last_capacity_entry_failed_entry_index = 0U;
    snapshot.last_capacity_entry_draw_id = 0U;
    snapshot.last_capacity_entry_pass_id = 0U;
    snapshot.last_capacity_entry_material_id = 0U;
    snapshot.last_capacity_entry_index_count = 0U;
    snapshot.last_capacity_entry_status = RenderDrawPacketStatus::Success;
    snapshot.last_failed_entry_index = 0U;
    snapshot.last_failed_draw_id = 0U;
    snapshot.last_failed_pass_id = 0U;
    snapshot.last_failed_material_id = 0U;
}

void RecordDrawCapacityFailure(
    RenderDrawPacketSnapshot &snapshot,
    const RenderDrawPacketResult &result) {
    snapshot.last_capacity_entry_draw_record_capacity =
        result.draw_record_capacity;
    snapshot.last_capacity_entry_current_draw_record_count =
        result.current_draw_record_count;
    snapshot.last_capacity_entry_required_draw_record_count =
        result.required_draw_record_count;
    snapshot.last_capacity_entry_failed_entry_index = result.failed_entry_index;
    snapshot.last_capacity_entry_draw_id = result.failed_draw_id;
    snapshot.last_capacity_entry_pass_id = result.failed_pass_id;
    snapshot.last_capacity_entry_material_id = result.failed_material_id;
    snapshot.last_capacity_entry_index_count = result.index_count;
    snapshot.last_capacity_entry_status = result.status;
    snapshot.last_failed_entry_index = result.failed_entry_index;
    snapshot.last_failed_draw_id = result.failed_draw_id;
    snapshot.last_failed_pass_id = result.failed_pass_id;
    snapshot.last_failed_material_id = result.failed_material_id;
}
}

RenderDrawPacket::RenderDrawPacket(const RenderDrawPacketDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderDrawPacketResult RenderDrawPacket::BuildPassRequest(
    const RenderDrawPacketRequest &request,
    RenderFixturePassRequest *out_request) {
    RenderDrawPacketResult result{};
    result.draw_id = request.draw_id;
    result.pass_id = request.pass_id;
    result.material_id = request.material_id;
    result.index_count = request.draw.index_count;

    if (out_request == nullptr) {
        result.status = RenderDrawPacketStatus::InvalidArgument;
        RecordRejectedDraw(result);
        return result;
    }

    result.status = ValidateRequest(request);
    if (result.status != RenderDrawPacketStatus::Success) {
        RecordRejectedDraw(result);
        return result;
    }

    if (HasDrawId(request.draw_id)) {
        result.status = RenderDrawPacketStatus::DuplicateDrawId;
        RecordRejectedDraw(result);
        return result;
    }

    result.draw_record_capacity = desc_.draw_record_capacity;
    result.current_draw_record_count = snapshot_.draw_record_count;
    result.required_draw_record_count = RequiredDrawRecordCount();
    if (!HasRecordCapacity()) {
        result.status = RenderDrawPacketStatus::DrawCapacityExceeded;
        result.failed_entry_index = snapshot_.draw_record_count;
        result.failed_draw_id = request.draw_id;
        result.failed_pass_id = request.pass_id;
        result.failed_material_id = request.material_id;
        RecordRejectedDraw(result);
        return result;
    }

    FillPassRequest(request, out_request);
    RecordAcceptedDraw(request, &result);
    return result;
}

RenderDrawPacketSnapshot RenderDrawPacket::Snapshot() const {
    return snapshot_;
}

void RenderDrawPacket::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.draw_record_capacity = desc_.draw_record_capacity;
    snapshot_.required_draw_record_count = 1U;
}

RenderDrawPacketStatus RenderDrawPacket::ValidateRequest(const RenderDrawPacketRequest &request) const {
    if (request.draw_id == 0U) {
        return RenderDrawPacketStatus::InvalidDrawId;
    }

    if (request.pass_id == 0U) {
        return RenderDrawPacketStatus::InvalidPassId;
    }

    if (request.material_id == 0U) {
        return RenderDrawPacketStatus::InvalidMaterialId;
    }

    if (!IsVertexBufferViewValid(request.vertex_buffer)) {
        return RenderDrawPacketStatus::MissingVertexBuffer;
    }

    if (!IsIndexBufferViewValid(request.index_buffer)) {
        return RenderDrawPacketStatus::MissingIndexBuffer;
    }

    if (!IsDrawValid(request.index_buffer, request.draw)) {
        return RenderDrawPacketStatus::InvalidDraw;
    }

    return RenderDrawPacketStatus::Success;
}

bool RenderDrawPacket::HasRecordCapacity() const {
    return snapshot_.draw_record_count < desc_.draw_record_capacity;
}

std::size_t RenderDrawPacket::RequiredDrawRecordCount() const {
    return snapshot_.draw_record_count + 1U;
}

bool RenderDrawPacket::HasDrawId(std::uint32_t draw_id) const {
    for (std::size_t index = 0U; index < snapshot_.draw_record_count; ++index) {
        if (records_[index].draw_id == draw_id) {
            return true;
        }
    }

    return false;
}

void RenderDrawPacket::FillPassRequest(
    const RenderDrawPacketRequest &request,
    RenderFixturePassRequest *out_request) const {
    if (out_request == nullptr) {
        return;
    }

    out_request->vertex_buffer = request.vertex_buffer;
    out_request->index_buffer = request.index_buffer;
    out_request->draw = request.draw;
    out_request->pass_id = request.pass_id;
    out_request->material_id = request.material_id;
}

void RenderDrawPacket::RecordAcceptedDraw(
    const RenderDrawPacketRequest &request,
    RenderDrawPacketResult *result) {
    if (result == nullptr) {
        return;
    }

    ClearDrawCapacityFailure(snapshot_);
    Record record{};
    record.draw_id = request.draw_id;
    record.pass_id = request.pass_id;
    record.material_id = request.material_id;
    record.vertex_buffer = request.vertex_buffer;
    record.index_buffer = request.index_buffer;
    record.draw = request.draw;

    if (snapshot_.draw_record_count < records_.size()) {
        records_[snapshot_.draw_record_count] = record;
    }

    ++snapshot_.draw_record_count;
    ++snapshot_.accepted_draw_count;
    snapshot_.required_draw_record_count = result->required_draw_record_count;
    snapshot_.last_draw_id = request.draw_id;
    snapshot_.last_pass_id = request.pass_id;
    snapshot_.last_material_id = request.material_id;
    snapshot_.last_index_count = request.draw.index_count;
    snapshot_.last_status = RenderDrawPacketStatus::Success;

    result->status = RenderDrawPacketStatus::Success;
}

void RenderDrawPacket::RecordRejectedDraw(const RenderDrawPacketResult &result) {
    ClearDrawCapacityFailure(snapshot_);
    snapshot_.last_draw_id = result.draw_id;
    snapshot_.last_pass_id = result.pass_id;
    snapshot_.last_material_id = result.material_id;
    snapshot_.last_index_count = result.index_count;
    if (result.required_draw_record_count > 0U) {
        snapshot_.required_draw_record_count = result.required_draw_record_count;
    }
    snapshot_.last_status = result.status;

    if (result.status == RenderDrawPacketStatus::DuplicateDrawId) {
        ++snapshot_.duplicate_draw_id_count;
        return;
    }

    if (result.status == RenderDrawPacketStatus::DrawCapacityExceeded) {
        RecordDrawCapacityFailure(snapshot_, result);
        ++snapshot_.draw_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}
}
