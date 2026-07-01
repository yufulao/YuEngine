// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::resource {
struct ResourceSnapshot final {
    std::uint32_t resource_capacity;
    std::uint32_t type_capacity;
    std::uint32_t dependency_edge_capacity;
    std::uint32_t registered_resource_count;
    std::uint32_t type_count;
    std::uint64_t acquired_handle_count;
    std::uint64_t released_handle_count;
    std::uint32_t retired_resource_count;
    std::uint32_t dependency_edge_count;
    std::uint32_t dependency_validation_count;
    std::uint32_t failed_operation_count;
    std::uint32_t last_required_resource_count;
    std::uint32_t last_required_type_count;
    std::uint32_t last_required_dependency_edge_count;
    std::uint32_t load_commit_record_count;
    std::uint64_t load_commit_count;
    std::uint64_t loaded_resource_count;
    std::uint64_t failed_resource_count;
    std::uint64_t rejected_load_commit_count;
    std::uint64_t duplicate_load_commit_count;
    std::uint64_t invalid_load_transition_count;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status;
    ResourceStatus last_status;
    ResourceLoadState last_load_state;
    ResourceLoadCommitStatus last_load_commit_status;
    std::uint32_t last_required_load_commit_count = 0U;
    ResourceHandle last_failed_load_commit_resource;
    ResourceTypeId last_failed_load_commit_type;
    std::uint64_t last_failed_load_commit_id = 0U;
    std::uint64_t last_failed_load_upload_id = 0U;
    std::uint64_t last_failed_load_staging_request_id = 0U;
    std::uint32_t last_failed_load_upload_byte_count = 0U;
    ResourceLoadState last_failed_load_state = ResourceLoadState::Unloaded;
    std::uint32_t last_failed_load_commit_capacity = 0U;
    std::uint32_t last_failed_load_commit_count = 0U;
    std::uint32_t last_failed_required_load_commit_count = 0U;
};
}
