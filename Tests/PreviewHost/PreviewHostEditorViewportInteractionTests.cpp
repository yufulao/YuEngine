// Module: Tests PreviewHost
// File: Tests/PreviewHost/PreviewHostEditorViewportInteractionTests.cpp

#include <array>
#include <cmath>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/PreviewHost/PreviewHost.h"

using yuengine::previewhost::PreviewHost;
using yuengine::previewhost::PreviewHostCameraState;
using yuengine::previewhost::PreviewHostEditorViewportInteractionRequest;
using yuengine::previewhost::PreviewHostEditorViewportInteractionResult;
using yuengine::previewhost::PreviewHostFrameFormat;
using yuengine::previewhost::PreviewHostHitRecord;
using yuengine::previewhost::PreviewHostSelectionRecord;
using yuengine::previewhost::PreviewHostStatus;
using yuengine::previewhost::PreviewHostTransformFeedback;
using yuengine::previewhost::PreviewHostViewportInteractionBlockedLayer;
using yuengine::previewhost::PreviewHostViewportInteractionCommand;
using yuengine::previewhost::PreviewHostViewportInteractionKind;
using yuengine::previewhost::PreviewHostViewportInteractionLedgerRecord;
using yuengine::previewhost::PreviewHostViewportSessionResult;
using yuengine::runtimeasset::RuntimeAssetSceneEntityRecord;
using yuengine::world::WorldObjectId;

