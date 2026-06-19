/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/Main.cpp
用途: 独立 D3D11 贴图网格和循环背景音乐 smoke demo。
*/

#define NOMINMAX

#include <windows.h>
#include <mmsystem.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <xaudio2.h>
#include <wrl/client.h>

#ifndef YU_ASSET_SMOKE_ENGINE_EVIDENCE
#define YU_ASSET_SMOKE_ENGINE_EVIDENCE 0
#endif

#if YU_ASSET_SMOKE_ENGINE_EVIDENCE
#include "L0EngineEvidence.h"
#include "L1VerticalSamplePrep.h"
#endif

#pragma pack(push, 8)
#include <vorbis/vorbisfile.h>
#pragma pack(pop)

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <span>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr int WINDOW_WIDTH = 960;
constexpr int WINDOW_HEIGHT = 540;
constexpr float PI_VALUE = 3.14159265358979323846f;

struct Vertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float u = 0.0f;
    float v = 0.0f;
};

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Vec2 {
    float u = 0.0f;
    float v = 0.0f;
};

struct MeshData {
    std::vector<Vertex> vertices;
};

struct TextureData {
    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<uint8_t> rgba;
};

struct WavData {
    WAVEFORMATEX format = {};
    std::vector<uint8_t> bytes;
};

struct Mat4 {
    float m[16] = {};
};

bool ReadWholeFile(const std::filesystem::path &path, std::vector<uint8_t> *bytes) {
    if (bytes == nullptr) {
        return false;
    }

    bytes->clear();

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamoff file_size = file.tellg();
    if (file_size <= 0) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    bytes->resize(static_cast<size_t>(file_size));
    file.read(reinterpret_cast<char *>(bytes->data()), file_size);
    if (!file.good()) {
        return false;
    }

    return true;
}

bool ReadTextFile(const std::filesystem::path &path, std::string *text) {
    if (text == nullptr) {
        return false;
    }

    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(path, &bytes)) {
        return false;
    }

    text->assign(reinterpret_cast<const char *>(bytes.data()), bytes.size());
    return true;
}

bool StartsWith(const std::string &value, const char *prefix) {
    if (prefix == nullptr) {
        return false;
    }

    return value.rfind(prefix, 0) == 0;
}

bool ParsePositiveInt(const std::string &text, int *value) {
    if (value == nullptr) {
        return false;
    }

    char *end = nullptr;
    long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str()) {
        return false;
    }

    if (*end != '\0') {
        return false;
    }

    if (parsed <= 0) {
        return false;
    }

    *value = static_cast<int>(parsed);
    return true;
}

bool ParseObjVertexToken(
    const std::string &token,
    const std::vector<Vec3> &positions,
    const std::vector<Vec2> &texcoords,
    Vertex *vertex) {
    if (vertex == nullptr) {
        return false;
    }

    size_t slash = token.find('/');
    if (slash == std::string::npos) {
        return false;
    }

    size_t second_slash = token.find('/', slash + 1);
    std::string position_text = token.substr(0, slash);
    std::string texcoord_text;

    if (second_slash == std::string::npos) {
        texcoord_text = token.substr(slash + 1);
    }

    if (second_slash != std::string::npos) {
        texcoord_text = token.substr(slash + 1, second_slash - slash - 1);
    }

    int position_index = 0;
    int texcoord_index = 0;
    if (!ParsePositiveInt(position_text, &position_index)) {
        return false;
    }

    if (!ParsePositiveInt(texcoord_text, &texcoord_index)) {
        return false;
    }

    size_t position_offset = static_cast<size_t>(position_index - 1);
    size_t texcoord_offset = static_cast<size_t>(texcoord_index - 1);
    if (position_offset >= positions.size()) {
        return false;
    }

    if (texcoord_offset >= texcoords.size()) {
        return false;
    }

    const Vec3 &position = positions[position_offset];
    const Vec2 &texcoord = texcoords[texcoord_offset];
    vertex->x = position.x;
    vertex->y = position.y;
    vertex->z = position.z;
    vertex->u = texcoord.u;
    vertex->v = texcoord.v;
    return true;
}

bool LoadObjMesh(const std::filesystem::path &path, MeshData *mesh) {
    if (mesh == nullptr) {
        return false;
    }

    std::string text;
    if (!ReadTextFile(path, &text)) {
        return false;
    }

    std::vector<Vec3> positions;
    std::vector<Vec2> texcoords;
    std::istringstream input(text);
    std::string line;

    mesh->vertices.clear();

    while (std::getline(input, line)) {
        if (StartsWith(line, "v ")) {
            std::istringstream line_input(line.substr(2));
            Vec3 position = {};
            line_input >> position.x >> position.y >> position.z;
            if (line_input.fail()) {
                return false;
            }

            positions.emplace_back(position);
            continue;
        }

        if (StartsWith(line, "vt ")) {
            std::istringstream line_input(line.substr(3));
            Vec2 texcoord = {};
            line_input >> texcoord.u >> texcoord.v;
            if (line_input.fail()) {
                return false;
            }

            texcoords.emplace_back(texcoord);
            continue;
        }

        if (StartsWith(line, "f ")) {
            std::istringstream line_input(line.substr(2));
            std::array<std::string, 3> tokens = {};
            line_input >> tokens[0] >> tokens[1] >> tokens[2];
            if (line_input.fail()) {
                return false;
            }

            for (const std::string &token : tokens) {
                Vertex vertex = {};
                if (!ParseObjVertexToken(token, positions, texcoords, &vertex)) {
                    return false;
                }

                mesh->vertices.emplace_back(vertex);
            }
        }
    }

    if (mesh->vertices.empty()) {
        return false;
    }

    return true;
}

