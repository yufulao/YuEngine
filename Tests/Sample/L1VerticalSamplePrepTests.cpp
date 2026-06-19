// 模块: Tests Sample
// 文件: Tests/Sample/L1VerticalSamplePrepTests.cpp

#include <cstdio>
#include <string_view>
#include <unordered_map>

#include "L1VerticalSamplePrep.h"

namespace {
constexpr const char *TEST_PREP = "Sample_L1VerticalPrep_BuildsManifestAndSubmitPrep";
constexpr const char *TEST_BOUNDARY = "Sample_L1VerticalPrep_UsesValueContractsWithoutHardware";
constexpr const char *TEST_OBJECT_GRAPH = "Sample_L1VerticalPrep_DeterministicObjectGraphHasStableSlots";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int SampleL1VerticalPrepBuildsManifestAndSubmitPrep() {
    asset_smoke_demo::L1VerticalSamplePrepResult result{};
    if (!asset_smoke_demo::RunL1VerticalSamplePrep(&result)) {
        return Fail(result.failure_stage);
    }

    if (!result.runtime_boot || !result.synthetic_manifest || !result.world_object) {
        return Fail("sample did not prepare runtime manifest and world object");
    }

    if (!result.asset_bindings || !result.input_command) {
        return Fail("sample did not bind assets and input command");
    }

    if (!result.render_scene_submit || !result.audio_scene_submit) {
        return Fail("sample did not prepare scene submits");
    }

    if (result.completed_frame_count != 2U) {
        return Fail("sample runtime frame count mismatch");
    }

    if (result.render_packet_count != 1U || result.audio_queue_request_count != 1U) {
        return Fail("sample submit output count mismatch");
    }

    return 0;
}

int SampleL1VerticalPrepUsesValueContractsWithoutHardware() {
    asset_smoke_demo::L1VerticalSamplePrepResult result{};
    if (!asset_smoke_demo::RunL1VerticalSamplePrep(&result)) {
        return Fail(result.failure_stage);
    }

    if (result.asset_count != 4U) {
        return Fail("sample did not use synthetic runtime assets");
    }

    if (result.input_command_count != 1U) {
        return Fail("sample did not use command snapshot input");
    }

    if (result.world_object_count != 1U) {
        return Fail("sample did not use deterministic world record");
    }

    return 0;
}

int SampleL1VerticalPrepDeterministicObjectGraphHasStableSlots() {
    asset_smoke_demo::L1VerticalSamplePrepResult result{};
    if (!asset_smoke_demo::RunL1VerticalSamplePrep(&result)) {
        return Fail(result.failure_stage);
    }

    if (!result.deterministic_object_graph) {
        return Fail("sample did not verify deterministic object graph");
    }

    if (!result.object_graph_invalid_no_mutation) {
        return Fail("sample did not verify invalid object graph no mutation");
    }

    if (result.world_object_count != 1U || result.object_graph_export_count != 1U) {
        return Fail("sample object graph count mismatch");
    }

    if (result.object_graph_component_slot_id != 31U) {
        return Fail("sample component slot mismatch");
    }

    if (result.object_graph_transform_z != 3.0F) {
        return Fail("sample transform snapshot mismatch");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> tests{
        {TEST_PREP, SampleL1VerticalPrepBuildsManifestAndSubmitPrep},
        {TEST_BOUNDARY, SampleL1VerticalPrepUsesValueContractsWithoutHardware},
        {TEST_OBJECT_GRAPH, SampleL1VerticalPrepDeterministicObjectGraphHasStableSlots},
    };

    const std::string_view test_name(argv[1]);
    const auto test = tests.find(test_name);
    if (test == tests.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test->second();
}
