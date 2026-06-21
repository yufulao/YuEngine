// 模块: YuRuntimeVisualCameraTweenViewer
// 文件: Apps/YuRuntimeVisualCameraTweenViewer/Main.cpp

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <limits>
#include <string_view>
#include <thread>
#include <vector>

#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowEventType.h"
#include "YuEngine/Platform/PlatformWindowPollResult.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"
#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiBlendUtility.h"
#include "YuEngine/Rhi/RhiColor.h"

using PlatformNativeSurface = yuengine::platform::PlatformNativeSurface;
using PlatformWindowDesc = yuengine::platform::PlatformWindowDesc;
using PlatformWindowEvent = yuengine::platform::PlatformWindowEvent;
using PlatformWindowEventType = yuengine::platform::PlatformWindowEventType;
using PlatformWindowPollResult = yuengine::platform::PlatformWindowPollResult;
using PlatformWindowStatus = yuengine::platform::PlatformWindowStatus;
using WindowsPlatformWindow = yuengine::platform::WindowsPlatformWindow;
using RhiBlendMode = yuengine::rhi::RhiBlendMode;
using RhiBlendStateDesc = yuengine::rhi::RhiBlendStateDesc;
using RhiColor = yuengine::rhi::RhiColor;
using yuengine::rhi::BlendRhiColor;

namespace {
constexpr int VIEWER_WIDTH = 960;
constexpr int VIEWER_HEIGHT = 540;
constexpr int RENDER_WIDTH = 640;
constexpr int RENDER_HEIGHT = 360;
constexpr int MAX_RUN_FRAMES = 3600;
constexpr int ESCAPE_KEY_CODE = 27;
constexpr float PI_VALUE = 3.14159265359F;
constexpr float CAMERA_TWEEN_DURATION_SECONDS = 6.0F;
constexpr float RASTER_EPSILON = 0.0001F;
constexpr float FAR_DEPTH = 1000000.0F;
constexpr std::uint32_t CYLINDER_SEGMENT_COUNT = 18U;
constexpr std::uint32_t CONE_SEGMENT_COUNT = 18U;
constexpr std::size_t SURFACE_VERTEX_CAPACITY = 4U;
constexpr std::size_t SURFACE_CAPACITY = 96U;
constexpr int TRANSPARENT_PANEL_MIN_X = 196;
constexpr int TRANSPARENT_PANEL_MAX_X = 444;
constexpr int TRANSPARENT_PANEL_MIN_Y = 100;
constexpr int TRANSPARENT_PANEL_MAX_Y = 255;
constexpr std::uint8_t TRANSPARENT_PANEL_ALPHA = 128U;
constexpr const char *USAGE_TEXT =
    "Usage: YuRuntimeVisualCameraTweenViewer [--frames <count>]\n";

struct Vec3 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

struct Rgb final {
    std::uint8_t r = 0U;
    std::uint8_t g = 0U;
    std::uint8_t b = 0U;
};

struct CameraPose final {
    Vec3 position{};
    Vec3 target{};
    Vec3 up{0.0F, 1.0F, 0.0F};
    float vertical_fov_radians = 1.0F;
};

enum class TweenEase {
    Linear,
    SmoothStep
};

struct CameraKeyframe final {
    float time_seconds = 0.0F;
    CameraPose pose{};
    TweenEase ease = TweenEase::Linear;
};

struct CameraBasis final {
    Vec3 position{};
    Vec3 right{};
    Vec3 up{};
    Vec3 forward{};
    float x_scale = 1.0F;
    float y_scale = 1.0F;
    float near_z = 0.1F;
};

struct ProjectedVertex final {
    float x = 0.0F;
    float y = 0.0F;
    float depth = 0.0F;
    bool valid = false;
};

struct Surface final {
    std::array<ProjectedVertex, SURFACE_VERTEX_CAPACITY> vertices{};
    std::size_t vertex_count = 0U;
    Rgb color{};
    float depth = FAR_DEPTH;
};

struct Transform final {
    Vec3 translation{};
    Vec3 rotation{};
    Vec3 scale{1.0F, 1.0F, 1.0F};
};

struct FrameBuffer final {
    std::vector<std::uint32_t> pixels{};
    std::vector<float> depths{};
};

struct RunOptions final {
    std::uint32_t max_frame_count = 0U;
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Blocker(std::string_view layer, std::string_view detail) {
    std::fprintf(stderr, "RVF018 BLOCKER layer=%.*s detail=%.*s\n",
        static_cast<int>(layer.size()),
        layer.data(),
        static_cast<int>(detail.size()),
        detail.data());
    return 77;
}

bool ParseUnsigned(std::string_view text, std::uint32_t *out_value) {
    if (out_value == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    std::uint32_t value = 0U;
    for (char character : text) {
        if (character < '0' || character > '9') {
            return false;
        }

        const std::uint32_t digit = static_cast<std::uint32_t>(character - '0');
        const std::uint32_t limit = std::numeric_limits<std::uint32_t>::max() / 10U;
        if (value > limit) {
            return false;
        }

        value *= 10U;
        const std::uint32_t digit_limit = std::numeric_limits<std::uint32_t>::max() - value;
        if (digit > digit_limit) {
            return false;
        }

        value += digit;
    }

    *out_value = value;
    return true;
}

bool ParseRunOptions(int argc, char **argv, RunOptions *out_options) {
    if (out_options == nullptr) {
        return false;
    }

    RunOptions options{};
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument != "--frames") {
            return false;
        }

        if (index + 1 >= argc) {
            return false;
        }

        ++index;
        if (!ParseUnsigned(argv[index], &options.max_frame_count)) {
            return false;
        }

        if (options.max_frame_count > MAX_RUN_FRAMES) {
            return false;
        }
    }

