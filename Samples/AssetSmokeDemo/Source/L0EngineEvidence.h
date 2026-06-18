/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/L0EngineEvidence.h
用途: 声明 L0 示例纵向证据入口。
*/

#pragma once

#include <cstdint>
#include <filesystem>
#include <span>

namespace asset_smoke_demo {

struct L0EngineEvidenceInput final {
    std::filesystem::path asset_root;
    std::filesystem::path texture_path;
    std::span<const std::uint8_t> texture_rgba{};
    std::uint32_t texture_width = 0U;
    std::uint32_t texture_height = 0U;
};

struct L0EngineEvidenceResult final {
    std::uint32_t file_read_byte_count = 0U;
    std::uint32_t decoded_texture_width = 0U;
    std::uint32_t decoded_texture_height = 0U;
    std::uint32_t uploaded_texture_generation = 0U;
    std::uint32_t render_frame_count = 0U;
    std::uint32_t input_event_count = 0U;
    const char *gamepad_state = "not_run";
    const char *audio_state = "not_run";
    const char *failure_stage = "not_run";
    bool file_read = false;
    bool resource_decode = false;
    bool texture_upload = false;
    bool rendercore_view_draw_material = false;
    bool hardware_frame = false;
    bool resize = false;
    bool shutdown = false;
};

bool RunL0EngineEvidence(const L0EngineEvidenceInput &input, L0EngineEvidenceResult *result);

} // asset_smoke_demo 命名空间
