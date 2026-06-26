// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixture.h

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
 * @comment 组合 public RHI 绑定 值 写入 固定容量 synthetic material fixture 记录.
 */
class MaterialBindingFixture final {
public:
    /**
     * @comment 构造 MaterialBindingFixture 实例。
     * @param desc 输入描述。
     */
    explicit MaterialBindingFixture(const MaterialBindingFixtureDesc &desc=MaterialBindingFixtureDesc());

    /**
     * @comment 验证并 writes material 绑定 值 到 一个 调用方持有 fixture pass 请求。
     * @param request 输入 材质绑定请求。
     * @param pass_request 调用方持有的 RenderFixturePass 请求 到 update。
     * @return 显式操作结果。
     */
    MaterialBindingFixtureResult Bind(
        const MaterialBindingFixtureRequest &request,
        RenderFixturePassRequest *pass_request);

    /**
     * @comment 绑定 material 值，并执行调用方持有的 fixture pass。
     * @param request 输入 材质绑定请求。
     * @param pass 调用方持有的 fixture pass。
     * @param pass_request 调用方持有的 RenderFixturePass 请求 到 update 和 execute。
     * @return 显式操作结果。
     */
    MaterialBindingFixtureResult BindAndExecute(
        const MaterialBindingFixtureRequest &request,
        RenderFixturePass *pass,
        RenderFixturePassRequest *pass_request);

    /**
     * @comment 返回当前 material 绑定 fixture 快照。
     * @return 快照值。
     */
    MaterialBindingFixtureSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 material 绑定 fixture 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t material_id = 0U;
        yuengine::rhi::RhiPipelineHandle pipeline{};
        yuengine::rhi::RhiBlendStateDesc blend_state{};
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
