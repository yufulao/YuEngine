/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/L1VerticalSamplePrep.h
用途: L1 纵向 sample synthetic scene manifest 和 submit prep。
*/

#pragma once

#include <cstdint>

namespace asset_smoke_demo {
struct L1VerticalSampleValidationRoute final {
    const char *debug_build_command = "";
    const char *release_build_command = "";
    const char *fast_validation_command = "";
    const char *sample_smoke_test_name = "";
    bool debug_command_available = false;
    bool release_command_available = false;
    bool fast_command_available = false;
    bool sample_smoke_registered = false;
};

struct L1VerticalSamplePrepResult final {
    const char *failure_stage = "not_run";
    bool runtime_boot = false;
    bool synthetic_manifest = false;
    bool world_object = false;
    bool deterministic_object_graph = false;
    bool object_graph_invalid_no_mutation = false;
    bool asset_bindings = false;
    bool texture_asset_binding = false;
    bool audio_asset_binding = false;
    bool input_command = false;
    bool render_scene_submit = false;
    bool render_scene_route = false;
    bool audio_scene_submit = false;
    bool audio_scene_route = false;
    bool resize_route = false;
    bool shutdown_route = false;
    bool validation_route = false;
    std::uint32_t completed_frame_count = 0U;
    std::uint32_t world_object_count = 0U;
    std::uint32_t object_graph_export_count = 0U;
    std::uint32_t object_graph_component_slot_id = 0U;
    float object_graph_transform_z = 0.0F;
    std::uint32_t asset_count = 0U;
    std::uint32_t input_command_count = 0U;
    std::uint32_t render_packet_count = 0U;
    std::uint32_t audio_queue_request_count = 0U;
    std::uint32_t route_evidence_count = 0U;
    std::uint32_t render_frame_id = 0U;
    std::uint32_t render_camera_id = 0U;
    std::uint32_t render_sampled_texture_slot = 0U;
    std::uint32_t audio_frame_id = 0U;
    std::uint32_t audio_bus_id = 0U;
    std::uint32_t audio_queue_id = 0U;
};

bool RunL1VerticalSamplePrep(L1VerticalSamplePrepResult *result);
bool BuildL1VerticalSampleValidationRoute(L1VerticalSampleValidationRoute *route);
}