bool LoadMaterialTexturePath(const std::filesystem::path &path, std::filesystem::path *texture_path) {
    if (texture_path == nullptr) {
        return false;
    }

    std::string text;
    if (!ReadTextFile(path, &text)) {
        return false;
    }

    std::istringstream input(text);
    std::string line;
    while (std::getline(input, line)) {
        if (!StartsWith(line, "texture=")) {
            continue;
        }

        std::filesystem::path parsed_path = line.substr(8);
        if (parsed_path.empty()) {
            return false;
        }

        if (parsed_path.is_relative()) {
            parsed_path = (path.parent_path() / parsed_path).lexically_normal();
        }

        *texture_path = parsed_path;
        return true;
    }

    return false;
}

uint16_t ReadU16Le(const uint8_t *data) {
    uint16_t low = static_cast<uint16_t>(data[0]);
    uint16_t high = static_cast<uint16_t>(data[1]) << 8;
    return static_cast<uint16_t>(low | high);
}

uint32_t ReadU32Le(const uint8_t *data) {
    uint32_t b0 = static_cast<uint32_t>(data[0]);
    uint32_t b1 = static_cast<uint32_t>(data[1]) << 8;
    uint32_t b2 = static_cast<uint32_t>(data[2]) << 16;
    uint32_t b3 = static_cast<uint32_t>(data[3]) << 24;
    return b0 | b1 | b2 | b3;
}

void WriteU16Le(std::ofstream *file, uint16_t value) {
    char bytes[2] = {};
    bytes[0] = static_cast<char>(value & 0xffU);
    bytes[1] = static_cast<char>((value >> 8U) & 0xffU);
    file->write(bytes, sizeof(bytes));
}

void WriteU32Le(std::ofstream *file, uint32_t value) {
    char bytes[4] = {};
    bytes[0] = static_cast<char>(value & 0xffU);
    bytes[1] = static_cast<char>((value >> 8U) & 0xffU);
    bytes[2] = static_cast<char>((value >> 16U) & 0xffU);
    bytes[3] = static_cast<char>((value >> 24U) & 0xffU);
    file->write(bytes, sizeof(bytes));
}

void WriteI32Le(std::ofstream *file, int32_t value) {
    WriteU32Le(file, static_cast<uint32_t>(value));
}

bool LoadTgaTexture(const std::filesystem::path &path, TextureData *texture) {
    if (texture == nullptr) {
        return false;
    }

    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(path, &bytes)) {
        return false;
    }

    if (bytes.size() < 18) {
        return false;
    }

    const uint8_t *header = bytes.data();
    uint8_t id_length = header[0];
    uint8_t color_map_type = header[1];
    uint8_t image_type = header[2];
    uint16_t width = ReadU16Le(header + 12);
    uint16_t height = ReadU16Le(header + 14);
    uint8_t bits_per_pixel = header[16];
    bool top_origin = (header[17] & 0x20) != 0;

    if (color_map_type != 0) {
        return false;
    }

    if (image_type != 2) {
        return false;
    }

    if (width == 0 || height == 0) {
        return false;
    }

    if (bits_per_pixel != 24 && bits_per_pixel != 32) {
        return false;
    }

    uint32_t bytes_per_pixel = static_cast<uint32_t>(bits_per_pixel / 8);
    size_t pixel_offset = static_cast<size_t>(18 + id_length);
    size_t pixel_count = static_cast<size_t>(width) * static_cast<size_t>(height);
    size_t pixel_bytes = pixel_count * static_cast<size_t>(bytes_per_pixel);
    if (pixel_offset + pixel_bytes > bytes.size()) {
        return false;
    }

    texture->width = width;
    texture->height = height;
    texture->rgba.assign(pixel_count * 4U, 255U);

    const uint8_t *source_pixels = bytes.data() + pixel_offset;
    for (uint32_t y = 0; y < height; ++y) {
        uint32_t source_y = y;
        if (!top_origin) {
            source_y = static_cast<uint32_t>(height - 1U - y);
        }

        for (uint32_t x = 0; x < width; ++x) {
            size_t source_index = (static_cast<size_t>(source_y) * width + x) * bytes_per_pixel;
            size_t target_index = (static_cast<size_t>(y) * width + x) * 4U;
            const uint8_t *source = source_pixels + source_index;
            texture->rgba[target_index + 0U] = source[2];
            texture->rgba[target_index + 1U] = source[1];
            texture->rgba[target_index + 2U] = source[0];
            texture->rgba[target_index + 3U] = 255U;
            if (bytes_per_pixel == 4U) {
                texture->rgba[target_index + 3U] = source[3];
            }
        }
    }

    return true;
}

