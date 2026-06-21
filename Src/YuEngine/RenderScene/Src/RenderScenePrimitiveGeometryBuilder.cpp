// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderScenePrimitiveGeometryBuilder.cpp

#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"

#include <cstdint>

#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"

namespace yuengine::renderscene {
namespace {
constexpr std::uint32_t MIN_SEGMENT_COUNT = 3U;
constexpr std::uint32_t MAX_SEGMENT_COUNT = 64U;
constexpr std::uint32_t CUBE_VERTEX_COUNT = 24U;
constexpr std::uint32_t CUBE_INDEX_COUNT = 36U;
constexpr std::uint32_t TRIANGLE_INDEX_STRIDE_BYTES = sizeof(std::uint16_t);

bool IsValidKind(RenderScenePrimitiveGeometryKind kind) {
    if (kind == RenderScenePrimitiveGeometryKind::Cube) {
        return true;
    }

    if (kind == RenderScenePrimitiveGeometryKind::Cylinder) {
        return true;
    }

    return kind == RenderScenePrimitiveGeometryKind::Cone;
}

bool IsValidBufferHandle(yuengine::rhi::RhiBufferHandle handle) {
    if (handle.slot == 0U) {
        return false;
    }

    return handle.generation != 0U;
}

bool HasEnoughBufferRange(const RenderScenePrimitiveGeometryRecord &record) {
    const std::size_t required_vertex_bytes = record.vertex_stride_bytes * record.vertex_count;
    if (record.draw.vertex_buffer.size_bytes < required_vertex_bytes) {
        return false;
    }

    const std::size_t required_index_bytes = record.index_stride_bytes * record.index_count;
    return record.draw.index_buffer.size_bytes >= required_index_bytes;
}
}

RenderScenePrimitiveGeometryStatus RenderScenePrimitiveGeometryBuilder::Build(
    const RenderScenePrimitiveGeometryRequest &request,
    RenderScenePrimitiveGeometryRecord *out_record) const {
    if (out_record == nullptr) {
        return RenderScenePrimitiveGeometryStatus::NullPointer;
    }

    const RenderScenePrimitiveGeometryStatus status = ValidateRequest(request);
    if (status != RenderScenePrimitiveGeometryStatus::Success) {
        return status;
    }

    RenderScenePrimitiveGeometryRecord record{};
    record.geometry_asset = request.geometry_asset;
    record.kind = request.kind;
    record.segment_count = request.segment_count;
    record.vertex_stride_bytes = request.vertex_buffer.stride_bytes;
    record.index_stride_bytes = TRIANGLE_INDEX_STRIDE_BYTES;
    record.draw.draw_id = request.draw_id;
    record.draw.pass_id = request.pass_id;
    record.draw.material_id = request.material_id;
    record.draw.vertex_buffer = request.vertex_buffer;
    record.draw.index_buffer = request.index_buffer;
    record.draw.draw.topology = yuengine::rhi::RhiPrimitiveTopology::TriangleList;
    FillCounts(request, &record);
    if (!HasEnoughBufferRange(record)) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    record.draw.draw.index_count = record.index_count;
    record.is_resolved = true;
    *out_record = record;
    return RenderScenePrimitiveGeometryStatus::Success;
}

RenderScenePrimitiveGeometryStatus RenderScenePrimitiveGeometryBuilder::Validate(
    const RenderScenePrimitiveGeometryRecord &record) const {
    if (!record.is_resolved) {
        return RenderScenePrimitiveGeometryStatus::MissingGeometryRecord;
    }

    if (!record.geometry_asset.IsValid()) {
        return RenderScenePrimitiveGeometryStatus::InvalidGeometryAsset;
    }

    if (!IsValidKind(record.kind)) {
        return RenderScenePrimitiveGeometryStatus::InvalidPrimitiveKind;
    }

    if (record.vertex_count == 0U || record.index_count == 0U) {
        return RenderScenePrimitiveGeometryStatus::MissingGeometryRecord;
    }

    if (record.draw.draw_id == 0U || record.draw.pass_id == 0U || record.draw.material_id == 0U) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (record.draw.draw.index_count != record.index_count) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (!HasEnoughBufferRange(record)) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    return RenderScenePrimitiveGeometryStatus::Success;
}

RenderScenePrimitiveGeometryStatus RenderScenePrimitiveGeometryBuilder::ValidateRequest(
    const RenderScenePrimitiveGeometryRequest &request) const {
    if (!request.geometry_asset.IsValid()) {
        return RenderScenePrimitiveGeometryStatus::InvalidGeometryAsset;
    }

    if (!IsValidKind(request.kind)) {
        return RenderScenePrimitiveGeometryStatus::InvalidPrimitiveKind;
    }

    if (request.kind != RenderScenePrimitiveGeometryKind::Cube) {
        if (request.segment_count < MIN_SEGMENT_COUNT || request.segment_count > MAX_SEGMENT_COUNT) {
            return RenderScenePrimitiveGeometryStatus::InvalidSegmentCount;
        }
    }

    if (request.draw_id == 0U || request.pass_id == 0U || request.material_id == 0U) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (!IsValidBufferHandle(request.vertex_buffer.buffer)) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (!IsValidBufferHandle(request.index_buffer.buffer)) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (request.vertex_buffer.stride_bytes == 0U || request.vertex_buffer.size_bytes == 0U) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (request.index_buffer.size_bytes == 0U) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    if (request.index_buffer.format != yuengine::rhi::RhiIndexFormat::Uint16) {
        return RenderScenePrimitiveGeometryStatus::InvalidDrawRecord;
    }

    return RenderScenePrimitiveGeometryStatus::Success;
}

void RenderScenePrimitiveGeometryBuilder::FillCounts(
    const RenderScenePrimitiveGeometryRequest &request,
    RenderScenePrimitiveGeometryRecord *out_record) const {
    if (out_record == nullptr) {
        return;
    }

    if (request.kind == RenderScenePrimitiveGeometryKind::Cube) {
        out_record->vertex_count = CUBE_VERTEX_COUNT;
        out_record->index_count = CUBE_INDEX_COUNT;
        out_record->segment_count = 0U;
        return;
    }

    if (request.kind == RenderScenePrimitiveGeometryKind::Cylinder) {
        out_record->vertex_count = (request.segment_count * 2U) + 2U;
        out_record->index_count = request.segment_count * 12U;
        return;
    }

    if (request.kind == RenderScenePrimitiveGeometryKind::Cone) {
        out_record->vertex_count = request.segment_count + 2U;
        out_record->index_count = request.segment_count * 6U;
    }
}
}