    *out_options = options;
    return true;
}

Vec3 MakeVec3(float x, float y, float z) {
    Vec3 value{};
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

Vec3 AddVec3(Vec3 left, Vec3 right) {
    return MakeVec3(left.x + right.x, left.y + right.y, left.z + right.z);
}

Vec3 SubtractVec3(Vec3 left, Vec3 right) {
    return MakeVec3(left.x - right.x, left.y - right.y, left.z - right.z);
}

Vec3 ScaleVec3(Vec3 value, float scale) {
    return MakeVec3(value.x * scale, value.y * scale, value.z * scale);
}

float DotVec3(Vec3 left, Vec3 right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 CrossVec3(Vec3 left, Vec3 right) {
    Vec3 result{};
    result.x = left.y * right.z - left.z * right.y;
    result.y = left.z * right.x - left.x * right.z;
    result.z = left.x * right.y - left.y * right.x;
    return result;
}

float LengthVec3(Vec3 value) {
    return std::sqrt(DotVec3(value, value));
}

Vec3 NormalizeVec3(Vec3 value) {
    const float length = LengthVec3(value);
    if (length <= RASTER_EPSILON) {
        return MakeVec3(0.0F, 0.0F, 0.0F);
    }

    return ScaleVec3(value, 1.0F / length);
}

float ClampFloat(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

float LerpFloat(float first, float second, float t) {
    return first + (second - first) * t;
}

Vec3 LerpVec3(Vec3 first, Vec3 second, float t) {
    return MakeVec3(
        LerpFloat(first.x, second.x, t),
        LerpFloat(first.y, second.y, t),
        LerpFloat(first.z, second.z, t));
}

float EvaluateEase(TweenEase ease, float linear_t) {
    const float clamped_t = ClampFloat(linear_t, 0.0F, 1.0F);
    if (ease == TweenEase::SmoothStep) {
        return clamped_t * clamped_t * (3.0F - 2.0F * clamped_t);
    }

    return clamped_t;
}

CameraPose LerpCameraPose(
    const CameraPose &first,
    const CameraPose &second,
    float t) {
    CameraPose result{};
    result.position = LerpVec3(first.position, second.position, t);
    result.target = LerpVec3(first.target, second.target, t);
    result.up = LerpVec3(first.up, second.up, t);
    result.vertical_fov_radians =
        LerpFloat(first.vertical_fov_radians, second.vertical_fov_radians, t);
    return result;
}

std::array<CameraKeyframe, 3U> BuildCameraKeyframes() {
    std::array<CameraKeyframe, 3U> keyframes{};
    keyframes[0U].time_seconds = 0.0F;
    keyframes[0U].pose.position = MakeVec3(-4.7F, 2.4F, -5.2F);
    keyframes[0U].pose.target = MakeVec3(-0.2F, 0.6F, 0.1F);
    keyframes[0U].pose.vertical_fov_radians = 1.08F;
    keyframes[0U].ease = TweenEase::SmoothStep;

    keyframes[1U].time_seconds = CAMERA_TWEEN_DURATION_SECONDS * 0.57F;
    keyframes[1U].pose.position = MakeVec3(2.4F, 3.1F, -3.4F);
    keyframes[1U].pose.target = MakeVec3(0.1F, 0.7F, 0.0F);
    keyframes[1U].pose.vertical_fov_radians = 0.82F;
    keyframes[1U].ease = TweenEase::Linear;

    keyframes[2U].time_seconds = CAMERA_TWEEN_DURATION_SECONDS;
    keyframes[2U].pose.position = MakeVec3(4.6F, 2.0F, 3.8F);
    keyframes[2U].pose.target = MakeVec3(0.3F, 0.5F, -0.2F);
    keyframes[2U].pose.vertical_fov_radians = 1.16F;
    keyframes[2U].ease = TweenEase::SmoothStep;
    return keyframes;
}

CameraPose SampleCameraTween(float time_seconds) {
    const std::array<CameraKeyframe, 3U> keyframes = BuildCameraKeyframes();
    float wrapped_time = std::fmod(time_seconds, CAMERA_TWEEN_DURATION_SECONDS);
    if (wrapped_time < 0.0F) {
        wrapped_time = 0.0F;
    }

    std::size_t source_index = 0U;
    std::size_t target_index = 1U;
    if (wrapped_time >= keyframes[1U].time_seconds) {
        source_index = 1U;
        target_index = 2U;
    }

    const CameraKeyframe &source = keyframes[source_index];
    const CameraKeyframe &target = keyframes[target_index];
    const float duration = target.time_seconds - source.time_seconds;
    float linear_t = 0.0F;
    if (duration > RASTER_EPSILON) {
        linear_t = (wrapped_time - source.time_seconds) / duration;
    }

    const float eased_t = EvaluateEase(source.ease, linear_t);
    return LerpCameraPose(source.pose, target.pose, eased_t);
}

Vec3 RotateX(Vec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    Vec3 result{};
    result.x = value.x;
    result.y = value.y * cosine_value - value.z * sine_value;
    result.z = value.y * sine_value + value.z * cosine_value;
    return result;
}

Vec3 RotateY(Vec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    Vec3 result{};
    result.x = value.x * cosine_value + value.z * sine_value;
    result.y = value.y;
    result.z = -value.x * sine_value + value.z * cosine_value;
    return result;
}

Vec3 RotateZ(Vec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    Vec3 result{};
    result.x = value.x * cosine_value - value.y * sine_value;
    result.y = value.x * sine_value + value.y * cosine_value;
    result.z = value.z;
    return result;
}

Vec3 TransformPoint(Vec3 value, const Transform &transform) {
    Vec3 result{};
    result.x = value.x * transform.scale.x;
    result.y = value.y * transform.scale.y;
    result.z = value.z * transform.scale.z;
    result = RotateX(result, transform.rotation.x);
    result = RotateY(result, transform.rotation.y);
    result = RotateZ(result, transform.rotation.z);
    result = AddVec3(result, transform.translation);
    return result;
}

CameraBasis BuildCameraBasis(const CameraPose &pose) {
    CameraBasis basis{};
    basis.position = pose.position;
    basis.forward = NormalizeVec3(SubtractVec3(pose.target, pose.position));
    basis.right = NormalizeVec3(CrossVec3(pose.up, basis.forward));
    basis.up = CrossVec3(basis.forward, basis.right);
    const float y_scale = 1.0F / std::tan(pose.vertical_fov_radians * 0.5F);
    basis.y_scale = y_scale;
    basis.x_scale = y_scale / 1.77777777778F;
    basis.near_z = 0.1F;
    return basis;
}

bool ProjectPoint(
    const CameraBasis &basis,
    Vec3 world_point,
    ProjectedVertex *out_vertex) {
    if (out_vertex == nullptr) {
        return false;
    }

    const Vec3 relative = SubtractVec3(world_point, basis.position);
    const float view_z = DotVec3(relative, basis.forward);
    if (view_z <= basis.near_z) {
        return false;
    }

    const float view_x = DotVec3(relative, basis.right);
    const float view_y = DotVec3(relative, basis.up);
    const float ndc_x = (view_x * basis.x_scale) / view_z;
    const float ndc_y = (view_y * basis.y_scale) / view_z;
    ProjectedVertex vertex{};
    vertex.x = (ndc_x * 0.5F + 0.5F) * static_cast<float>(RENDER_WIDTH);
    vertex.y = (0.5F - ndc_y * 0.5F) * static_cast<float>(RENDER_HEIGHT);
    vertex.depth = view_z;
    vertex.valid = true;
    *out_vertex = vertex;
    return true;
}

std::uint8_t ClampByte(std::uint32_t value) {
    if (value > 255U) {
        return 255U;
    }

    return static_cast<std::uint8_t>(value);
}

Rgb ShadeColor(Rgb color, float shade, std::uint32_t bias) {
    Rgb result{};
    const std::uint32_t red_value =
        static_cast<std::uint32_t>(static_cast<float>(color.r) * shade) + bias;
    const std::uint32_t green_value =
        static_cast<std::uint32_t>(static_cast<float>(color.g) * shade) + bias / 2U;
    const std::uint32_t blue_value =
        static_cast<std::uint32_t>(static_cast<float>(color.b) * shade) + bias / 3U;
    result.r = ClampByte(red_value);
    result.g = ClampByte(green_value);
    result.b = ClampByte(blue_value);
    return result;
}

Rgb MaterialColor(std::size_t slot, std::size_t surface_index, float shade) {
    const std::array<Rgb, 3U> base_colors{
        Rgb{72U, 240U, 92U},
        Rgb{255U, 96U, 72U},
        Rgb{86U, 112U, 255U}};
    const std::size_t color_index = (slot + surface_index) % base_colors.size();
    const std::uint32_t bias = static_cast<std::uint32_t>(surface_index * 9U);
    return ShadeColor(base_colors[color_index], shade, bias);
}

std::uint32_t PackBgr32(Rgb color) {
    return static_cast<std::uint32_t>(color.b) |
        (static_cast<std::uint32_t>(color.g) << 8U) |
        (static_cast<std::uint32_t>(color.r) << 16U);
}

std::uint32_t PackBgr32(RhiColor color) {
    return static_cast<std::uint32_t>(color.b) |
        (static_cast<std::uint32_t>(color.g) << 8U) |
        (static_cast<std::uint32_t>(color.r) << 16U);
}

RhiColor UnpackBgr32(std::uint32_t pixel) {
    RhiColor color{};
    color.b = static_cast<std::uint8_t>(pixel & 0xFFU);
    color.g = static_cast<std::uint8_t>((pixel >> 8U) & 0xFFU);
    color.r = static_cast<std::uint8_t>((pixel >> 16U) & 0xFFU);
    color.a = 255U;
    return color;
}

RhiBlendStateDesc BuildTransparentPanelBlendState() {
    RhiBlendStateDesc blend_state{};
    blend_state.mode = RhiBlendMode::AlphaOver;
    blend_state.constant_alpha = 255U;
    return blend_state;
}

RhiColor TransparentPanelSourceColor(int row, int column, float time_seconds) {
    const int time_phase = static_cast<int>(time_seconds * 5.0F);
    const int stripe_index = ((column / 18) + (row / 18) + time_phase) % 3;
    if (stripe_index == 0) {
        return RhiColor{236U, 216U, 48U, TRANSPARENT_PANEL_ALPHA};
    }

    if (stripe_index == 1) {
        return RhiColor{64U, 198U, 255U, 116U};
    }

    return RhiColor{255U, 96U, 168U, 140U};
}

void RasterTransparentPanel(FrameBuffer *frame_buffer, float time_seconds) {
    if (frame_buffer == nullptr) {
        return;
    }

    const RhiBlendStateDesc blend_state = BuildTransparentPanelBlendState();
    for (int row = TRANSPARENT_PANEL_MIN_Y; row <= TRANSPARENT_PANEL_MAX_Y; ++row) {
        for (int column = TRANSPARENT_PANEL_MIN_X; column <= TRANSPARENT_PANEL_MAX_X; ++column) {
            const std::size_t offset =
                static_cast<std::size_t>(row) * static_cast<std::size_t>(RENDER_WIDTH) +
                static_cast<std::size_t>(column);
            const RhiColor destination = UnpackBgr32(frame_buffer->pixels[offset]);
            const RhiColor source = TransparentPanelSourceColor(row, column, time_seconds);
            const RhiColor blended = BlendRhiColor(source, destination, blend_state);
            frame_buffer->pixels[offset] = PackBgr32(blended);
        }
    }
}

bool AddSurface(
    const CameraBasis &basis,
    std::span<const Vec3> vertices,
    Rgb color,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    if (out_surfaces == nullptr) {
        return false;
    }

    if (inout_surface_count == nullptr) {
        return false;
    }

    if (vertices.size() < 3U || vertices.size() > SURFACE_VERTEX_CAPACITY) {
        return false;
    }

    if (*inout_surface_count >= out_surfaces->size()) {
        return false;
    }

    Surface surface{};
    surface.vertex_count = vertices.size();
    surface.color = color;
    float depth_sum = 0.0F;
    for (std::size_t index = 0U; index < vertices.size(); ++index) {
        ProjectedVertex vertex{};
        if (!ProjectPoint(basis, vertices[index], &vertex)) {
            return true;
        }

        surface.vertices[index] = vertex;
        depth_sum += vertex.depth;
    }

    surface.depth = depth_sum / static_cast<float>(surface.vertex_count);
    (*out_surfaces)[*inout_surface_count] = surface;
    ++(*inout_surface_count);
    return true;
}

bool AddQuad(
    const CameraBasis &basis,
    const std::array<Vec3, 4U> &vertices,
    Rgb color,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const std::span<const Vec3> vertex_span(vertices.data(), vertices.size());
    return AddSurface(basis, vertex_span, color, out_surfaces, inout_surface_count);
}

bool AddTriangle(
    const CameraBasis &basis,
    const std::array<Vec3, 3U> &vertices,
    Rgb color,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const std::span<const Vec3> vertex_span(vertices.data(), vertices.size());
    return AddSurface(basis, vertex_span, color, out_surfaces, inout_surface_count);
}

bool BuildCube(
    const CameraBasis &basis,
    const Transform &transform,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const std::array<Vec3, 8U> local{
        MakeVec3(-0.6F, -0.6F, -0.6F),
        MakeVec3(0.6F, -0.6F, -0.6F),
        MakeVec3(0.6F, 0.6F, -0.6F),
        MakeVec3(-0.6F, 0.6F, -0.6F),
        MakeVec3(-0.6F, -0.6F, 0.6F),
        MakeVec3(0.6F, -0.6F, 0.6F),
        MakeVec3(0.6F, 0.6F, 0.6F),
        MakeVec3(-0.6F, 0.6F, 0.6F)};
    std::array<Vec3, 8U> world{};
    for (std::size_t index = 0U; index < local.size(); ++index) {
        world[index] = TransformPoint(local[index], transform);
    }

    const std::array<std::array<std::size_t, 4U>, 6U> faces{
        std::array<std::size_t, 4U>{0U, 1U, 2U, 3U},
        std::array<std::size_t, 4U>{5U, 4U, 7U, 6U},
        std::array<std::size_t, 4U>{4U, 0U, 3U, 7U},
        std::array<std::size_t, 4U>{1U, 5U, 6U, 2U},
        std::array<std::size_t, 4U>{3U, 2U, 6U, 7U},
        std::array<std::size_t, 4U>{4U, 5U, 1U, 0U}};
    for (std::size_t face_index = 0U; face_index < faces.size(); ++face_index) {
        const std::array<std::size_t, 4U> &face = faces[face_index];
        const std::array<Vec3, 4U> vertices{
            world[face[0U]],
            world[face[1U]],
            world[face[2U]],
            world[face[3U]]};
        const Rgb color = MaterialColor(0U, face_index, 0.78F + static_cast<float>(face_index) * 0.04F);
        if (!AddQuad(basis, vertices, color, out_surfaces, inout_surface_count)) {
            return false;
        }
    }

    return true;
}

bool BuildCylinder(
    const CameraBasis &basis,
    const Transform &transform,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const Vec3 top_center = TransformPoint(MakeVec3(0.0F, 0.75F, 0.0F), transform);
    const Vec3 bottom_center = TransformPoint(MakeVec3(0.0F, -0.75F, 0.0F), transform);
    for (std::uint32_t segment = 0U; segment < CYLINDER_SEGMENT_COUNT; ++segment) {
        const float first_angle = 2.0F * PI_VALUE * static_cast<float>(segment) /
            static_cast<float>(CYLINDER_SEGMENT_COUNT);
        const float second_angle = 2.0F * PI_VALUE * static_cast<float>(segment + 1U) /
            static_cast<float>(CYLINDER_SEGMENT_COUNT);
        const Vec3 bottom_first = TransformPoint(
            MakeVec3(std::cos(first_angle) * 0.48F, -0.75F, std::sin(first_angle) * 0.48F),
            transform);
        const Vec3 bottom_second = TransformPoint(
            MakeVec3(std::cos(second_angle) * 0.48F, -0.75F, std::sin(second_angle) * 0.48F),
            transform);
        const Vec3 top_first = TransformPoint(
            MakeVec3(std::cos(first_angle) * 0.48F, 0.75F, std::sin(first_angle) * 0.48F),
            transform);
        const Vec3 top_second = TransformPoint(
            MakeVec3(std::cos(second_angle) * 0.48F, 0.75F, std::sin(second_angle) * 0.48F),
            transform);
        const float shade = 0.64F + 0.32F * static_cast<float>(segment % 3U) / 2.0F;
        const Rgb side_color = MaterialColor(1U, segment, shade);
        const std::array<Vec3, 4U> side{bottom_first, bottom_second, top_second, top_first};
        if (!AddQuad(basis, side, side_color, out_surfaces, inout_surface_count)) {
            return false;
        }

        const std::array<Vec3, 3U> top{top_center, top_first, top_second};
        const std::array<Vec3, 3U> bottom{bottom_center, bottom_second, bottom_first};
        const Rgb top_color = MaterialColor(1U, segment + 1U, 0.86F);
        const Rgb bottom_color = MaterialColor(1U, segment + 2U, 0.54F);
        if (!AddTriangle(basis, top, top_color, out_surfaces, inout_surface_count)) {
            return false;
        }

        if (!AddTriangle(basis, bottom, bottom_color, out_surfaces, inout_surface_count)) {
            return false;
        }
    }

    return true;
}

bool BuildCone(
    const CameraBasis &basis,
    const Transform &transform,
    std::array<Surface, SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const Vec3 apex = TransformPoint(MakeVec3(0.0F, 0.95F, 0.0F), transform);
    const Vec3 base_center = TransformPoint(MakeVec3(0.0F, -0.65F, 0.0F), transform);
    for (std::uint32_t segment = 0U; segment < CONE_SEGMENT_COUNT; ++segment) {
        const float first_angle = 2.0F * PI_VALUE * static_cast<float>(segment) /
            static_cast<float>(CONE_SEGMENT_COUNT);
        const float second_angle = 2.0F * PI_VALUE * static_cast<float>(segment + 1U) /
            static_cast<float>(CONE_SEGMENT_COUNT);
        const Vec3 first = TransformPoint(
            MakeVec3(std::cos(first_angle) * 0.58F, -0.65F, std::sin(first_angle) * 0.58F),
            transform);
        const Vec3 second = TransformPoint(
            MakeVec3(std::cos(second_angle) * 0.58F, -0.65F, std::sin(second_angle) * 0.58F),
            transform);
        const float shade = 0.58F + 0.38F * static_cast<float>(segment % 4U) / 3.0F;
        const std::array<Vec3, 3U> side{apex, first, second};
        const Rgb side_color = MaterialColor(2U, segment, shade);
        if (!AddTriangle(basis, side, side_color, out_surfaces, inout_surface_count)) {
            return false;
        }

        const std::array<Vec3, 3U> base{base_center, second, first};
        const Rgb base_color = MaterialColor(2U, segment + 2U, 0.50F);
        if (!AddTriangle(basis, base, base_color, out_surfaces, inout_surface_count)) {
            return false;
        }
    }

    return true;
}

float EdgeFunction(
    const ProjectedVertex &left,
    const ProjectedVertex &right,
    float x,
    float y) {
    return (x - left.x) * (right.y - left.y) - (y - left.y) * (right.x - left.x);
}

bool PointInTriangle(
    const ProjectedVertex &first,
    const ProjectedVertex &second,
    const ProjectedVertex &third,
    float x,
    float y) {
    const float edge0 = EdgeFunction(first, second, x, y);
    const float edge1 = EdgeFunction(second, third, x, y);
    const float edge2 = EdgeFunction(third, first, x, y);
    if (edge0 >= -RASTER_EPSILON &&
        edge1 >= -RASTER_EPSILON &&
        edge2 >= -RASTER_EPSILON) {
        return true;
    }

    if (edge0 <= RASTER_EPSILON &&
        edge1 <= RASTER_EPSILON &&
        edge2 <= RASTER_EPSILON) {
        return true;
    }

    return false;
}

bool PointInSurface(const Surface &surface, float x, float y) {
    if (surface.vertex_count == 3U) {
        return PointInTriangle(surface.vertices[0U], surface.vertices[1U], surface.vertices[2U], x, y);
    }

    if (surface.vertex_count == 4U) {
        if (PointInTriangle(surface.vertices[0U], surface.vertices[1U], surface.vertices[2U], x, y)) {
            return true;
        }

        return PointInTriangle(surface.vertices[0U], surface.vertices[2U], surface.vertices[3U], x, y);
    }

    return false;
}

float DistanceToSegment(
    const ProjectedVertex &left,
    const ProjectedVertex &right,
    float x,
    float y) {
    const float dx = right.x - left.x;
    const float dy = right.y - left.y;
    const float length_square = dx * dx + dy * dy;
    if (length_square <= RASTER_EPSILON) {
        const float px = x - left.x;
        const float py = y - left.y;
        return std::sqrt(px * px + py * py);
    }

    const float raw_t = ((x - left.x) * dx + (y - left.y) * dy) / length_square;
    const float clamped_t = ClampFloat(raw_t, 0.0F, 1.0F);
    const float closest_x = left.x + dx * clamped_t;
    const float closest_y = left.y + dy * clamped_t;
    const float px = x - closest_x;
    const float py = y - closest_y;
    return std::sqrt(px * px + py * py);
}

bool PointNearSurfaceEdge(const Surface &surface, float x, float y) {
    for (std::size_t index = 0U; index < surface.vertex_count; ++index) {
        const std::size_t next_index = (index + 1U) % surface.vertex_count;
        const float distance = DistanceToSegment(surface.vertices[index], surface.vertices[next_index], x, y);
        if (distance <= 1.15F) {
            return true;
        }
    }

    return false;
}

Rgb DarkenColor(Rgb color) {
    Rgb result{};
    result.r = static_cast<std::uint8_t>(static_cast<std::uint32_t>(color.r) / 3U);
    result.g = static_cast<std::uint8_t>(static_cast<std::uint32_t>(color.g) / 3U);
    result.b = static_cast<std::uint8_t>(static_cast<std::uint32_t>(color.b) / 3U);
    return result;
}

void FillBackground(FrameBuffer *frame_buffer, float time_seconds) {
    if (frame_buffer == nullptr) {
        return;
    }

    for (int row = 0; row < RENDER_HEIGHT; ++row) {
        const float v = static_cast<float>(row) / static_cast<float>(RENDER_HEIGHT);
        for (int column = 0; column < RENDER_WIDTH; ++column) {
            const float u = static_cast<float>(column) / static_cast<float>(RENDER_WIDTH);
            const std::uint8_t red = static_cast<std::uint8_t>(6U + static_cast<std::uint32_t>(u * 18.0F));
            const std::uint8_t green = static_cast<std::uint8_t>(12U + static_cast<std::uint32_t>(v * 26.0F));
            const std::uint8_t blue = static_cast<std::uint8_t>(18U + static_cast<std::uint32_t>(std::sin(time_seconds) * 4.0F + 4.0F));
            const std::size_t offset =
                static_cast<std::size_t>(row) * static_cast<std::size_t>(RENDER_WIDTH) +
                static_cast<std::size_t>(column);
            frame_buffer->pixels[offset] = PackBgr32(Rgb{red, green, blue});
            frame_buffer->depths[offset] = FAR_DEPTH;
        }
    }
}

void RasterSurface(const Surface &surface, FrameBuffer *frame_buffer) {
    if (frame_buffer == nullptr) {
        return;
    }

    float min_x = static_cast<float>(RENDER_WIDTH - 1);
    float max_x = 0.0F;
    float min_y = static_cast<float>(RENDER_HEIGHT - 1);
    float max_y = 0.0F;
    for (std::size_t index = 0U; index < surface.vertex_count; ++index) {
        const ProjectedVertex &vertex = surface.vertices[index];
        min_x = std::min(min_x, vertex.x);
        max_x = std::max(max_x, vertex.x);
        min_y = std::min(min_y, vertex.y);
        max_y = std::max(max_y, vertex.y);
    }

    const int begin_x = std::max(0, static_cast<int>(std::floor(min_x)) - 2);
    const int end_x = std::min(RENDER_WIDTH - 1, static_cast<int>(std::ceil(max_x)) + 2);
    const int begin_y = std::max(0, static_cast<int>(std::floor(min_y)) - 2);
    const int end_y = std::min(RENDER_HEIGHT - 1, static_cast<int>(std::ceil(max_y)) + 2);
    for (int row = begin_y; row <= end_y; ++row) {
        for (int column = begin_x; column <= end_x; ++column) {
            const float x = static_cast<float>(column) + 0.5F;
            const float y = static_cast<float>(row) + 0.5F;
            if (!PointInSurface(surface, x, y)) {
                continue;
            }

            const std::size_t offset =
                static_cast<std::size_t>(row) * static_cast<std::size_t>(RENDER_WIDTH) +
                static_cast<std::size_t>(column);
            if (surface.depth >= frame_buffer->depths[offset]) {
                continue;
            }

            Rgb color = surface.color;
            if (PointNearSurfaceEdge(surface, x, y)) {
                color = DarkenColor(color);
            }

            frame_buffer->depths[offset] = surface.depth;
            frame_buffer->pixels[offset] = PackBgr32(color);
        }
    }
}

bool RenderSceneFrame(float time_seconds, FrameBuffer *frame_buffer) {
    if (frame_buffer == nullptr) {
        return false;
    }

    FillBackground(frame_buffer, time_seconds);
    const CameraPose pose = SampleCameraTween(time_seconds);
    const CameraBasis basis = BuildCameraBasis(pose);
    std::array<Surface, SURFACE_CAPACITY> surfaces{};
    std::size_t surface_count = 0U;

    Transform cube_transform{};
    cube_transform.translation = MakeVec3(-1.8F, 0.15F, 0.0F);
    cube_transform.rotation = MakeVec3(0.0F, time_seconds * 0.18F, 0.0F);
    cube_transform.scale = MakeVec3(1.0F, 1.0F, 1.0F);
    if (!BuildCube(basis, cube_transform, &surfaces, &surface_count)) {
        return false;
    }

    Transform cylinder_transform{};
    cylinder_transform.translation = MakeVec3(0.05F, 0.6F, 0.0F);
    cylinder_transform.rotation = MakeVec3(0.0F, time_seconds * 0.12F, 0.0F);
    cylinder_transform.scale = MakeVec3(1.0F, 1.0F, 1.0F);
    if (!BuildCylinder(basis, cylinder_transform, &surfaces, &surface_count)) {
        return false;
    }

    Transform cone_transform{};
    cone_transform.translation = MakeVec3(1.85F, 0.05F, 0.85F);
    cone_transform.rotation = MakeVec3(0.0F, time_seconds * -0.16F, 0.0F);
    cone_transform.scale = MakeVec3(1.0F, 1.0F, 1.0F);
    if (!BuildCone(basis, cone_transform, &surfaces, &surface_count)) {
        return false;
    }

    for (std::size_t index = 0U; index < surface_count; ++index) {
        RasterSurface(surfaces[index], frame_buffer);
    }

    RasterTransparentPanel(frame_buffer, time_seconds);
    return true;
}

bool BlitFrame(HWND window_handle, const FrameBuffer &frame_buffer) {
    HDC device_context = GetDC(window_handle);
    if (device_context == nullptr) {
        return false;
    }

    BITMAPINFO bitmap_info{};
    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biWidth = RENDER_WIDTH;
    bitmap_info.bmiHeader.biHeight = -RENDER_HEIGHT;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;
    const int copied_lines = StretchDIBits(
        device_context,
        0,
        0,
        VIEWER_WIDTH,
        VIEWER_HEIGHT,
        0,
        0,
        RENDER_WIDTH,
        RENDER_HEIGHT,
        frame_buffer.pixels.data(),
        &bitmap_info,
        DIB_RGB_COLORS,
        SRCCOPY);
    const int release_result = ReleaseDC(window_handle, device_context);
    if (release_result == 0) {
        return false;
    }

    return copied_lines != 0;
}

bool ShouldCloseWindow(WindowsPlatformWindow &window) {
    std::array<PlatformWindowEvent, 16U> events{};
    const PlatformWindowPollResult poll_result =
        window.PollEvents(events.data(), events.size());
    if (poll_result.status != PlatformWindowStatus::Success &&
        poll_result.status != PlatformWindowStatus::OutputBufferFull) {
        return false;
    }

    for (std::size_t index = 0U; index < poll_result.event_count; ++index) {
        if (events[index].type == PlatformWindowEventType::CloseRequested) {
            return true;
        }

        if (events[index].type == PlatformWindowEventType::RawKeyDown &&
            events[index].raw_code == ESCAPE_KEY_CODE) {
            return true;
        }
    }

    return false;
}

int RunViewer(const RunOptions &options) {
    WindowsPlatformWindow window;
    PlatformWindowDesc desc{};
    desc.title = "YuEngine RVF-018 Camera Tween Blend Viewer";
    desc.client_width = VIEWER_WIDTH;
    desc.client_height = VIEWER_HEIGHT;
    desc.visible = true;

    const PlatformWindowStatus create_status = window.Create(desc);
    if (create_status != PlatformWindowStatus::Success) {
        return Blocker("NativeWindowCreate", "WindowsPlatformWindow::Create failed");
    }

    const PlatformWindowStatus show_status = window.Show();
    if (show_status != PlatformWindowStatus::Success) {
        static_cast<void>(window.Destroy());
        return Blocker("NativeWindowShow", "WindowsPlatformWindow::Show failed");
    }

    const PlatformNativeSurface surface = window.GetNativeSurface();
    if (!surface.valid || surface.window_value == 0U) {
        static_cast<void>(window.Destroy());
        return Blocker("NativeWindowSurface", "WindowsPlatformWindow returned invalid native surface");
    }

    HWND window_handle = reinterpret_cast<HWND>(surface.window_value);
    FrameBuffer frame_buffer{};
    frame_buffer.pixels.resize(static_cast<std::size_t>(RENDER_WIDTH) * static_cast<std::size_t>(RENDER_HEIGHT));
    frame_buffer.depths.resize(frame_buffer.pixels.size());

    const auto start_time = std::chrono::steady_clock::now();
    std::uint32_t frame_index = 0U;
    while (true) {
        const auto current_time = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = current_time - start_time;
        if (!RenderSceneFrame(elapsed.count(), &frame_buffer)) {
            static_cast<void>(window.Destroy());
            return Blocker("Primitive3DDepthRaster", "camera tween scene raster failed");
        }

        if (!BlitFrame(window_handle, frame_buffer)) {
            static_cast<void>(window.Destroy());
            return Blocker("LiveWindowBlit", "GDI blit to platform window failed");
        }

        ++frame_index;
        if (ShouldCloseWindow(window)) {
            break;
        }

        if (options.max_frame_count != 0U && frame_index >= options.max_frame_count) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    static_cast<void>(window.Destroy());
    std::printf("RVF018 PASS frames=%u mode=camera_tween panel=alpha_blend window=live\n", frame_index);
    return 0;
}
}

int main(int argc, char **argv) {
    RunOptions options{};
    if (!ParseRunOptions(argc, argv, &options)) {
        std::fwrite(USAGE_TEXT, sizeof(char), std::char_traits<char>::length(USAGE_TEXT), stderr);
        return 2;
    }

    return RunViewer(options);
}
