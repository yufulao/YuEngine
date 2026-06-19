// 模块: Tests World
// 文件: Tests/World/WorldIdentityBaselineTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>

#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldIdentityBaseline.h"
#include "YuEngine/World/WorldIdentityBaselineDesc.h"
#include "YuEngine/World/WorldIdentityBaselineObjectDesc.h"
#include "YuEngine/World/WorldIdentityBaselineRecord.h"
#include "YuEngine/World/WorldIdentityBaselineResult.h"
#include "YuEngine/World/WorldIdentityBaselineSnapshot.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectTypeId;
using yuengine::world::WorldComponentSlotId;
using yuengine::world::WorldComponentTypeId;
using yuengine::world::WorldIdentityBaseline;
using yuengine::world::WorldIdentityBaselineDesc;
using yuengine::world::WorldIdentityBaselineObjectDesc;
using yuengine::world::WorldIdentityBaselineRecord;
using yuengine::world::WorldIdentityBaselineResult;
using yuengine::world::WorldIdentityBaselineSnapshot;
using yuengine::world::WorldIdentityBaselineStatus;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldTransformState;

namespace {
constexpr std::string_view TEST_CREATE_QUERY_EXPORT_DESTROY =
    "WorldIdentityBaseline_CreateQueryExportDestroyDeterministic";
constexpr std::string_view TEST_INVALID_COMPONENT =
    "WorldIdentityBaseline_InvalidComponentDoesNotMutate";
constexpr std::string_view TEST_DUPLICATE_WORLD_OBJECT =
    "WorldIdentityBaseline_DuplicateWorldObjectDoesNotMutate";
constexpr std::string_view TEST_DESTROY_MISSING =
    "WorldIdentityBaseline_DestroyMissingDoesNotMutate";

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

WorldObjectId ObjectId(std::uint32_t value) {
    return WorldObjectId{value};
}

ObjectTypeId ObjectType(std::uint32_t value) {
    return ObjectTypeId{value};
}

WorldComponentTypeId ComponentType(std::uint32_t value) {
    return WorldComponentTypeId{value};
}

WorldComponentSlotId ComponentSlot(std::uint32_t value) {
    return WorldComponentSlotId{value};
}

WorldIdentityBaselineDesc MakeBaselineDesc() {
    WorldIdentityBaselineDesc desc{};
    desc.world_desc.object_capacity = 4U;
    desc.world_desc.phase_trace_capacity = 4U;
    desc.object_registry_desc.object_capacity = 4U;
    desc.object_registry_desc.type_capacity = 4U;
    desc.identity_bridge_desc.bridge_capacity = 4U;
    desc.transform_bridge_desc.bridge_capacity = 4U;
    desc.component_attachment_desc.attachment_capacity = 4U;
    desc.record_capacity = 4U;
    return desc;
}

WorldTransformState TransformForObject(std::uint32_t value) {
    const float base_value = static_cast<float>(value);
    WorldTransformState transform{};
    transform.translation_x = base_value;
    transform.translation_y = base_value + 1.0F;
    transform.translation_z = base_value + 2.0F;
    transform.rotation_w = 1.0F;
    transform.scale_x = 1.0F;
    transform.scale_y = 1.0F;
    transform.scale_z = 1.0F;
    return transform;
}

WorldIdentityBaselineObjectDesc MakeObjectDesc(
    std::uint32_t world_id,
    std::uint32_t object_type,
    std::uint32_t component_type,
    std::uint32_t component_slot) {
    WorldIdentityBaselineObjectDesc desc{};
    desc.world_object_id = ObjectId(world_id);
    desc.object_descriptor = ObjectDescriptor{ObjectType(object_type), 0U};
    desc.component_type_id = ComponentType(component_type);
    desc.component_slot_id = ComponentSlot(component_slot);
    desc.transform_state = TransformForObject(world_id);
    desc.is_enabled = true;
    return desc;
}

bool RecordMatchesDesc(
    const WorldIdentityBaselineRecord &record,
    const WorldIdentityBaselineObjectDesc &desc) {
    if (record.world_object_id.value != desc.world_object_id.value) {
        return false;
    }

    if (!record.object_handle.IsValid()) {
        return false;
    }

    if (record.component_type_id.value != desc.component_type_id.value) {
        return false;
    }

    if (record.component_slot_id.value != desc.component_slot_id.value) {
        return false;
    }

    if (record.transform_state.translation_x != desc.transform_state.translation_x) {
        return false;
    }

    if (record.transform_state.translation_y != desc.transform_state.translation_y) {
        return false;
    }

    if (record.transform_state.translation_z != desc.transform_state.translation_z) {
        return false;
    }

    return record.is_active;
}

int ExpectSuccess(const WorldIdentityBaselineResult &result, std::string_view failure_message) {
    if (result.Succeeded()) {
        return 0;
    }

    return Fail(std::string(failure_message));
}

int WorldIdentityBaselineCreateQueryExportDestroyDeterministic() {
    WorldIdentityBaseline baseline(MakeBaselineDesc());
    const WorldIdentityBaselineObjectDesc first_desc = MakeObjectDesc(22U, 7U, 12U, 30U);
    const WorldIdentityBaselineObjectDesc second_desc = MakeObjectDesc(11U, 8U, 13U, 31U);

    const WorldIdentityBaselineResult first_create = baseline.CreateObject(first_desc);
    int ret_code = ExpectSuccess(first_create, "first create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    const WorldIdentityBaselineResult second_create = baseline.CreateObject(second_desc);
    ret_code = ExpectSuccess(second_create, "second create failed");
    if (ret_code != 0) {
        return ret_code;
    }

    std::array<WorldIdentityBaselineRecord, 2U> records{};
    const std::uint32_t export_count = baseline.ExportRecords(
        records.data(),
        static_cast<std::uint32_t>(records.size()));
    if (export_count != 2U) {
        return Fail("export count mismatch after create");
    }

    if (!RecordMatchesDesc(records[0U], first_desc)) {
        return Fail("first exported record mismatch");
    }

    if (!RecordMatchesDesc(records[1U], second_desc)) {
        return Fail("second exported record mismatch");
    }

    const WorldIdentityBaselineResult query_result = baseline.QueryObject(second_desc.world_object_id);
    ret_code = ExpectSuccess(query_result, "query second object failed");
    if (ret_code != 0) {
        return ret_code;
    }

    if (!RecordMatchesDesc(query_result.record, second_desc)) {
        return Fail("query second record mismatch");
    }

    const WorldIdentityBaselineStatus destroy_first = baseline.DestroyObject(first_desc.world_object_id);
    if (destroy_first != WorldIdentityBaselineStatus::Success) {
        return Fail("destroy first object failed");
    }

    records = {};
    const std::uint32_t after_destroy_count = baseline.ExportRecords(
        records.data(),
        static_cast<std::uint32_t>(records.size()));
    if (after_destroy_count != 1U) {
        return Fail("export count mismatch after destroy");
    }

    if (!RecordMatchesDesc(records[0U], second_desc)) {
        return Fail("remaining exported record mismatch");
    }

    const WorldIdentityBaselineResult missing_query = baseline.QueryObject(first_desc.world_object_id);
    if (missing_query.status != WorldIdentityBaselineStatus::RecordNotFound) {
        return Fail("destroyed object query did not report missing record");
    }

    const WorldIdentityBaselineStatus destroy_second = baseline.DestroyObject(second_desc.world_object_id);
    if (destroy_second != WorldIdentityBaselineStatus::Success) {
        return Fail("destroy second object failed");
    }

    const WorldIdentityBaselineSnapshot snapshot = baseline.Snapshot();
    if (snapshot.active_record_count != 0U) {
        return Fail("active count not cleared after destroy");
    }

    if (snapshot.created_record_count != 2ULL) {
        return Fail("created counter mismatch");
    }

    if (snapshot.destroyed_record_count != 2ULL) {
        return Fail("destroyed counter mismatch");
    }

    return 0;
}

int WorldIdentityBaselineInvalidComponentDoesNotMutate() {
    WorldIdentityBaseline baseline(MakeBaselineDesc());
    WorldIdentityBaselineObjectDesc desc = MakeObjectDesc(1U, 7U, 12U, 30U);
    desc.component_type_id = WorldComponentTypeId{};

    const WorldIdentityBaselineResult result = baseline.CreateObject(desc);
    if (result.status != WorldIdentityBaselineStatus::InvalidComponentTypeId) {
        return Fail("invalid component type status mismatch");
    }

    const WorldIdentityBaselineSnapshot snapshot = baseline.Snapshot();
    if (snapshot.active_record_count != 0U) {
        return Fail("invalid component mutated active records");
    }

    if (snapshot.failed_operation_count != 1ULL) {
        return Fail("invalid component failed counter mismatch");
    }

    const std::uint32_t export_count = baseline.ExportRecords(nullptr, 0U);
    if (export_count != 0U) {
        return Fail("invalid component exported records");
    }

    return 0;
}

int WorldIdentityBaselineDuplicateWorldObjectDoesNotMutate() {
    WorldIdentityBaseline baseline(MakeBaselineDesc());
    const WorldIdentityBaselineObjectDesc first_desc = MakeObjectDesc(2U, 7U, 12U, 30U);
    const WorldIdentityBaselineObjectDesc duplicate_desc = MakeObjectDesc(2U, 8U, 13U, 31U);

    const WorldIdentityBaselineResult first_create = baseline.CreateObject(first_desc);
    int ret_code = ExpectSuccess(first_create, "first create failed before duplicate");
    if (ret_code != 0) {
        return ret_code;
    }

    const WorldIdentityBaselineResult duplicate_result = baseline.CreateObject(duplicate_desc);
    if (duplicate_result.status != WorldIdentityBaselineStatus::DuplicateWorldObjectId) {
        return Fail("duplicate world object status mismatch");
    }

    std::array<WorldIdentityBaselineRecord, 2U> records{};
    const std::uint32_t export_count = baseline.ExportRecords(
        records.data(),
        static_cast<std::uint32_t>(records.size()));
    if (export_count != 1U) {
        return Fail("duplicate changed exported record count");
    }

    if (!RecordMatchesDesc(records[0U], first_desc)) {
        return Fail("duplicate mutated original record");
    }

    const WorldIdentityBaselineSnapshot snapshot = baseline.Snapshot();
    if (snapshot.active_record_count != 1U) {
        return Fail("duplicate changed active count");
    }

    if (snapshot.created_record_count != 1ULL) {
        return Fail("duplicate changed created count");
    }

    if (snapshot.failed_operation_count != 1ULL) {
        return Fail("duplicate failed counter mismatch");
    }

    return 0;
}

int WorldIdentityBaselineDestroyMissingDoesNotMutate() {
    WorldIdentityBaseline baseline(MakeBaselineDesc());

    const WorldIdentityBaselineStatus status = baseline.DestroyObject(ObjectId(3U));
    if (status != WorldIdentityBaselineStatus::RecordNotFound) {
        return Fail("destroy missing status mismatch");
    }

    const WorldIdentityBaselineSnapshot snapshot = baseline.Snapshot();
    if (snapshot.active_record_count != 0U) {
        return Fail("destroy missing mutated active count");
    }

    if (snapshot.failed_operation_count != 1ULL) {
        return Fail("destroy missing failed counter mismatch");
    }

    const std::uint32_t export_count = baseline.ExportRecords(nullptr, 0U);
    if (export_count != 0U) {
        return Fail("destroy missing exported records");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected exactly one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_CREATE_QUERY_EXPORT_DESTROY) {
        return WorldIdentityBaselineCreateQueryExportDestroyDeterministic();
    }

    if (test_name == TEST_INVALID_COMPONENT) {
        return WorldIdentityBaselineInvalidComponentDoesNotMutate();
    }

    if (test_name == TEST_DUPLICATE_WORLD_OBJECT) {
        return WorldIdentityBaselineDuplicateWorldObjectDoesNotMutate();
    }

    if (test_name == TEST_DESTROY_MISSING) {
        return WorldIdentityBaselineDestroyMissingDoesNotMutate();
    }

    return Fail("unknown test name");
}
