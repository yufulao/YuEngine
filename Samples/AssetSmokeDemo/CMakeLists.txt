cmake_minimum_required(VERSION 3.20)

project(YuAssetSmokeDemo LANGUAGES CXX)

set(YU_ASSET_SMOKE_UE_ENGINE_ROOT "" CACHE PATH "Unreal Engine root used only for Ogg and Vorbis sample dependencies.")
if("${YU_ASSET_SMOKE_UE_ENGINE_ROOT}" STREQUAL "" AND DEFINED ENV{UE_ENGINE_ROOT})
    file(TO_CMAKE_PATH "$ENV{UE_ENGINE_ROOT}" YU_ASSET_SMOKE_UE_ENGINE_ROOT)
endif()
if("${YU_ASSET_SMOKE_UE_ENGINE_ROOT}" STREQUAL "")
    set(YU_ASSET_SMOKE_UE_ENGINE_ROOT "D:/app/Epic Games/UE_5.5/Engine")
endif()

set(UE_ENGINE_ROOT "${YU_ASSET_SMOKE_UE_ENGINE_ROOT}")
set(UE_OGG_ROOT "${UE_ENGINE_ROOT}/Source/ThirdParty/Ogg/libogg-1.2.2")
set(UE_VORBIS_ROOT "${UE_ENGINE_ROOT}/Source/ThirdParty/Vorbis/libvorbis-1.3.2")
set(UE_OGG_BIN "${UE_ENGINE_ROOT}/Binaries/ThirdParty/Ogg/Win64/VS2015")
set(UE_VORBIS_BIN "${UE_ENGINE_ROOT}/Binaries/ThirdParty/Vorbis/Win64/VS2015")

set(YU_ASSET_SMOKE_REQUIRED_FILES
    "${UE_OGG_ROOT}/include/ogg/ogg.h"
    "${UE_VORBIS_ROOT}/include/vorbis/vorbisfile.h"
    "${UE_OGG_ROOT}/lib/Win64/VS2015/libogg_64.lib"
    "${UE_VORBIS_ROOT}/lib/win64/VS2015/libvorbis_64.lib"
    "${UE_VORBIS_ROOT}/lib/win64/VS2015/libvorbisfile_64.lib"
    "${UE_OGG_BIN}/libogg_64.dll"
    "${UE_VORBIS_BIN}/libvorbis_64.dll"
    "${UE_VORBIS_BIN}/libvorbisfile_64.dll"
)

foreach(required_file IN LISTS YU_ASSET_SMOKE_REQUIRED_FILES)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "AssetSmokeDemo requires Unreal Engine Ogg/Vorbis dependencies. Set -DYU_ASSET_SMOKE_UE_ENGINE_ROOT=... or UE_ENGINE_ROOT. Missing: ${required_file}")
    endif()
endforeach()

add_executable(YuAssetSmokeDemo
    Source/Main.cpp
)

target_compile_features(YuAssetSmokeDemo PRIVATE cxx_std_17)

target_include_directories(YuAssetSmokeDemo PRIVATE
    "${UE_OGG_ROOT}/include"
    "${UE_VORBIS_ROOT}/include"
)

if(MSVC)
    target_compile_options(YuAssetSmokeDemo PRIVATE
        /source-charset:utf-8
        /W4
    )
endif()

target_link_libraries(YuAssetSmokeDemo PRIVATE
    d3d11
    dxgi
    d3dcompiler
    "${UE_OGG_ROOT}/lib/Win64/VS2015/libogg_64.lib"
    "${UE_VORBIS_ROOT}/lib/win64/VS2015/libvorbis_64.lib"
    "${UE_VORBIS_ROOT}/lib/win64/VS2015/libvorbisfile_64.lib"
    ole32
    xaudio2
    winmm
)

add_custom_command(TARGET YuAssetSmokeDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${UE_OGG_BIN}/libogg_64.dll"
        "$<TARGET_FILE_DIR:YuAssetSmokeDemo>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${UE_VORBIS_BIN}/libvorbis_64.dll"
        "$<TARGET_FILE_DIR:YuAssetSmokeDemo>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${UE_VORBIS_BIN}/libvorbisfile_64.dll"
        "$<TARGET_FILE_DIR:YuAssetSmokeDemo>"
)