namespace {
constexpr const char *TEST_ORBIT =
    "PreviewHostEditorViewportInteraction_OrbitCommandUpdatesCameraLedger";
constexpr const char *TEST_SELECT =
    "PreviewHostEditorViewportInteraction_SelectCommandEmitsFeedback";
constexpr const char *TEST_REJECT =
    "PreviewHostEditorViewportInteraction_RejectsInvalidSessionWithoutMutation";
constexpr const char *TEST_CAPACITY =
    "PreviewHostEditorViewportInteraction_OutputCapacityPreservesRequiredCounts";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr float TOLERANCE = 0.0001F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

PreviewHostViewportSessionResult BuiltViewportSession() {
    PreviewHostViewportSessionResult result{};
    result.status = PreviewHostStatus::Success;
    result.frame.status = PreviewHostStatus::Success;
    result.frame.frame.frame_id = 88U;
    result.frame.frame.width = 1280U;
    result.frame.frame.height = 720U;
    result.frame.frame.format = PreviewHostFrameFormat::Headless;
    result.frame.submitted_render_scene_frame = true;
    result.frame.headless_output = true;
    result.frame.submitted_entity_count = 2U;
    result.camera_state.camera_id = 11U;
    result.camera_state.orbit_angle_radians = 0.5F;
    result.camera_state.orbit_radius = 8.0F;
    result.camera_state.orbit_height = 2.0F;
    result.selected_entity_index = 0U;
    result.viewport_width = 1280U;
    result.viewport_height = 720U;
    result.consumed_viewport_controls = true;
    result.built_frame = true;
    return result;
}

RuntimeAssetSceneEntityRecord Entity(
    std::uint32_t entity_id,
    std::uint32_t world_object_id,
    float translation_x) {
    RuntimeAssetSceneEntityRecord record{};
    record.entity_id = entity_id;
    record.world_object_id = WorldObjectId{world_object_id};
    record.transform.translation_x = translation_x;
    return record;
}

bool SentinelLedgerUnchanged(const PreviewHostViewportInteractionLedgerRecord &record) {
    return record.frame_id == 9001U && record.command_sequence == 9002U &&
        record.kind == PreviewHostViewportInteractionKind::Zoom;
}

bool SentinelHitUnchanged(const PreviewHostHitRecord &record) {
    return record.entity_index == 9003U && !record.hit_available;
}

bool SentinelSelectionUnchanged(const PreviewHostSelectionRecord &record) {
    return record.entity_index == 9004U && !record.selectable;
}

bool SentinelTransformUnchanged(const PreviewHostTransformFeedback &record) {
    return record.world_object_id.value == 9005U && !record.transform_available;
}

int PreviewHostEditorViewportInteractionOrbitCommandUpdatesCameraLedger() {
    const PreviewHostViewportSessionResult session = BuiltViewportSession();
    std::array<PreviewHostViewportInteractionLedgerRecord, 1U> ledger{};

    PreviewHostViewportInteractionCommand command{};
    command.kind = PreviewHostViewportInteractionKind::Orbit;
    command.orbit_delta_radians = 0.25F;

    PreviewHostEditorViewportInteractionRequest request{};
    request.viewport_session = &session;
    request.command = command;
    request.command_sequence = 7U;
    request.ledger_output =
        std::span<PreviewHostViewportInteractionLedgerRecord>(ledger.data(), ledger.size());

    PreviewHost host;
    PreviewHostEditorViewportInteractionResult result{};
    const PreviewHostStatus status =
        host.BuildEditorViewportInteractionSurface(request, &result);
    if (status != PreviewHostStatus::Success || !result.Succeeded()) {
        return Fail("preview host viewport orbit interaction failed");
    }

    if (!result.consumed_viewport_session ||
        !result.consumed_engine_viewport_frame ||
        !result.processed_camera_command ||
        result.processed_selection_command ||
        result.ledger_record_count != 1U ||
        !result.emitted_interaction_ledger) {
        return Fail("preview host viewport orbit counters mismatch");
    }

    if (!Approx(result.before_camera.orbit_angle_radians, 0.5F) ||
        !Approx(result.after_camera.orbit_angle_radians, 0.75F) ||
        !Approx(ledger[0U].before_camera.orbit_angle_radians, 0.5F) ||
        !Approx(ledger[0U].after_camera.orbit_angle_radians, 0.75F) ||
        ledger[0U].frame_id != 88U ||
        ledger[0U].command_sequence != 7U ||
        !ledger[0U].camera_changed) {
        return Fail("preview host viewport orbit ledger mismatch");
    }

    if (result.opened_native_window || result.used_forbidden_preview_path) {
        return Fail("preview host viewport orbit boundary flags changed");
    }

    return 0;
}

int PreviewHostEditorViewportInteractionSelectCommandEmitsFeedback() {
    const PreviewHostViewportSessionResult session = BuiltViewportSession();
    const std::array<RuntimeAssetSceneEntityRecord, 2U> scene_entities{
        Entity(1U, 41U, 1.5F),
        Entity(2U, 42U, 2.5F)};
    std::array<PreviewHostHitRecord, 1U> hits{};
    std::array<PreviewHostSelectionRecord, 1U> selections{};
    std::array<PreviewHostTransformFeedback, 1U> transforms{};
    std::array<PreviewHostViewportInteractionLedgerRecord, 1U> ledger{};

    PreviewHostViewportInteractionCommand command{};
    command.kind = PreviewHostViewportInteractionKind::SelectEntity;
    command.target_entity_index = 1U;

    PreviewHostEditorViewportInteractionRequest request{};
    request.viewport_session = &session;
    request.command = command;
    request.command_sequence = 9U;
    request.scene_entities =
        std::span<const RuntimeAssetSceneEntityRecord>(
            scene_entities.data(),
            scene_entities.size());
    request.hit_output = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    request.selection_output =
        std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    request.transform_feedback_output =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());
    request.ledger_output =
        std::span<PreviewHostViewportInteractionLedgerRecord>(ledger.data(), ledger.size());

    PreviewHost host;
    PreviewHostEditorViewportInteractionResult result{};
    const PreviewHostStatus status =
        host.BuildEditorViewportInteractionSurface(request, &result);
    if (status != PreviewHostStatus::Success) {
        return Fail("preview host viewport selection interaction failed");
    }

    if (!result.processed_selection_command ||
        result.selected_entity_index != 1U ||
        result.selected_world_object_id.value != 42U ||
        result.hit_record_count != 1U ||
        result.selection_record_count != 1U ||
        result.transform_feedback_count != 1U ||
        !result.emitted_hit_feedback ||
        !result.emitted_selection_feedback ||
        !result.emitted_transform_feedback) {
        return Fail("preview host viewport selection counters mismatch");
    }

