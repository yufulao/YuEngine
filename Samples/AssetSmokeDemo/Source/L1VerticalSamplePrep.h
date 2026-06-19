/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/L1VerticalSamplePrep.h
用途: L1 纵向 sample synthetic scene manifest 和 submit prep。
*/

#pragma once

#include <cstdint>

namespace asset_smoke_demo {
struct L1VerticalSamplePrepResult final {
    const char *failure_stage = "not_run";
    bool runtime_boot = false;
    bool synthetic_manifest = false;
    bool world_object = false;
    bool asset_bindings = false;
    bool input_command = false;
    bool render_scene_submit = false;
    bool audio_scene_submit = false;
    std::uint32_t completed_frame_count = 0U;
    std::uint32_t world_object_count = 0U;
    std::uint32_t asset_count = 0U;
    std::uint32_t input_command_count = 0U;
    std::uint32_t render_packet_count = 0U;
    std::uint32_t audio_queue_request_count = 0U;
};

bool RunL1VerticalSamplePrep(L1VerticalSamplePrepResult *result);
}