bool LoadWavPcm(const std::filesystem::path &path, WavData *wav) {
    if (wav == nullptr) {
        return false;
    }

    std::vector<uint8_t> bytes;
    if (!ReadWholeFile(path, &bytes)) {
        return false;
    }

    if (bytes.size() < 44U) {
        return false;
    }

    if (std::memcmp(bytes.data(), "RIFF", 4U) != 0) {
        return false;
    }

    if (std::memcmp(bytes.data() + 8U, "WAVE", 4U) != 0) {
        return false;
    }

    bool found_format = false;
    bool found_data = false;
    WAVEFORMATEX format = {};
    std::vector<uint8_t> audio_bytes;

    size_t offset = 12U;
    while (offset + 8U <= bytes.size()) {
        const uint8_t *chunk = bytes.data() + offset;
        uint32_t chunk_size = ReadU32Le(chunk + 4U);
        size_t chunk_data_offset = offset + 8U;
        size_t next_offset = chunk_data_offset + chunk_size + (chunk_size & 1U);
        if (chunk_data_offset + chunk_size > bytes.size()) {
            return false;
        }

        if (std::memcmp(chunk, "fmt ", 4U) == 0) {
            if (chunk_size < 16U) {
                return false;
            }

            uint16_t format_tag = ReadU16Le(bytes.data() + chunk_data_offset + 0U);
            uint16_t channels = ReadU16Le(bytes.data() + chunk_data_offset + 2U);
            uint32_t samples_per_second = ReadU32Le(bytes.data() + chunk_data_offset + 4U);
            uint32_t average_bytes_per_second = ReadU32Le(bytes.data() + chunk_data_offset + 8U);
            uint16_t block_align = ReadU16Le(bytes.data() + chunk_data_offset + 12U);
            uint16_t bits_per_sample = ReadU16Le(bytes.data() + chunk_data_offset + 14U);
            if (format_tag != WAVE_FORMAT_PCM) {
                return false;
            }

            if (channels == 0 || samples_per_second == 0 || block_align == 0) {
                return false;
            }

            format.wFormatTag = WAVE_FORMAT_PCM;
            format.nChannels = channels;
            format.nSamplesPerSec = samples_per_second;
            format.nAvgBytesPerSec = average_bytes_per_second;
            format.nBlockAlign = block_align;
            format.wBitsPerSample = bits_per_sample;
            format.cbSize = 0;
            found_format = true;
        }

        if (std::memcmp(chunk, "data", 4U) == 0) {
            audio_bytes.assign(bytes.begin() + static_cast<std::ptrdiff_t>(chunk_data_offset),
                bytes.begin() + static_cast<std::ptrdiff_t>(chunk_data_offset + chunk_size));
            found_data = true;
        }

        offset = next_offset;
    }

    if (!found_format || !found_data) {
        return false;
    }

    if (audio_bytes.empty()) {
        return false;
    }

    wav->format = format;
    wav->bytes = std::move(audio_bytes);
    return true;
}

size_t ReadVorbisFile(void *ptr, size_t size, size_t count, void *datasource) {
    if (ptr == nullptr || datasource == nullptr) {
        return 0U;
    }

    FILE *file = static_cast<FILE *>(datasource);
    return std::fread(ptr, size, count, file);
}

int SeekVorbisFile(void *datasource, ogg_int64_t offset, int whence) {
    if (datasource == nullptr) {
        return -1;
    }

    int origin = SEEK_SET;
    switch (whence) {
    case SEEK_SET:
        origin = SEEK_SET;
        break;
    case SEEK_CUR:
        origin = SEEK_CUR;
        break;
    case SEEK_END:
        origin = SEEK_END;
        break;
    default:
        return -1;
    }

    FILE *file = static_cast<FILE *>(datasource);
    return _fseeki64(file, static_cast<__int64>(offset), origin);
}

int CloseVorbisFile(void *datasource) {
    if (datasource == nullptr) {
        return EOF;
    }

    FILE *file = static_cast<FILE *>(datasource);
    return std::fclose(file);
}

long TellVorbisFile(void *datasource) {
    if (datasource == nullptr) {
        return -1L;
    }

    FILE *file = static_cast<FILE *>(datasource);
    __int64 position = _ftelli64(file);
    if (position < 0) {
        return -1L;
    }

    constexpr __int64 MAX_LONG_VALUE = 2147483647LL;
    if (position > MAX_LONG_VALUE) {
        return static_cast<long>(MAX_LONG_VALUE);
    }

    return static_cast<long>(position);
}

bool LoadOggVorbisPcm(const std::filesystem::path &path, WavData *wav) {
    if (wav == nullptr) {
        return false;
    }

    FILE *file = nullptr;
    errno_t open_error = _wfopen_s(&file, path.c_str(), L"rb");
    if (open_error != 0 || file == nullptr) {
        return false;
    }

    ov_callbacks callbacks = {};
    callbacks.read_func = ReadVorbisFile;
    callbacks.seek_func = SeekVorbisFile;
    callbacks.close_func = CloseVorbisFile;
    callbacks.tell_func = TellVorbisFile;

    OggVorbis_File vorbis_file = {};
    int open_result = ov_open_callbacks(file, &vorbis_file, nullptr, 0, callbacks);
    if (open_result < 0) {
        std::fclose(file);
        return false;
    }

    bool result = false;
    vorbis_info *info = ov_info(&vorbis_file, -1);
    if (info != nullptr) {
        if (info->channels > 0 && info->channels <= 8 && info->rate > 0) {
            std::vector<uint8_t> pcm_bytes;
            std::array<char, 4096> buffer = {};
            int bitstream = 0;
            bool decode_failed = false;

            for (;;) {
                long bytes_read = ov_read(
                    &vorbis_file,
                    buffer.data(),
                    static_cast<int>(buffer.size()),
                    0,
                    2,
                    1,
                    &bitstream);
                if (bytes_read == 0) {
                    break;
                }

                if (bytes_read < 0) {
                    decode_failed = true;
                    break;
                }

                const uint8_t *begin = reinterpret_cast<const uint8_t *>(buffer.data());
                const uint8_t *end = begin + bytes_read;
                pcm_bytes.insert(pcm_bytes.end(), begin, end);
            }

            if (!decode_failed && !pcm_bytes.empty()) {
                WAVEFORMATEX format = {};
                format.wFormatTag = WAVE_FORMAT_PCM;
                format.nChannels = static_cast<WORD>(info->channels);
                format.nSamplesPerSec = static_cast<DWORD>(info->rate);
                format.wBitsPerSample = 16U;
                format.nBlockAlign = static_cast<WORD>(format.nChannels * sizeof(int16_t));
                format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
                format.cbSize = 0U;

                wav->format = format;
                wav->bytes = std::move(pcm_bytes);
                result = true;
            }
        }
    }

    ov_clear(&vorbis_file);
    return result;
}