    if (hits[0U].world_object_id.value != 42U ||
        hits[0U].entity_index != 1U ||
        !hits[0U].hit_available ||
        selections[0U].world_object_id.value != 42U ||
        !selections[0U].selectable ||
        transforms[0U].world_object_id.value != 42U ||
        !Approx(transforms[0U].transform.translation_x, 2.5F) ||
        !transforms[0U].transform_available ||
        ledger[0U].selected_world_object_id.value != 42U ||
        !ledger[0U].selection_changed) {
        return Fail("preview host viewport selection feedback mismatch");
    }

    return 0;
}

int PreviewHostEditorViewportInteractionRejectsInvalidSessionWithoutMutation() {
    PreviewHostViewportSessionResult session = BuiltViewportSession();
    session.built_frame = false;
    session.frame.submitted_render_scene_frame = false;
    std::array<RuntimeAssetSceneEntityRecord, 1U> scene_entities{Entity(1U, 51U, 3.5F)};
    std::array<PreviewHostHitRecord, 1U> hits{};
    hits[0U].entity_index = 9003U;
    hits[0U].hit_available = false;
    std::array<PreviewHostSelectionRecord, 1U> selections{};
    selections[0U].entity_index = 9004U;
    selections[0U].selectable = false;
    std::array<PreviewHostTransformFeedback, 1U> transforms{};
    transforms[0U].world_object_id = WorldObjectId{9005U};
    transforms[0U].transform_available = false;
    std::array<PreviewHostViewportInteractionLedgerRecord, 1U> ledger{};
    ledger[0U].frame_id = 9001U;
    ledger[0U].command_sequence = 9002U;
    ledger[0U].kind = PreviewHostViewportInteractionKind::Zoom;

    PreviewHostViewportInteractionCommand command{};
    command.kind = PreviewHostViewportInteractionKind::SelectEntity;
    command.target_entity_index = 0U;

    PreviewHostEditorViewportInteractionRequest request{};
    request.viewport_session = &session;
    request.command = command;
    request.command_sequence = 11U;
    request.scene_entities =
        std::span<const RuntimeAssetSceneEntityRecord>(
            scene_entities.data(),
            scene_entities.size());
    request.hit_output = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    request.selection_output =
        std::span<PreviewHostSelectionRecord>(selections.data(), selections.size());
    request.transform_feedback_output =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());
    request.ledger_output =
        std::span<PreviewHostViewportInteractionLedgerRecord>(ledger.data(), ledger.size());

    PreviewHost host;
    PreviewHostEditorViewportInteractionResult result{};
    const PreviewHostStatus status =
        host.BuildEditorViewportInteractionSurface(request, &result);
    if (status != PreviewHostStatus::StaleSession) {
        return Fail("preview host viewport invalid session status mismatch");
    }

    if (result.blocked_layer != PreviewHostViewportInteractionBlockedLayer::ViewportSession ||
        result.consumed_viewport_session ||
        result.emitted_interaction_ledger) {
        return Fail("preview host viewport invalid session result mismatch");
    }

    if (!SentinelLedgerUnchanged(ledger[0U]) ||
        !SentinelHitUnchanged(hits[0U]) ||
        !SentinelSelectionUnchanged(selections[0U]) ||
        !SentinelTransformUnchanged(transforms[0U])) {
        return Fail("preview host viewport invalid session mutated output");
    }

    return 0;
}

