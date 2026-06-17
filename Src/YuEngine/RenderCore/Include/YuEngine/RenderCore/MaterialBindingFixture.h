// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixture.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureConstants.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureDesc.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureResult.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureSnapshot.h"
#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"

namespace yuengine::rendercore {
/**
 * @comment Groups public RHI binding values into bounded synthetic material fixture records.
 */
class MaterialBindingFixture final {
public:
    /**
     * @comment Constructs a MaterialBindingFixture instance.
     * @param desc Input descriptor.
     */
    explicit MaterialBindingFixture(const MaterialBindingFixtureDesc &desc=MaterialBindingFixtureDesc());

    /**
     * @comment Validates and writes material binding values to a caller-owned fixture pass request.
     * @param request Input material binding request.
     * @param pass_request Caller-owned RenderFixturePass request to update.
     * @return Explicit operation result.
     */
    MaterialBindingFixtureResult Bind(
        const MaterialBindingFixtureRequest &request,
        RenderFixturePassRequest *pass_request);

    /**
     * @comment Binds material values and executes the caller-owned fixture pass.
     * @param request Input material binding request.
     * @param pass Caller-owned fixture pass.
     * @param pass_request Caller-owned RenderFixturePass request to update and execute.
     * @return Explicit operation result.
     */
    MaterialBindingFixtureResult BindAndExecute(
        const MaterialBindingFixtureRequest &request,
        RenderFixturePass *pass,
        RenderFixturePassRequest *pass_request);

    /**
     * @comment Returns the current material binding fixture snapshot.
     * @return Snapshot value.
     */
    MaterialBindingFixtureSnapshot Snapshot() const;
    /**
     * @comment Resets bounded material binding fixture records and counters.
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t material_id = 0U;
        yuengine::rhi::RhiPipelineHandle pipeline{};
        yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
        yuengine::rhi::RhiSamplerBinding sampler{};
        std::array<std::uint8_t, MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES> constant_bytes{};
        std::size_t constant_byte_count = 0U;
        std::uint32_t pass_id = 0U;
    };

    MaterialBindingFixtureStatus ValidateRequest(const MaterialBindingFixtureRequest &request) const;
    bool HasRecordCapacity() const;
    bool HasMaterialId(std::uint32_t material_id) const;
    void FillPassRequest(const MaterialBindingFixtureRequest &request, RenderFixturePassRequest *pass_request) const;
    void RecordAcceptedBinding(const MaterialBindingFixtureRequest &request, MaterialBindingFixtureResult *result);
    void RecordRejectedBinding(const MaterialBindingFixtureResult &result);
    void RecordRenderSuccess(const RenderFixturePassResult &pass_result, MaterialBindingFixtureResult *result);
    void RecordRenderFailure(const RenderFixturePassResult &pass_result, MaterialBindingFixtureResult *result);

    MaterialBindingFixtureDesc desc_;
    MaterialBindingFixtureSnapshot snapshot_;
    std::array<Record, MAX_MATERIAL_BINDING_FIXTURE_RECORDS> records_;
};
}
