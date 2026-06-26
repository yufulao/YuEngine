// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneRuntimeMaterialBuilder.cpp

#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"

#include <algorithm>
#include <cstddef>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::renderscene {
namespace {
bool IsPipelineHandleSet(yuengine::rhi::RhiPipelineHandle handle) {
    return handle.generation != 0U;
}

bool IsTextureHandleSet(yuengine::rhi::RhiTextureHandle handle) {
    return handle.generation != 0U;
}

bool IsSamplerHandleSet(yuengine::rhi::RhiSamplerHandle handle) {
    return handle.generation != 0U;
}

bool IsTextureSlotInRange(std::uint32_t slot) {
    return slot < yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS;
}

bool IsSamplerSlotInRange(std::uint32_t slot) {
    return slot < yuengine::rhi::MAX_RHI_SAMPLER_SLOTS;
}

bool IsBlendStateValid(const yuengine::rhi::RhiBlendStateDesc &blend_state) {
    if (blend_state.mode == yuengine::rhi::RhiBlendMode::Opaque) {
        return true;
    }

    return blend_state.mode == yuengine::rhi::RhiBlendMode::AlphaOver;
}
}

RenderSceneRuntimeMaterialStatus RenderSceneRuntimeMaterialBuilder::Build(
    const RenderSceneRuntimeMaterialRequest &request,
    RenderSceneRuntimeMaterialRecord *out_record) const {
    if (out_record == nullptr) {
        return RenderSceneRuntimeMaterialStatus::NullPointer;
    }

    const RenderSceneRuntimeMaterialStatus status = ValidateRequest(request);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        return status;
    }

    RenderSceneRuntimeMaterialRecord record{};
    record.material_asset = request.material_asset;
    record.material_id = request.material_id;
    record.pipeline = request.pipeline;
    record.blend_state = request.blend_state;
    for (const RenderSceneRuntimeMaterialTextureSlot &texture_slot : request.texture_slots) {
        InsertTextureSlotSorted(texture_slot, &record);
    }

    const std::span<const std::uint8_t> material_constants = request.material_constant_bytes;
    std::copy(
        material_constants.begin(),
        material_constants.end(),
        record.material_constant_bytes.begin());
    record.material_constant_byte_count = material_constants.size();
    record.is_resolved = true;
    *out_record = record;
    return RenderSceneRuntimeMaterialStatus::Success;
}

RenderSceneRuntimeMaterialStatus RenderSceneRuntimeMaterialBuilder::Validate(
    const RenderSceneRuntimeMaterialRecord &record) const {
    if (!record.is_resolved) {
        return RenderSceneRuntimeMaterialStatus::MissingMaterialRecord;
    }

    if (!record.material_asset.IsValid()) {
        return RenderSceneRuntimeMaterialStatus::InvalidMaterialAsset;
    }

    if (record.material_id == 0U) {
        return RenderSceneRuntimeMaterialStatus::InvalidMaterialId;
    }

    if (!IsPipelineHandleSet(record.pipeline)) {
        return RenderSceneRuntimeMaterialStatus::InvalidPipeline;
    }

    if (!IsBlendStateValid(record.blend_state)) {
        return RenderSceneRuntimeMaterialStatus::InvalidBlendState;
    }

    if (record.texture_slot_count < MIN_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RenderSceneRuntimeMaterialStatus::MissingTextureSlot;
    }

    if (record.texture_slot_count > MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RenderSceneRuntimeMaterialStatus::TextureSlotCapacityExceeded;
    }

    if (record.material_constant_byte_count > MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES) {
        return RenderSceneRuntimeMaterialStatus::MaterialConstantCapacityExceeded;
    }

    for (std::size_t index = 0U; index < record.texture_slot_count; ++index) {
        const RenderSceneRuntimeMaterialStatus status = ValidateTextureSlot(record.texture_slots[index]);
        if (status != RenderSceneRuntimeMaterialStatus::Success) {
            return status;
        }
    }

    for (std::size_t left = 0U; left < record.texture_slot_count; ++left) {
        const std::uint32_t slot = record.texture_slots[left].slot;
        for (std::size_t right = left + 1U; right < record.texture_slot_count; ++right) {
            if (record.texture_slots[right].slot == slot) {
                return RenderSceneRuntimeMaterialStatus::DuplicateTextureSlot;
            }
        }
    }

    return RenderSceneRuntimeMaterialStatus::Success;
}

