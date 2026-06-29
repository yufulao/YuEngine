// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectRestoreHandoffBridge.cpp

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h"

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateBridge.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateResult.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreBridge.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreResult.h"

using yuengine::world::WorldSceneActiveRestoreGateBridge;
using yuengine::world::WorldSceneActiveRestoreGateResult;
using yuengine::world::WorldSceneActiveRestoreGateStatus;
using yuengine::world::WorldSceneApplyTimeRestoreProofStatus;
using yuengine::world::WorldSceneObjectTransformRestoreBridge;
using yuengine::world::WorldSceneObjectTransformRestoreResult;
using yuengine::world::WorldSceneObjectTransformRestoreStatus;

namespace yuengine::runtimeassetworldadapter {
RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffResult::Success(
    RuntimeAssetWorldObjectRestoreHandoffState state) {
    return RuntimeAssetWorldObjectRestoreHandoffResult{
        RuntimeAssetWorldObjectRestoreHandoffStatus::Success,
        RuntimeAssetWorldObjectAdapterStatus::Success,
        WorldSceneActiveRestoreGateStatus::Success,
        WorldSceneApplyTimeRestoreProofStatus::Success,
        WorldSceneObjectTransformRestoreStatus::Success,
        state};
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffResult::Failure(
    RuntimeAssetWorldObjectRestoreHandoffStatus status,
    RuntimeAssetWorldObjectAdapterStatus adapter_status,
    WorldSceneActiveRestoreGateStatus gate_status,
    WorldSceneApplyTimeRestoreProofStatus proof_status,
    WorldSceneObjectTransformRestoreStatus restore_status) {
    return RuntimeAssetWorldObjectRestoreHandoffResult{
        status,
        adapter_status,
        gate_status,
        proof_status,
        restore_status,
        RuntimeAssetWorldObjectRestoreHandoffState{}};
}

bool RuntimeAssetWorldObjectRestoreHandoffResult::Succeeded() const {
    return status == RuntimeAssetWorldObjectRestoreHandoffStatus::Success;
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::ApplyRestore(
    const RuntimeAssetWorldObjectRestoreHandoffRequest &request) {
    ++snapshot_.handoff_attempt_count;

    const RuntimeAssetWorldObjectRestoreHandoffStatus request_status = ValidateRequest(request);
    if (request_status != RuntimeAssetWorldObjectRestoreHandoffStatus::Success) {
        return RecordFailure(request_status);
    }

    RuntimeAssetWorldObjectAdapterBridge adapter_bridge{};
    const RuntimeAssetWorldObjectAdapterResult adapter_result =
        adapter_bridge.BuildRestoreRecords(*request.adapter_request);
    if (!adapter_result.Succeeded()) {
        return RecordAdapterFailure(adapter_result.status);
    }

    WorldSceneActiveRestoreGateBridge gate_bridge{};
    const WorldSceneActiveRestoreGateResult gate_result = gate_bridge.BuildGate(
        request.world,
        request.object_registry,
        request.resource_registry,
        request.identity_destination,
        request.transform_destination,
        request.attachment_destination,
        request.binding_destination,
        request.adapter_request->output_identities,
        adapter_result.state.output_identity_count,
        request.adapter_request->output_transforms,
        adapter_result.state.output_transform_count,
        request.input_attachments,
        request.input_attachment_count,
        request.input_bindings,
        request.input_binding_count,
        request.plan_scratch,
        request.plan_scratch_capacity,
        request.proof_scratch,
        request.proof_scratch_capacity,
        request.slice_scratch,
        request.slice_scratch_capacity,
        request.output_gates,
        request.output_gate_capacity);
    if (!gate_result.Succeeded()) {
        return RecordGateFailure(gate_result.status, gate_result.proof_status);
    }

    WorldSceneObjectTransformRestoreBridge restore_bridge{};
    const WorldSceneObjectTransformRestoreResult restore_result = restore_bridge.Restore(
        request.world,
        request.object_registry,
        request.identity_destination,
        request.transform_destination,
        request.adapter_request->output_identities,
        adapter_result.state.output_identity_count,
        request.adapter_request->output_transforms,
        adapter_result.state.output_transform_count);
    if (!restore_result.Succeeded()) {
        return RecordRestoreFailure(restore_result.status);
    }

    RuntimeAssetWorldObjectRestoreHandoffState state{};
    state.input_mapping_count = adapter_result.state.input_mapping_count;
    state.output_identity_count = adapter_result.state.output_identity_count;
    state.output_transform_count = adapter_result.state.output_transform_count;
    state.proof_record_count = gate_result.state.proof_record_count;
    state.slice_record_count = gate_result.state.slice_record_count;
    state.gate_record_count = gate_result.state.gate_record_count;
    state.restored_identity_count = restore_result.state.restored_identity_count;
    state.restored_transform_count = restore_result.state.restored_transform_count;
    return RecordSuccess(state);
}

RuntimeAssetWorldObjectRestoreHandoffSnapshot RuntimeAssetWorldObjectRestoreHandoffBridge::Snapshot() const {
    return snapshot_;
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::RecordFailure(
    RuntimeAssetWorldObjectRestoreHandoffStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    snapshot_.last_adapter_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    snapshot_.last_gate_status = WorldSceneActiveRestoreGateStatus::Success;
    snapshot_.last_proof_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    snapshot_.last_restore_status = WorldSceneObjectTransformRestoreStatus::Success;
    return RuntimeAssetWorldObjectRestoreHandoffResult::Failure(status);
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::RecordAdapterFailure(
    RuntimeAssetWorldObjectAdapterStatus adapter_status) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = RuntimeAssetWorldObjectRestoreHandoffStatus::AdapterBuildFailed;
    snapshot_.last_adapter_status = adapter_status;
    snapshot_.last_gate_status = WorldSceneActiveRestoreGateStatus::Success;
    snapshot_.last_proof_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    snapshot_.last_restore_status = WorldSceneObjectTransformRestoreStatus::Success;
    return RuntimeAssetWorldObjectRestoreHandoffResult::Failure(
        RuntimeAssetWorldObjectRestoreHandoffStatus::AdapterBuildFailed,
        adapter_status);
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::RecordGateFailure(
    WorldSceneActiveRestoreGateStatus gate_status,
    WorldSceneApplyTimeRestoreProofStatus proof_status) {
    ++snapshot_.failed_operation_count;
    ++snapshot_.rejected_operation_count;
    snapshot_.last_status = RuntimeAssetWorldObjectRestoreHandoffStatus::GateFailed;
    snapshot_.last_adapter_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    snapshot_.last_gate_status = gate_status;
    snapshot_.last_proof_status = proof_status;
    snapshot_.last_restore_status = WorldSceneObjectTransformRestoreStatus::Success;
    return RuntimeAssetWorldObjectRestoreHandoffResult::Failure(
        RuntimeAssetWorldObjectRestoreHandoffStatus::GateFailed,
        RuntimeAssetWorldObjectAdapterStatus::Success,
        gate_status,
        proof_status);
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::RecordRestoreFailure(
    WorldSceneObjectTransformRestoreStatus restore_status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = RuntimeAssetWorldObjectRestoreHandoffStatus::RestoreFailed;
    snapshot_.last_adapter_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    snapshot_.last_gate_status = WorldSceneActiveRestoreGateStatus::Success;
    snapshot_.last_proof_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    snapshot_.last_restore_status = restore_status;
    return RuntimeAssetWorldObjectRestoreHandoffResult::Failure(
        RuntimeAssetWorldObjectRestoreHandoffStatus::RestoreFailed,
        RuntimeAssetWorldObjectAdapterStatus::Success,
        WorldSceneActiveRestoreGateStatus::Success,
        WorldSceneApplyTimeRestoreProofStatus::Success,
        restore_status);
}

RuntimeAssetWorldObjectRestoreHandoffResult RuntimeAssetWorldObjectRestoreHandoffBridge::RecordSuccess(
    const RuntimeAssetWorldObjectRestoreHandoffState &state) {
    ++snapshot_.accepted_handoff_count;
    snapshot_.built_identity_count += state.output_identity_count;
    snapshot_.built_transform_count += state.output_transform_count;
    snapshot_.emitted_gate_record_count += state.gate_record_count;
    snapshot_.restored_identity_count += state.restored_identity_count;
    snapshot_.restored_transform_count += state.restored_transform_count;
    snapshot_.last_status = RuntimeAssetWorldObjectRestoreHandoffStatus::Success;
    snapshot_.last_adapter_status = RuntimeAssetWorldObjectAdapterStatus::Success;
    snapshot_.last_gate_status = WorldSceneActiveRestoreGateStatus::Success;
    snapshot_.last_proof_status = WorldSceneApplyTimeRestoreProofStatus::Success;
    snapshot_.last_restore_status = WorldSceneObjectTransformRestoreStatus::Success;
    return RuntimeAssetWorldObjectRestoreHandoffResult::Success(state);
}

RuntimeAssetWorldObjectRestoreHandoffStatus RuntimeAssetWorldObjectRestoreHandoffBridge::ValidateRequest(
    const RuntimeAssetWorldObjectRestoreHandoffRequest &request) const {
    if (request.adapter_request == nullptr) {
        return RuntimeAssetWorldObjectRestoreHandoffStatus::InvalidAdapterRequest;
    }

    return RuntimeAssetWorldObjectRestoreHandoffStatus::Success;
}
}
