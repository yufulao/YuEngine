// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneCameraFrameBinder.h

#pragma once

#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"

namespace yuengine::renderscene {
class RenderSceneCameraFrameBinder final {
public:
    /**
     * @comment 从 runtime camera record 构建 RenderScene camera frame 和 capture metadata。
     * @param request 输入 camera binding request。
     * @param out_result 调用方持有的输出结果。
     * @return 显式绑定状态。
     */
    RenderSceneStatus BuildActiveCameraFrame(
        const RenderSceneCameraBindingRequest &request,
        RenderSceneCameraBindingResult *out_result);

private:
    const RenderSceneRuntimeCameraRecord *FindActiveCamera(
        const RenderSceneCameraBindingRequest &request) const;
    RenderSceneStatus MapCameraStatus(yuengine::rendercore::RenderCameraStatus status) const;

    yuengine::rendercore::RenderCamera camera_{};
};
}