Mat4 MakeIdentity() {
    Mat4 result = {};
    result.m[0] = 1.0f;
    result.m[5] = 1.0f;
    result.m[10] = 1.0f;
    result.m[15] = 1.0f;
    return result;
}

Mat4 MultiplyMatrix(const Mat4 &left, const Mat4 &right) {
    Mat4 result = {};
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            float value = 0.0f;
            for (int index = 0; index < 4; ++index) {
                value += left.m[row * 4 + index] * right.m[index * 4 + column];
            }

            result.m[row * 4 + column] = value;
        }
    }

    return result;
}

Mat4 MakeRotationY(float angle) {
    Mat4 result = MakeIdentity();
    float angle_cos = std::cos(angle);
    float angle_sin = std::sin(angle);
    result.m[0] = angle_cos;
    result.m[2] = angle_sin;
    result.m[8] = -angle_sin;
    result.m[10] = angle_cos;
    return result;
}

Mat4 MakeTranslation(float x, float y, float z) {
    Mat4 result = MakeIdentity();
    result.m[12] = x;
    result.m[13] = y;
    result.m[14] = z;
    return result;
}

Mat4 MakePerspective(float fov_y, float aspect, float near_z, float far_z) {
    Mat4 result = {};
    float y_scale = 1.0f / std::tan(fov_y * 0.5f);
    float x_scale = y_scale / aspect;
    float depth_scale = far_z / (far_z - near_z);
    result.m[0] = x_scale;
    result.m[5] = y_scale;
    result.m[10] = depth_scale;
    result.m[11] = 1.0f;
    result.m[14] = -near_z * depth_scale;
    return result;
}