int PreviewHostEditorViewportInteractionOutputCapacityPreservesRequiredCounts() {
    const PreviewHostViewportSessionResult session = BuiltViewportSession();

    PreviewHostViewportInteractionCommand orbit_command{};
    orbit_command.kind = PreviewHostViewportInteractionKind::Orbit;
    orbit_command.orbit_delta_radians = 0.25F;

    PreviewHostEditorViewportInteractionRequest orbit_request{};
    orbit_request.viewport_session = &session;
    orbit_request.command = orbit_command;
    orbit_request.command_sequence = 13U;

    PreviewHost host;
    PreviewHostEditorViewportInteractionResult orbit_result{};
    const PreviewHostStatus orbit_status =
        host.BuildEditorViewportInteractionSurface(orbit_request, &orbit_result);
    if (orbit_status != PreviewHostStatus::OutputCapacityExceeded) {
        return Fail("preview host viewport ledger capacity status mismatch");
    }

    if (orbit_result.ledger_record_count != 1U ||
        orbit_result.hit_record_count != 0U ||
        orbit_result.selection_record_count != 0U ||
        orbit_result.transform_feedback_count != 0U) {
        return Fail("preview host viewport ledger capacity required counts mismatch");
    }

    if (orbit_result.blocked_layer != PreviewHostViewportInteractionBlockedLayer::Output ||
        orbit_result.emitted_interaction_ledger) {
        return Fail("preview host viewport ledger capacity result mismatch");
    }

    const std::array<RuntimeAssetSceneEntityRecord, 2U> scene_entities{
        Entity(1U, 41U, 1.5F),
        Entity(2U, 42U, 2.5F)};
    std::array<PreviewHostHitRecord, 1U> hits{};
    hits[0U].entity_index = 9003U;
    hits[0U].hit_available = false;
    std::array<PreviewHostTransformFeedback, 1U> transforms{};
    transforms[0U].world_object_id = WorldObjectId{9005U};
    transforms[0U].transform_available = false;
    std::array<PreviewHostViewportInteractionLedgerRecord, 1U> ledger{};
    ledger[0U].frame_id = 9001U;
    ledger[0U].command_sequence = 9002U;
    ledger[0U].kind = PreviewHostViewportInteractionKind::Zoom;

    PreviewHostViewportInteractionCommand select_command{};
    select_command.kind = PreviewHostViewportInteractionKind::SelectEntity;
    select_command.target_entity_index = 1U;

    PreviewHostEditorViewportInteractionRequest select_request{};
    select_request.viewport_session = &session;
    select_request.command = select_command;
    select_request.command_sequence = 14U;
    select_request.scene_entities =
        std::span<const RuntimeAssetSceneEntityRecord>(
            scene_entities.data(),
            scene_entities.size());
    select_request.hit_output = std::span<PreviewHostHitRecord>(hits.data(), hits.size());
    select_request.transform_feedback_output =
        std::span<PreviewHostTransformFeedback>(transforms.data(), transforms.size());
    select_request.ledger_output =
        std::span<PreviewHostViewportInteractionLedgerRecord>(ledger.data(), ledger.size());

    PreviewHostEditorViewportInteractionResult select_result{};
    const PreviewHostStatus select_status =
        host.BuildEditorViewportInteractionSurface(select_request, &select_result);
    if (select_status != PreviewHostStatus::OutputCapacityExceeded) {
        return Fail("preview host viewport feedback capacity status mismatch");
    }

    if (select_result.hit_record_count != 1U ||
        select_result.selection_record_count != 1U ||
        select_result.transform_feedback_count != 1U ||
        select_result.ledger_record_count != 1U) {
        return Fail("preview host viewport feedback capacity required counts mismatch");
    }

    if (select_result.blocked_layer != PreviewHostViewportInteractionBlockedLayer::Output ||
        select_result.emitted_hit_feedback ||
        select_result.emitted_selection_feedback ||
        select_result.emitted_transform_feedback ||
        select_result.emitted_interaction_ledger) {
        return Fail("preview host viewport feedback capacity result mismatch");
    }

    if (!SentinelLedgerUnchanged(ledger[0U]) ||
        !SentinelHitUnchanged(hits[0U]) ||
        !SentinelTransformUnchanged(transforms[0U])) {
        return Fail("preview host viewport feedback capacity mutated output");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_ORBIT) {
        return PreviewHostEditorViewportInteractionOrbitCommandUpdatesCameraLedger();
    }

    if (name == TEST_SELECT) {
        return PreviewHostEditorViewportInteractionSelectCommandEmitsFeedback();
    }

    if (name == TEST_REJECT) {
        return PreviewHostEditorViewportInteractionRejectsInvalidSessionWithoutMutation();
    }

    if (name == TEST_CAPACITY) {
        return PreviewHostEditorViewportInteractionOutputCapacityPreservesRequiredCounts();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
