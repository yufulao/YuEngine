// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterial.h

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
 * @comment 构建 已验证 材质绑定请求s 从 紧凑 render material 值.
 */
class RenderMaterial final {
public:
    /**
     * @comment 构造 RenderMaterial 实例。
     * @param desc 输入描述。
     */
    explicit RenderMaterial(const RenderMaterialDesc &desc=RenderMaterialDesc());

    /**
     * @comment 验证 material 值 和 writes 一个 调用方持有 绑定 请求。
     * @param request 输入 render material 请求。
     * @param out_request 调用方持有的 材质绑定请求。
     * @return 显式操作结果。
     */
    RenderMaterialResult BuildBindingRequest(
        const RenderMaterialRequest &request,
        MaterialBindingFixtureRequest *out_request);

    /**
     * @comment 返回当前 render material 快照。
     * @return 快照值。
     */
    RenderMaterialSnapshot Snapshot() const;

    /**
     * @comment 重置固定容量 render material 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t material_id = 0U;
        std::uint32_t program_id = 0U;
        yuengine::rhi::RhiPipelineHandle pipeline{};
        yuengine::rhi::RhiBlendStateDesc blend_state{};
        yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
        yuengine::rhi::RhiSamplerBinding sampler{};
        std::array<std::uint8_t, MAX_RENDER_MATERIAL_CONSTANT_BYTES> constant_bytes{};
        std::size_t constant_byte_count = 0U;
        std::uint32_t pass_id = 0U;
    };

    RenderMaterialStatus ValidateRequest(const RenderMaterialRequest &request) const;
    bool HasRecordCapacity() const;
    std::size_t RequiredMaterialRecordCount() const;
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
