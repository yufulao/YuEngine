cmake_minimum_required(VERSION 3.20)

project(YuAssetSmokeDemo LANGUAGES CXX)

option(YU_ASSET_SMOKE_ENGINE_EVIDENCE "Build the sample against YuEngine L0 evidence modules." ON)

get_filename_component(YU_ASSET_SMOKE_DEFAULT_ENGINE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(YU_ASSET_SMOKE_ENGINE_ROOT "${YU_ASSET_SMOKE_DEFAULT_ENGINE_ROOT}" CACHE PATH "YuEngine source root used by the sample engine evidence path.")

set(YU_ASSET_SMOKE_UE_ENGINE_ROOT "" CACHE PATH "Unreal Engine root used only for Ogg and Vorbis sample dependencies.")
if("${YU_ASSET_SMOKE_UE_ENGINE_ROOT}" STREQUAL "" AND DEFINED ENV{UE_ENGINE_ROOT})
    file(TO_CMAKE_PATH "$ENV{UE_ENGINE_ROOT}" YU_ASSET_SMOKE_UE_ENGINE_ROOT)
endif()
if("${YU_ASSET_SMOKE_UE_ENGINE_ROOT}" STREQUAL "")
    set(YU_ASSET_SMOKE_UE_ENGINE_ROOT "D:/app/Epic Games/UE_5.5/Engine")
endif()

if(MSVC)
    add_compile_options(/source-charset:utf-8)
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

set(YU_ASSET_SMOKE_SOURCES
    Source/Main.cpp
)

if(YU_ASSET_SMOKE_ENGINE_EVIDENCE)
    if(NOT EXISTS "${YU_ASSET_SMOKE_ENGINE_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR "YU_ASSET_SMOKE_ENGINE_ROOT must point to the YuEngine repository root.")
    endif()

    if(NOT TARGET YuHardware)
        add_subdirectory("${YU_ASSET_SMOKE_ENGINE_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/YuEngine")
    endif()

    list(APPEND YU_ASSET_SMOKE_SOURCES
        Source/L0EngineEvidence.cpp
    )
endif()

add_executable(YuAssetSmokeDemo
    ${YU_ASSET_SMOKE_SOURCES}
)

target_compile_features(YuAssetSmokeDemo PRIVATE cxx_std_20)

target_include_directories(YuAssetSmokeDemo PRIVATE
    Source
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

if(YU_ASSET_SMOKE_ENGINE_EVIDENCE)
    target_compile_definitions(YuAssetSmokeDemo PRIVATE YU_ASSET_SMOKE_ENGINE_EVIDENCE=1)
    target_link_libraries(YuAssetSmokeDemo PRIVATE
        YuFile
        YuHardware
        YuRenderCore
        YuResource
        YuRHI
        YuStreaming
    )
endif()

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