bool CompileShader(const char *source, const char *entry, const char *target, ID3DBlob **blob) {
    if (source == nullptr || entry == nullptr || target == nullptr || blob == nullptr) {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> error_blob;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
    HRESULT hr = D3DCompile(
        source,
        std::strlen(source),
        nullptr,
        nullptr,
        nullptr,
        entry,
        target,
        flags,
        0U,
        blob,
        error_blob.GetAddressOf());
    if (FAILED(hr)) {
        if (error_blob != nullptr) {
            const char *message = static_cast<const char *>(error_blob->GetBufferPointer());
            std::fprintf(stderr, "Shader compile failed: %s\n", message);
        }

        return false;
    }

    return true;
}

class Renderer {
public:
    bool Initialize(HWND window, const MeshData &mesh, const TextureData &texture) {
        if (window == nullptr) {
            return false;
        }

        if (mesh.vertices.empty()) {
            return false;
        }

        if (texture.rgba.empty()) {
            return false;
        }

        vertex_count_ = static_cast<UINT>(mesh.vertices.size());
        if (!CreateDevice(window)) {
            return false;
        }

        if (!CreateShaders()) {
            return false;
        }

        if (!CreateMeshBuffer(mesh)) {
            return false;
        }

        if (!CreateTexture(texture)) {
            return false;
        }

        if (!CreateConstantBuffer()) {
            return false;
        }

        if (!CreateRasterizerState()) {
            return false;
        }

        return true;
    }

    bool Render(float seconds, const std::filesystem::path *capture_path) {
        if (context_ == nullptr || swap_chain_ == nullptr) {
            return false;
        }

        float clear_color[4] = { 0.04f, 0.05f, 0.08f, 1.0f };
        context_->ClearRenderTargetView(render_target_.Get(), clear_color);
        context_->OMSetRenderTargets(1U, render_target_.GetAddressOf(), nullptr);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(WINDOW_WIDTH);
        viewport.Height = static_cast<float>(WINDOW_HEIGHT);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context_->RSSetViewports(1U, &viewport);
        context_->RSSetState(rasterizer_state_.Get());

        UINT stride = sizeof(Vertex);
        UINT offset = 0U;
        ID3D11Buffer *vertex_buffers[] = { vertex_buffer_.Get() };
        context_->IASetInputLayout(input_layout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->IASetVertexBuffers(0U, 1U, vertex_buffers, &stride, &offset);
        context_->VSSetShader(vertex_shader_.Get(), nullptr, 0U);
        context_->PSSetShader(pixel_shader_.Get(), nullptr, 0U);
        context_->PSSetShaderResources(0U, 1U, texture_view_.GetAddressOf());
        context_->PSSetSamplers(0U, 1U, sampler_.GetAddressOf());

        Mat4 rotation = MakeRotationY(seconds * 0.8f);
        Mat4 translation = MakeTranslation(0.0f, 0.0f, 2.7f);
        Mat4 world = MultiplyMatrix(rotation, translation);
        Mat4 projection = MakePerspective(PI_VALUE / 3.0f, static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f);
        Mat4 mvp = MultiplyMatrix(world, projection);

        context_->UpdateSubresource(constant_buffer_.Get(), 0U, nullptr, &mvp, 0U, 0U);
        ID3D11Buffer *constant_buffers[] = { constant_buffer_.Get() };
        context_->VSSetConstantBuffers(0U, 1U, constant_buffers);

        context_->Draw(vertex_count_, 0U);
        if (capture_path != nullptr) {
            if (!CaptureBackBufferBmp(*capture_path)) {
                return false;
            }
        }

        swap_chain_->Present(1U, 0U);
        return true;
    }

    bool CaptureBackBufferBmp(const std::filesystem::path &path) {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
        HRESULT hr = swap_chain_->GetBuffer(0U, IID_PPV_ARGS(back_buffer.GetAddressOf()));
        if (FAILED(hr)) {
            return false;
        }

        D3D11_TEXTURE2D_DESC texture_desc = {};
        back_buffer->GetDesc(&texture_desc);
        if (texture_desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM) {
            return false;
        }

        D3D11_TEXTURE2D_DESC staging_desc = texture_desc;
        staging_desc.BindFlags = 0U;
        staging_desc.MiscFlags = 0U;
        staging_desc.Usage = D3D11_USAGE_STAGING;
        staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> staging_texture;
        hr = device_->CreateTexture2D(&staging_desc, nullptr, staging_texture.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        context_->CopyResource(staging_texture.Get(), back_buffer.Get());

        D3D11_MAPPED_SUBRESOURCE mapped = {};
        hr = context_->Map(staging_texture.Get(), 0U, D3D11_MAP_READ, 0U, &mapped);
        if (FAILED(hr)) {
            return false;
        }

        bool result = WriteMappedTextureBmp(path, mapped, texture_desc.Width, texture_desc.Height);
        context_->Unmap(staging_texture.Get(), 0U);
        return result;
    }

    bool WriteMappedTextureBmp(
        const std::filesystem::path &path,
        const D3D11_MAPPED_SUBRESOURCE &mapped,
        uint32_t width,
        uint32_t height) {
        if (mapped.pData == nullptr) {
            return false;
        }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        uint32_t row_size = width * 4U;
        uint32_t pixel_data_size = row_size * height;
        uint32_t file_size = 14U + 40U + pixel_data_size;

        file.put('B');
        file.put('M');
        WriteU32Le(&file, file_size);
        WriteU16Le(&file, 0U);
        WriteU16Le(&file, 0U);
        WriteU32Le(&file, 54U);
        WriteU32Le(&file, 40U);
        WriteI32Le(&file, static_cast<int32_t>(width));
        WriteI32Le(&file, static_cast<int32_t>(height));
        WriteU16Le(&file, 1U);
        WriteU16Le(&file, 32U);
        WriteU32Le(&file, 0U);
        WriteU32Le(&file, pixel_data_size);
        WriteI32Le(&file, 2835);
        WriteI32Le(&file, 2835);
        WriteU32Le(&file, 0U);
        WriteU32Le(&file, 0U);

        std::vector<uint8_t> row(row_size);
        const uint8_t *source_base = static_cast<const uint8_t *>(mapped.pData);
        for (uint32_t row_index = 0U; row_index < height; ++row_index) {
            uint32_t source_y = height - 1U - row_index;
            const uint8_t *source_row = source_base + static_cast<size_t>(source_y) * mapped.RowPitch;
            for (uint32_t x = 0U; x < width; ++x) {
                size_t source_index = static_cast<size_t>(x) * 4U;
                size_t target_index = source_index;
                row[target_index + 0U] = source_row[source_index + 2U];
                row[target_index + 1U] = source_row[source_index + 1U];
                row[target_index + 2U] = source_row[source_index + 0U];
                row[target_index + 3U] = 255U;
            }

            file.write(reinterpret_cast<const char *>(row.data()), row.size());
            if (!file.good()) {
                return false;
            }
        }

        return true;
    }

private:
    bool CreateDevice(HWND window) {
        DXGI_SWAP_CHAIN_DESC swap_desc = {};
        swap_desc.BufferCount = 1U;
        swap_desc.BufferDesc.Width = WINDOW_WIDTH;
        swap_desc.BufferDesc.Height = WINDOW_HEIGHT;
        swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swap_desc.BufferDesc.RefreshRate.Numerator = 60U;
        swap_desc.BufferDesc.RefreshRate.Denominator = 1U;
        swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_desc.OutputWindow = window;
        swap_desc.SampleDesc.Count = 1U;
        swap_desc.SampleDesc.Quality = 0U;
        swap_desc.Windowed = TRUE;
        swap_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL feature_levels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };

        D3D_FEATURE_LEVEL created_level = D3D_FEATURE_LEVEL_10_0;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            feature_levels,
            static_cast<UINT>(std::size(feature_levels)),
            D3D11_SDK_VERSION,
            &swap_desc,
            swap_chain_.GetAddressOf(),
            device_.GetAddressOf(),
            &created_level,
            context_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
        hr = swap_chain_->GetBuffer(0U, IID_PPV_ARGS(back_buffer.GetAddressOf()));
        if (FAILED(hr)) {
            return false;
        }

        hr = device_->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool CreateShaders() {
        static const char *VERTEX_SHADER =
            "cbuffer TransformBuffer : register(b0) { row_major float4x4 mvp; };"
            "struct VSIn { float3 position : POSITION; float2 uv : TEXCOORD0; };"
            "struct PSIn { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };"
            "PSIn VSMain(VSIn input) {"
            "    PSIn output;"
            "    output.position = mul(float4(input.position, 1.0f), mvp);"
            "    output.uv = input.uv;"
            "    return output;"
            "}";

        static const char *PIXEL_SHADER =
            "Texture2D diffuse_texture : register(t0);"
            "SamplerState diffuse_sampler : register(s0);"
            "struct PSIn { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };"
            "float4 PSMain(PSIn input) : SV_Target {"
            "    return diffuse_texture.Sample(diffuse_sampler, input.uv);"
            "}";

        Microsoft::WRL::ComPtr<ID3DBlob> vertex_blob;
        Microsoft::WRL::ComPtr<ID3DBlob> pixel_blob;
        if (!CompileShader(VERTEX_SHADER, "VSMain", "vs_4_0", vertex_blob.GetAddressOf())) {
            return false;
        }

        if (!CompileShader(PIXEL_SHADER, "PSMain", "ps_4_0", pixel_blob.GetAddressOf())) {
            return false;
        }

        HRESULT hr = device_->CreateVertexShader(
            vertex_blob->GetBufferPointer(),
            vertex_blob->GetBufferSize(),
            nullptr,
            vertex_shader_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        hr = device_->CreatePixelShader(
            pixel_blob->GetBufferPointer(),
            pixel_blob->GetBufferSize(),
            nullptr,
            pixel_shader_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC input_elements[] = {
            { "POSITION", 0U, DXGI_FORMAT_R32G32B32_FLOAT, 0U, static_cast<UINT>(offsetof(Vertex, x)), D3D11_INPUT_PER_VERTEX_DATA, 0U },
            { "TEXCOORD", 0U, DXGI_FORMAT_R32G32_FLOAT, 0U, static_cast<UINT>(offsetof(Vertex, u)), D3D11_INPUT_PER_VERTEX_DATA, 0U }
        };

        hr = device_->CreateInputLayout(
            input_elements,
            static_cast<UINT>(std::size(input_elements)),
            vertex_blob->GetBufferPointer(),
            vertex_blob->GetBufferSize(),
            input_layout_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool CreateMeshBuffer(const MeshData &mesh) {
        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA initial_data = {};
        initial_data.pSysMem = mesh.vertices.data();

        HRESULT hr = device_->CreateBuffer(&buffer_desc, &initial_data, vertex_buffer_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool CreateTexture(const TextureData &texture) {
        D3D11_TEXTURE2D_DESC texture_desc = {};
        texture_desc.Width = texture.width;
        texture_desc.Height = texture.height;
        texture_desc.MipLevels = 1U;
        texture_desc.ArraySize = 1U;
        texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_desc.SampleDesc.Count = 1U;
        texture_desc.Usage = D3D11_USAGE_DEFAULT;
        texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA initial_data = {};
        initial_data.pSysMem = texture.rgba.data();
        initial_data.SysMemPitch = texture.width * 4U;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> gpu_texture;
        HRESULT hr = device_->CreateTexture2D(&texture_desc, &initial_data, gpu_texture.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        hr = device_->CreateShaderResourceView(gpu_texture.Get(), nullptr, texture_view_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        D3D11_SAMPLER_DESC sampler_desc = {};
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

        hr = device_->CreateSamplerState(&sampler_desc, sampler_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool CreateConstantBuffer() {
        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.ByteWidth = sizeof(Mat4);
        buffer_desc.Usage = D3D11_USAGE_DEFAULT;
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        HRESULT hr = device_->CreateBuffer(&buffer_desc, nullptr, constant_buffer_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    bool CreateRasterizerState() {
        D3D11_RASTERIZER_DESC rasterizer_desc = {};
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.DepthClipEnable = TRUE;

        HRESULT hr = device_->CreateRasterizerState(&rasterizer_desc, rasterizer_state_.GetAddressOf());
        if (FAILED(hr)) {
            return false;
        }

        return true;
    }

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_;
    UINT vertex_count_ = 0U;
};

class AudioLoop {
public:
    ~AudioLoop() {
        Stop();
    }

    bool Start(const std::filesystem::path &path) {
        Stop();

        WavData wav = {};
        if (LoadWavPcm(path, &wav)) {
            if (StartXAudio(&wav, L"XAudio2/Wav")) {
                return true;
            }
        }

        WavData ogg = {};
        if (LoadOggVorbisPcm(path, &ogg)) {
            if (StartXAudio(&ogg, L"XAudio2/Vorbis")) {
                return true;
            }
        }

        BOOL played = PlaySoundW(path.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_LOOP);
        if (played != FALSE) {
            winmm_active_ = true;
            mode_ = L"WinMM";
            return true;
        }

        mode_ = L"Unavailable";
        return false;
    }

    void Stop() {
        if (source_voice_ != nullptr) {
            source_voice_->Stop(0U);
            source_voice_->DestroyVoice();
            source_voice_ = nullptr;
        }

        if (master_voice_ != nullptr) {
            master_voice_->DestroyVoice();
            master_voice_ = nullptr;
        }

        audio_.Reset();
        audio_bytes_.clear();

        if (winmm_active_) {
            PlaySoundW(nullptr, nullptr, 0U);
            winmm_active_ = false;
        }
    }

    const std::wstring &GetMode() const {
        return mode_;
    }

private:
    bool StartXAudio(WavData *wav, const wchar_t *mode) {
        if (wav == nullptr || mode == nullptr) {
            return false;
        }

        HRESULT hr = XAudio2Create(audio_.GetAddressOf(), 0U, XAUDIO2_DEFAULT_PROCESSOR);
        if (SUCCEEDED(hr)) {
            hr = audio_->CreateMasteringVoice(&master_voice_);
        }

        if (SUCCEEDED(hr)) {
            hr = audio_->CreateSourceVoice(&source_voice_, &wav->format);
        }

        if (SUCCEEDED(hr)) {
            audio_bytes_ = std::move(wav->bytes);
            XAUDIO2_BUFFER buffer = {};
            buffer.AudioBytes = static_cast<UINT32>(audio_bytes_.size());
            buffer.pAudioData = audio_bytes_.data();
            buffer.Flags = XAUDIO2_END_OF_STREAM;
            buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
            hr = source_voice_->SubmitSourceBuffer(&buffer);
        }

        if (SUCCEEDED(hr)) {
            hr = source_voice_->Start(0U);
        }

        if (SUCCEEDED(hr)) {
            mode_ = mode;
            return true;
        }

        Stop();
        return false;
    }

    Microsoft::WRL::ComPtr<IXAudio2> audio_;
    IXAudio2MasteringVoice *master_voice_ = nullptr;
    IXAudio2SourceVoice *source_voice_ = nullptr;
    std::vector<uint8_t> audio_bytes_;
    bool winmm_active_ = false;
    std::wstring mode_ = L"Unavailable";
};

LRESULT CALLBACK DemoWindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }

    return DefWindowProcW(window, message, wparam, lparam);
}

HWND CreateDemoWindow(HINSTANCE instance) {
    WNDCLASSW window_class = {};
    window_class.lpfnWndProc = DemoWindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = L"YuAssetSmokeDemoWindow";
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));

    ATOM atom = RegisterClassW(&window_class);
    if (atom == 0) {
        DWORD error = GetLastError();
        if (error != ERROR_CLASS_ALREADY_EXISTS) {
            return nullptr;
        }
    }

    RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND window = CreateWindowExW(
        0U,
        window_class.lpszClassName,
        L"YuEngine isolated asset smoke",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);
    if (window == nullptr) {
        return nullptr;
    }

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    return window;
}

std::filesystem::path GetDefaultAssetRoot() {
    wchar_t module_path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, module_path, MAX_PATH);
    std::filesystem::path root = std::filesystem::path(module_path).parent_path();
    root = root.parent_path();
    root = root.parent_path();
    return root / L"Assets";
}

int RunDemo(int argc, wchar_t **argv) {
    std::filesystem::path asset_root = GetDefaultAssetRoot();
    std::filesystem::path capture_path;
    std::filesystem::path music_override_path;
    int seconds_to_run = 8;
    bool should_capture = false;
    bool has_music_override = false;

    for (int index = 1; index < argc; ++index) {
        std::wstring argument = argv[index];
        if (argument == L"--assets") {
            if (index + 1 >= argc) {
                return 2;
            }

            ++index;
            asset_root = argv[index];
            continue;
        }

        if (argument == L"--seconds") {
            if (index + 1 >= argc) {
                return 2;
            }

            ++index;
            seconds_to_run = std::max(1, _wtoi(argv[index]));
            continue;
        }

        if (argument == L"--capture") {
            if (index + 1 >= argc) {
                return 2;
            }

            ++index;
            capture_path = argv[index];
            should_capture = true;
            continue;
        }

        if (argument == L"--music") {
            if (index + 1 >= argc) {
                return 2;
            }

            ++index;
            music_override_path = argv[index];
            has_music_override = true;
            continue;
        }
    }

    std::filesystem::path mesh_path = asset_root / L"Meshes" / L"TexturedMesh.obj";
    std::filesystem::path material_path = asset_root / L"Materials" / L"DemoMaterial.txt";
    std::filesystem::path music_path = asset_root / L"Music" / L"Loop.wav";
    if (has_music_override) {
        music_path = music_override_path;
    }

    MeshData mesh = {};
    if (!LoadObjMesh(mesh_path, &mesh)) {
        std::fwprintf(stderr, L"Failed to load mesh: %ls\n", mesh_path.c_str());
        return 3;
    }

    std::filesystem::path texture_path;
    if (!LoadMaterialTexturePath(material_path, &texture_path)) {
        std::fwprintf(stderr, L"Failed to load material: %ls\n", material_path.c_str());
        return 4;
    }

    TextureData texture = {};
    if (!LoadTgaTexture(texture_path, &texture)) {
        std::fwprintf(stderr, L"Failed to load texture: %ls\n", texture_path.c_str());
        return 5;
    }

#if YU_ASSET_SMOKE_ENGINE_EVIDENCE
    asset_smoke_demo::L0EngineEvidenceResult engine_evidence = {};
    asset_smoke_demo::L1VerticalSamplePrepResult l1_prep = {};
    asset_smoke_demo::L0EngineEvidenceInput engine_input = {};
    engine_input.asset_root = asset_root;
    engine_input.texture_path = texture_path;
    engine_input.texture_rgba = std::span<const uint8_t>(texture.rgba.data(), texture.rgba.size());
    engine_input.texture_width = texture.width;
    engine_input.texture_height = texture.height;
    if (!asset_smoke_demo::RunL0EngineEvidence(engine_input, &engine_evidence)) {
        std::fprintf(
            stderr,
            "YuAssetSmokeDemo L0 engine evidence failed stage=%s file_read=%u resource_decode=%u texture_upload=%u rendercore=%u hardware=%u resize=%u shutdown=%u.\n",
            engine_evidence.failure_stage,
            engine_evidence.file_read ? 1U : 0U,
            engine_evidence.resource_decode ? 1U : 0U,
            engine_evidence.texture_upload ? 1U : 0U,
            engine_evidence.rendercore_view_draw_material ? 1U : 0U,
            engine_evidence.hardware_frame ? 1U : 0U,
            engine_evidence.resize ? 1U : 0U,
            engine_evidence.shutdown ? 1U : 0U);
        return 9;
    }

    if (!asset_smoke_demo::RunL1VerticalSamplePrep(&l1_prep)) {
        std::fprintf(
            stderr,
            "YuAssetSmokeDemo L1 sample prep failed stage=%s runtime=%u manifest=%u world=%u assets=%u input=%u render=%u audio=%u.\n",
            l1_prep.failure_stage,
            l1_prep.runtime_boot ? 1U : 0U,
            l1_prep.synthetic_manifest ? 1U : 0U,
            l1_prep.world_object ? 1U : 0U,
            l1_prep.asset_bindings ? 1U : 0U,
            l1_prep.input_command ? 1U : 0U,
            l1_prep.render_scene_submit ? 1U : 0U,
            l1_prep.audio_scene_submit ? 1U : 0U);
        return 10;
    }
#endif

    HINSTANCE instance = GetModuleHandleW(nullptr);
    HWND window = CreateDemoWindow(instance);
    if (window == nullptr) {
        std::fprintf(stderr, "Failed to create window.\n");
        return 6;
    }

    Renderer renderer;
    if (!renderer.Initialize(window, mesh, texture)) {
        std::fprintf(stderr, "Failed to initialize D3D11 renderer.\n");
        return 7;
    }

    AudioLoop audio;
    audio.Start(music_path);

    std::wstring title = L"YuEngine isolated asset smoke - D3D11 textured mesh - BGM: ";
    title += audio.GetMode();
    SetWindowTextW(window, title.c_str());

    MSG message = {};
    bool running = true;
    bool captured = false;
    auto start_time = std::chrono::steady_clock::now();
    while (running) {
        while (PeekMessageW(&message, nullptr, 0U, 0U, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                running = false;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        if (!running) {
            break;
        }

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - start_time;
        if (elapsed.count() >= static_cast<float>(seconds_to_run)) {
            running = false;
            continue;
        }

        const std::filesystem::path *current_capture_path = nullptr;
        if (should_capture && !captured && elapsed.count() >= 1.0f) {
            current_capture_path = &capture_path;
        }

        if (!renderer.Render(elapsed.count(), current_capture_path)) {
            return 8;
        }

        if (current_capture_path != nullptr) {
            captured = true;
        }
    }

    audio.Stop();
    if (IsWindow(window)) {
        DestroyWindow(window);
    }

    std::wprintf(
        L"YuAssetSmokeDemo PASS mesh_vertices=%u texture=%ux%u bgm=%ls seconds=%d\n",
        static_cast<unsigned int>(mesh.vertices.size()),
        static_cast<unsigned int>(texture.width),
        static_cast<unsigned int>(texture.height),
        audio.GetMode().c_str(),
        seconds_to_run);
#if YU_ASSET_SMOKE_ENGINE_EVIDENCE
    std::printf(
        "YuAssetSmokeDemo L0_ENGINE PASS file_bytes=%u decoded_texture=%ux%u upload_generation=%u render_frames=%u input_events=%u gamepad=%s audio=%s resize=pass shutdown=pass\n",
        engine_evidence.file_read_byte_count,
        engine_evidence.decoded_texture_width,
        engine_evidence.decoded_texture_height,
        engine_evidence.uploaded_texture_generation,
        engine_evidence.render_frame_count,
        engine_evidence.input_event_count,
        engine_evidence.gamepad_state,
        engine_evidence.audio_state);
    std::printf(
        "YuAssetSmokeDemo L1_PREP PASS frames=%u world_objects=%u assets=%u input_commands=%u render_packets=%u audio_requests=%u\n",
        l1_prep.completed_frame_count,
        l1_prep.world_object_count,
        l1_prep.asset_count,
        l1_prep.input_command_count,
        l1_prep.render_packet_count,
        l1_prep.audio_queue_request_count);
#endif
    return 0;
}

} // 匿名命名空间

int wmain(int argc, wchar_t **argv) {
    HRESULT co_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    bool should_uninitialize = SUCCEEDED(co_result);

    int result = RunDemo(argc, argv);

    if (should_uninitialize) {
        CoUninitialize();
    }

    return result;
}
