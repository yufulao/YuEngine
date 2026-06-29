// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofSliceRecord.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanRecord.h"

namespace yuengine::object {
class ObjectRegistry;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::world {
class WorldComponentAttachmentBridge;
class WorldComponentResourceBindingBridge;
class WorldInstance;
class WorldObjectIdentityBridge;
class WorldTransformBridge;
}

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectRestoreHandoffRequest final {
    const RuntimeAssetWorldObjectAdapterRequest *adapter_request = nullptr;
    yuengine::world::WorldInstance *world = nullptr;
    yuengine::object::ObjectRegistry *object_registry = nullptr;
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::world::WorldObjectIdentityBridge *identity_destination = nullptr;
    yuengine::world::WorldTransformBridge *transform_destination = nullptr;
    yuengine::world::WorldComponentAttachmentBridge *attachment_destination = nullptr;
    yuengine::world::WorldComponentResourceBindingBridge *binding_destination = nullptr;
    const yuengine::world::WorldComponentAttachmentSnapshotRecord *input_attachments = nullptr;
    std::uint32_t input_attachment_count = 0U;
    const yuengine::world::WorldComponentResourceBindingSnapshotRecord *input_bindings = nullptr;
    std::uint32_t input_binding_count = 0U;
    yuengine::world::WorldSceneDecodedRestorePlanRecord *plan_scratch = nullptr;
    std::uint32_t plan_scratch_capacity = 0U;
    yuengine::world::WorldSceneApplyTimeRestoreProofRecord *proof_scratch = nullptr;
    std::uint32_t proof_scratch_capacity = 0U;
    yuengine::world::WorldSceneApplyTimeRestoreProofSliceRecord *slice_scratch = nullptr;
    std::uint32_t slice_scratch_capacity = 0U;
    yuengine::world::WorldSceneActiveRestoreGateRecord *output_gates = nullptr;
    std::uint32_t output_gate_capacity = 0U;
};
}
