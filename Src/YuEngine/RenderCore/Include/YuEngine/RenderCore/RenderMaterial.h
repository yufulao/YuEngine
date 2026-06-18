// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterial.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/RenderMaterialConstants.h"
#include "YuEngine/RenderCore/RenderMaterialDesc.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/RenderCore/RenderMaterialResult.h"
#include "YuEngine/RenderCore/RenderMaterialSnapshot.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Builds validated material binding requests from compact render material values.
 */
class RenderMaterial final {
public:
    /**
     * @comment Constructs a RenderMaterial instance.
     * @param desc Input descriptor.
     */
    explicit RenderMaterial(const RenderMaterialDesc &desc=RenderMaterialDesc());

    /**
     * @comment Validates material values and writes a caller-owned binding request.
     * @param request Input render material request.
     * @param out_request Caller-owned material binding request.
     * @return Explicit operation result.
     */
    RenderMaterialResult BuildBindingRequest(
        const RenderMaterialRequest &request,
        MaterialBindingFixtureRequest *out_request);

    /**
     * @comment Returns the current render material snapshot.
     * @return Snapshot value.
     */
    RenderMaterialSnapshot Snapshot() const;

    /**
     * @comment Resets bounded render material records and counters.
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t material_id = 0U;
        std::uint32_t program_id = 0U;
        yuengine::rhi::RhiPipelineHandle pipeline{};
        yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
        yuengine::rhi::RhiSamplerBinding sampler{};
        std::array<std::uint8_t, MAX_RENDER_MATERIAL_CONSTANT_BYTES> constant_bytes{};
        std::size_t constant_byte_count = 0U;
        std::uint32_t pass_id = 0U;
    };

    RenderMaterialStatus ValidateRequest(const RenderMaterialRequest &request) const;
    bool HasRecordCapacity() const;
    bool HasMaterialId(std::uint32_t material_id) const;
    void FillBindingRequest(
        const RenderMaterialRequest &request,
        MaterialBindingFixtureRequest *out_request) const;
    void RecordAcceptedMaterial(const RenderMaterialRequest &request, RenderMaterialResult *result);
    void RecordRejectedMaterial(const RenderMaterialResult &result);

    RenderMaterialDesc desc_;
    RenderMaterialSnapshot snapshot_;
    std::array<Record, MAX_RENDER_MATERIAL_RECORDS> records_;
};
}
