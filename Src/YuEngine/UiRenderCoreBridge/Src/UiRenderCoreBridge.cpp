// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Src/UiRenderCoreBridge.cpp

#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridge.h"

#include <cstdint>
#include <limits>
#include <span>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"

namespace yuengine::uirendercorebridge {
namespace {
bool IsKnownDrawElementType(yuengine::uicore::UiDrawElementType type) {
    if (type == yuengine::uicore::UiDrawElementType::Rect) {
        return true;
    }

    if (type == yuengine::uicore::UiDrawElementType::TexturedQuad) {
        return true;
    }

    return type == yuengine::uicore::UiDrawElementType::Text;
}

bool FindOutputCapacityFailureIndex(
    const UiRenderCoreBridgeRequest &request,
    std::size_t draw_count,
    std::size_t *out_failed_entry_index) {
    if (out_failed_entry_index == nullptr) {
        return false;
    }

    *out_failed_entry_index = draw_count;

    if (request.pass_requests.size() < draw_count) {
        *out_failed_entry_index = request.pass_requests.size();
    }

    if (request.pass_results.size() < *out_failed_entry_index) {
        *out_failed_entry_index = request.pass_results.size();
    }

    if (request.pass_requests.data() == nullptr) {
        *out_failed_entry_index = 0U;
    }

    if (request.pass_results.data() == nullptr) {
        *out_failed_entry_index = 0U;
    }

    return *out_failed_entry_index < draw_count;
}

bool HasValidPassIdRange(std::uint32_t pass_id_base, std::size_t draw_count) {
    if (pass_id_base == 0U) {
        return false;
    }

    const std::uint32_t max_value = std::numeric_limits<std::uint32_t>::max();
    const std::size_t remaining_count = static_cast<std::size_t>(max_value - pass_id_base) + 1U;
    return draw_count <= remaining_count;
}

std::uint32_t PassIdFor(std::uint32_t pass_id_base, std::size_t index) {
    return pass_id_base + static_cast<std::uint32_t>(index);
}
}

UiRenderCoreBridgeResult UiRenderCoreBridge::Submit(const UiRenderCoreBridgeRequest &request) {
    UiRenderCoreBridgeResult result{};
    result.draw_element_count = request.draw_elements.size();

    result.status = ValidateRequest(request, &result);
    if (result.status != UiRenderCoreBridgeStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    result.status = FillPassRequests(request, &result);
    if (result.status != UiRenderCoreBridgeStatus::Success) {
        RecordRejectedResult(result);
        return result;
    }

    yuengine::rendercore::RenderSubmissionBatchFixtureRequest batch_request{};
    batch_request.pass = request.pass;
    batch_request.pass_requests = std::span<const yuengine::rendercore::RenderFixturePassRequest>(
        request.pass_requests.data(),
        request.draw_elements.size());
    batch_request.pass_results = std::span<yuengine::rendercore::RenderFixturePassResult>(
        request.pass_results.data(),
        request.draw_elements.size());

    const yuengine::rendercore::RenderSubmissionBatchFixtureResult submission_result =
        request.submission_batch->Execute(batch_request);
    result.submission_status = submission_result.status;
    result.pass_status = submission_result.pass_status;
    result.rhi_status = submission_result.rhi_status;
    result.submitted_entry_count = submission_result.entry_count;
    result.completed_entry_count = submission_result.completed_entry_count;
    result.failed_entry_index = submission_result.failed_entry_index;
    if (submission_result.failed_entry_index < request.draw_elements.size()) {
        result.failed_node_id = request.draw_elements[submission_result.failed_entry_index].node_id;
    }

    if (submission_result.status != yuengine::rendercore::RenderSubmissionBatchFixtureStatus::Success) {
        result.status = UiRenderCoreBridgeStatus::SubmissionFailed;
        RecordSubmittedResult(result);
        return result;
    }

    result.status = UiRenderCoreBridgeStatus::Success;
    RecordSubmittedResult(result);
    return result;
}

UiRenderCoreBridgeSnapshot UiRenderCoreBridge::Snapshot() const {
    return snapshot_;
}

void UiRenderCoreBridge::Reset() {
    snapshot_ = {};
}

UiRenderCoreBridgeStatus UiRenderCoreBridge::ValidateRequest(
    const UiRenderCoreBridgeRequest &request,
    UiRenderCoreBridgeResult *result) const {
    if (result == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    if (request.pass == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    if (request.submission_batch == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    const std::size_t draw_count = request.draw_elements.size();
    if (draw_count == 0U) {
        return UiRenderCoreBridgeStatus::EmptyDrawList;
    }

    if (request.draw_elements.data() == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    std::size_t capacity_failed_entry_index = 0U;
    if (FindOutputCapacityFailureIndex(request, draw_count, &capacity_failed_entry_index)) {
        result->required_draw_record_count = draw_count;
        result->failed_entry_index = capacity_failed_entry_index;
        result->failed_node_id = request.draw_elements[capacity_failed_entry_index].node_id;
        return UiRenderCoreBridgeStatus::OutputCapacityExceeded;
    }

    if (!HasValidPassIdRange(request.pass_id_base, draw_count)) {
        return UiRenderCoreBridgeStatus::PassIdOverflow;
    }

    for (std::size_t index = 0U; index < draw_count; ++index) {
        const UiRenderCoreBridgeStatus status = ValidateDrawElement(request.draw_elements[index], index, result);
        if (status != UiRenderCoreBridgeStatus::Success) {
            return status;
        }
    }

    return UiRenderCoreBridgeStatus::Success;
}

UiRenderCoreBridgeStatus UiRenderCoreBridge::ValidateDrawElement(
    const yuengine::uicore::UiDrawElement &element,
    std::size_t index,
    UiRenderCoreBridgeResult *result) const {
    if (result == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    result->failed_entry_index = index;
    result->failed_node_id = element.node_id;
    if (element.node_id.value == 0U) {
        return UiRenderCoreBridgeStatus::InvalidDrawElement;
    }

    if (!IsKnownDrawElementType(element.type)) {
        return UiRenderCoreBridgeStatus::InvalidDrawElement;
    }

    if (element.material_key == 0U) {
        return UiRenderCoreBridgeStatus::InvalidDrawElement;
    }

    return UiRenderCoreBridgeStatus::Success;
}

UiRenderCoreBridgeStatus UiRenderCoreBridge::FillPassRequests(
    const UiRenderCoreBridgeRequest &request,
    UiRenderCoreBridgeResult *result) const {
    if (result == nullptr) {
        return UiRenderCoreBridgeStatus::InvalidArgument;
    }

    const std::size_t draw_count = request.draw_elements.size();
    for (std::size_t index = 0U; index < draw_count; ++index) {
        FillPassRequest(request, request.draw_elements[index], index);
    }

    result->failed_entry_index = 0U;
    result->failed_node_id = yuengine::uicore::UiNodeId{};
    return UiRenderCoreBridgeStatus::Success;
}

void UiRenderCoreBridge::FillPassRequest(
    const UiRenderCoreBridgeRequest &request,
    const yuengine::uicore::UiDrawElement &element,
    std::size_t index) const {
    yuengine::rendercore::RenderFixturePassRequest &pass_request = request.pass_requests[index];
    pass_request = request.template_pass_request;
    pass_request.pass_id = PassIdFor(request.pass_id_base, index);
    pass_request.material_id = element.material_key;
    pass_request.material_constant_byte_count = 0U;
}

void UiRenderCoreBridge::RecordRejectedResult(const UiRenderCoreBridgeResult &result) {
    ++snapshot_.failed_validation_count;
    snapshot_.last_draw_element_count = result.draw_element_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_required_draw_record_count = result.required_draw_record_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_failed_node_id = result.failed_node_id;
    snapshot_.last_status = result.status;
    snapshot_.last_submission_status = result.submission_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;
}

void UiRenderCoreBridge::RecordSubmittedResult(const UiRenderCoreBridgeResult &result) {
    snapshot_.accepted_draw_count += result.draw_element_count;
    snapshot_.submitted_draw_count += result.submitted_entry_count;
    snapshot_.completed_draw_count += result.completed_entry_count;
    snapshot_.last_draw_element_count = result.draw_element_count;
    snapshot_.last_completed_entry_count = result.completed_entry_count;
    snapshot_.last_required_draw_record_count = result.required_draw_record_count;
    snapshot_.last_failed_entry_index = result.failed_entry_index;
    snapshot_.last_failed_node_id = result.failed_node_id;
    snapshot_.last_status = result.status;
    snapshot_.last_submission_status = result.submission_status;
    snapshot_.last_pass_status = result.pass_status;
    snapshot_.last_rhi_status = result.rhi_status;

    if (result.status == UiRenderCoreBridgeStatus::Success) {
        return;
    }

    ++snapshot_.submission_failure_count;
}
}
