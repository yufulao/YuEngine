// 模块: Tests UiRuntime
// 文件: Tests/UiRuntime/UiProjectRuntimeSmokeSampleTests.cpp

#include <cstdio>
#include <string_view>

#include "YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSample.h"
#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleResult.h"
#include "YuEngine/UiRuntime/UiProjectRuntimeSmokeSampleStatus.h"

using yuengine::uiruntime::UiManagerFullscreenStackStatus;
using yuengine::uiruntime::UiManagerPanelMapStatus;
using yuengine::uiruntime::UiManagerPopupStackStatus;
using yuengine::uiruntime::UiProjectRuntimeSmokeSample;
using yuengine::uiruntime::UiProjectRuntimeSmokeSampleResult;
using yuengine::uiruntime::UiProjectRuntimeSmokeSampleStatus;

namespace {
constexpr const char *TEST_RUN_PASS =
    "UiRuntime_ProjectRuntimeSmokeSample_OpensPopupFullscreenGridAndReportsPass";
constexpr const char *TEST_INVALID_OUTPUT =
    "UiRuntime_ProjectRuntimeSmokeSample_InvalidOutputReportsFailureStatus";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int FailStatus(std::string_view message, UiProjectRuntimeSmokeSampleStatus status) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fprintf(stderr, ": %d\n", static_cast<int>(status));
    return 1;
}

int RunOpenPopupFullscreenGridAndReportsPassTest() {
    UiProjectRuntimeSmokeSample sample;
    UiProjectRuntimeSmokeSampleResult result{};
    const UiProjectRuntimeSmokeSampleStatus status = sample.Run(&result);
    if (status != UiProjectRuntimeSmokeSampleStatus::Success) {
        return FailStatus("smoke sample status mismatch", status);
    }

    if (!result.Succeeded() || !result.passed || !result.cleanup_passed) {
        return Fail("smoke sample pass flags mismatch");
    }

    if (!result.popup_opened ||
        !result.popup_displayed ||
        !result.popup_closed ||
        !result.popup_released) {
        return Fail("popup smoke flags mismatch");
    }

    if (!result.fullscreen_opened ||
        !result.fullscreen_top_displayed ||
        !result.fullscreen_back_restored ||
        !result.fullscreen_released) {
        return Fail("fullscreen smoke flags mismatch");
    }

    if (!result.grid_opened ||
        !result.grid_displayed ||
        !result.grid_visible_data_read ||
        !result.grid_released) {
        return Fail("grid smoke flags mismatch");
    }

    if (result.smoke_step_count != 3U ||
        result.popup_open_display_count != 1U ||
        result.fullscreen_open_display_count != 3U ||
        result.grid_open_display_count != 1U ||
        result.grid_visible_item_count == 0U ||
        result.released_panel_count != 4U) {
        return Fail("smoke counters mismatch");
    }

    if (result.panel_map_snapshot.loaded_panel_count != 0U ||
        result.panel_map_snapshot.active_panel_count != 0U ||
        result.panel_map_snapshot.release_operation_count != 4U ||
        result.panel_map_snapshot.last_status != UiManagerPanelMapStatus::Success) {
        return Fail("panel map cleanup snapshot mismatch");
    }

    if (result.popup_stack_snapshot.popup_count != 0U ||
        result.popup_stack_snapshot.top_panel_id.IsValid() ||
        result.popup_stack_snapshot.open_operation_count != 1U ||
        result.popup_stack_snapshot.close_operation_count != 1U ||
        result.popup_stack_snapshot.release_operation_count != 1U ||
        result.popup_stack_snapshot.last_status != UiManagerPopupStackStatus::Success) {
        return Fail("popup stack cleanup snapshot mismatch");
    }

    if (result.fullscreen_stack_snapshot.fullscreen_count != 0U ||
        result.fullscreen_stack_snapshot.top_panel_id.IsValid() ||
        result.fullscreen_stack_snapshot.open_operation_count != 2U ||
        result.fullscreen_stack_snapshot.back_navigation_operation_count != 1U ||
        result.fullscreen_stack_snapshot.release_operation_count != 1U ||
        result.fullscreen_stack_snapshot.last_status != UiManagerFullscreenStackStatus::Success) {
        return Fail("fullscreen stack cleanup snapshot mismatch");
    }

    return 0;
}

int RunInvalidOutputReportsFailureStatusTest() {
    UiProjectRuntimeSmokeSample sample;
    const UiProjectRuntimeSmokeSampleStatus status = sample.Run(nullptr);
    if (status == UiProjectRuntimeSmokeSampleStatus::InvalidOutputBuffer) {
        return 0;
    }

    return Fail("invalid output status mismatch");
}

int RunNamedTest(std::string_view test_name) {
    if (test_name == TEST_RUN_PASS) {
        return RunOpenPopupFullscreenGridAndReportsPassTest();
    }

    if (test_name == TEST_INVALID_OUTPUT) {
        return RunInvalidOutputReportsFailureStatusTest();
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