RenderSceneRuntimeMaterialStatus RenderSceneRuntimeMaterialBuilder::ValidateRequest(
    const RenderSceneRuntimeMaterialRequest &request) const {
    if (!request.material_asset.IsValid()) {
        return RenderSceneRuntimeMaterialStatus::InvalidMaterialAsset;
    }

    if (request.material_id == 0U) {
        return RenderSceneRuntimeMaterialStatus::InvalidMaterialId;
    }

    if (!IsPipelineHandleSet(request.pipeline)) {
        return RenderSceneRuntimeMaterialStatus::InvalidPipeline;
    }

    if (!IsBlendStateValid(request.blend_state)) {
        return RenderSceneRuntimeMaterialStatus::InvalidBlendState;
    }

    if (request.texture_slots.size() < MIN_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RenderSceneRuntimeMaterialStatus::MissingTextureSlot;
    }

    if (request.texture_slots.size() > MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS) {
        return RenderSceneRuntimeMaterialStatus::TextureSlotCapacityExceeded;
    }

    if (request.material_constant_bytes.size() > MAX_RENDER_SCENE_RUNTIME_MATERIAL_CONSTANT_BYTES) {
        return RenderSceneRuntimeMaterialStatus::MaterialConstantCapacityExceeded;
    }

    if (request.material_constant_bytes.size() != 0U &&
        request.material_constant_bytes.data() == nullptr) {
        return RenderSceneRuntimeMaterialStatus::NullPointer;
    }

    for (std::size_t index = 0U; index < request.texture_slots.size(); ++index) {
        const RenderSceneRuntimeMaterialTextureSlot &texture_slot = request.texture_slots[index];
        const RenderSceneRuntimeMaterialStatus status = ValidateTextureSlot(texture_slot);
        if (status != RenderSceneRuntimeMaterialStatus::Success) {
            return status;
        }

        if (HasDuplicateTextureSlot(request, texture_slot.slot, index)) {
            return RenderSceneRuntimeMaterialStatus::DuplicateTextureSlot;
        }
    }

    return RenderSceneRuntimeMaterialStatus::Success;
}

RenderSceneRuntimeMaterialStatus RenderSceneRuntimeMaterialBuilder::ValidateTextureSlot(
    const RenderSceneRuntimeMaterialTextureSlot &texture_slot) const {
    if (!texture_slot.texture_asset.IsValid()) {
        return RenderSceneRuntimeMaterialStatus::InvalidTextureAsset;
    }

    if (!IsTextureHandleSet(texture_slot.sampled_texture.texture)) {
        return RenderSceneRuntimeMaterialStatus::InvalidTextureBinding;
    }

    if (!IsTextureSlotInRange(texture_slot.sampled_texture.slot)) {
        return RenderSceneRuntimeMaterialStatus::InvalidTextureBinding;
    }

    if (texture_slot.sampled_texture.slot != texture_slot.slot) {
        return RenderSceneRuntimeMaterialStatus::InvalidTextureBinding;
    }

    if (!IsSamplerHandleSet(texture_slot.sampler.sampler)) {
        return RenderSceneRuntimeMaterialStatus::InvalidSamplerBinding;
    }

    if (!IsSamplerSlotInRange(texture_slot.sampler.slot)) {
        return RenderSceneRuntimeMaterialStatus::InvalidSamplerBinding;
    }

    if (texture_slot.sampler.slot != texture_slot.slot) {
        return RenderSceneRuntimeMaterialStatus::InvalidSamplerBinding;
    }

    return RenderSceneRuntimeMaterialStatus::Success;
}

bool RenderSceneRuntimeMaterialBuilder::HasDuplicateTextureSlot(
    const RenderSceneRuntimeMaterialRequest &request,
    std::uint32_t slot,
    std::size_t current_index) const {
    for (std::size_t index = current_index + 1U; index < request.texture_slots.size(); ++index) {
        if (request.texture_slots[index].slot == slot) {
            return true;
        }
    }

    return false;
}

void RenderSceneRuntimeMaterialBuilder::InsertTextureSlotSorted(
    const RenderSceneRuntimeMaterialTextureSlot &texture_slot,
    RenderSceneRuntimeMaterialRecord *out_record) const {
    if (out_record == nullptr) {
        return;
    }

    std::size_t insert_index = out_record->texture_slot_count;
    while (insert_index > 0U) {
        const std::size_t previous_index = insert_index - 1U;
        if (out_record->texture_slots[previous_index].slot < texture_slot.slot) {
            break;
        }

        out_record->texture_slots[insert_index] = out_record->texture_slots[previous_index];
        insert_index = previous_index;
    }

    out_record->texture_slots[insert_index] = texture_slot;
    ++out_record->texture_slot_count;
}
}
