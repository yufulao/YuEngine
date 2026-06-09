cmake_minimum_required(VERSION 3.24)

project(YuEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(yuengine_runtime
    src/yuengine/core/Json.cpp
    src/yuengine/native/NativeRegistry.cpp
    src/yuengine/native/NativeServices.cpp
    src/yuengine/project/ProjectManifest.cpp
    src/yuengine/resource/ResourceDiagnostics.cpp
    src/yuengine/resource/VirtualFileSystem.cpp
    src/yuengine/runtime/EngineRuntime.cpp
    src/yuengine/runtime/RuntimeContext.cpp
    src/yuengine/script/ScriptRuntime.cpp
    src/yuengine/script/SqasmModule.cpp
)

target_include_directories(yuengine_runtime PUBLIC src)

if(MSVC)
    target_compile_options(yuengine_runtime PRIVATE /W4 /permissive-)
else()
    target_compile_options(yuengine_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(yuengine_cli apps/yuengine_cli/main.cpp)
target_link_libraries(yuengine_cli PRIVATE yuengine_runtime)

include(CTest)

if(BUILD_TESTING)
    add_test(
        NAME yuengine_validate_touhou_sample
        COMMAND yuengine_cli validate samples/touhou_new_world/project.json
    )
    set_tests_properties(yuengine_validate_touhou_sample PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    add_test(
        NAME yuengine_validate_empty_sample
        COMMAND yuengine_cli validate samples/empty_project/project.json
    )
    set_tests_properties(yuengine_validate_empty_sample PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

    add_test(
        NAME yuengine_boot_touhou_sample
        COMMAND yuengine_cli boot samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_boot_touhou_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=39"
        FAIL_REGULAR_EXPRESSION "\"status\": \"E1\""
    )

    add_test(
        NAME yuengine_boot_empty_sample
        COMMAND yuengine_cli boot samples/empty_project/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_boot_empty_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true phases=7 failed_phases=0 native_apis=84 native_services=11 obligations=0"
    )

    add_test(
        NAME yuengine_resources_touhou_sample
        COMMAND yuengine_cli resources samples/touhou_new_world/project.json
    )
    set_tests_properties(yuengine_resources_touhou_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "menu/title/title_back_sc.dds;menu/title/logo_sc.dds;menu/title/dlc_sc.dds;map/doujou/doujou.sge;map/doujou/doujou.rcm"
        FAIL_REGULAR_EXPRESSION "\"found\": false;\"query\": \"/\";88/88;titlemenu setup;C:\\\\DumpStack"
    )

    add_test(
        NAME yuengine_resources_empty_sample
        COMMAND yuengine_cli resources samples/empty_project/project.json
    )
    set_tests_properties(yuengine_resources_empty_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "\"ok\": true"
    )

    add_test(
        NAME yuengine_script_title_sample
        COMMAND yuengine_cli script samples/touhou_new_world/project.json script/menu/titlemenu.b64
    )
    set_tests_properties(yuengine_script_title_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "functions=81 instructions=4466 calls=593 resources=[0-9]+ closure_bindings=80"
    )

    add_test(
        NAME yuengine_script_first_mission_sample
        COMMAND yuengine_cli script samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64
    )
    set_tests_properties(yuengine_script_first_mission_sample PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "functions=62 instructions=4068 calls=640 resources=[0-9]+ closure_bindings=61"
    )

    add_test(
        NAME yuengine_native_services_surface
        COMMAND yuengine_cli native-services samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_native_services_surface PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "services=11 native_apis=84 unowned_apis=0 unbound_apis=0 not_started=84"
    )

    add_test(
        NAME yuengine_script_plan_title_entry
        COMMAND yuengine_cli script-plan samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_script_plan_title_entry PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "entry=setupProc found=true direct_calls=2 builtin_calls=1 native_obligations=0 object_method_calls=1 script_calls=0 ambiguous_calls=0 unresolved_calls=0 class_method_tables=8 object_bindings=1 status=vm_not_implemented"
    )

    add_test(
        NAME yuengine_script_run_title_entry
        COMMAND yuengine_cli script-run samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR} --frames 1
    )
    set_tests_properties(yuengine_script_run_title_entry PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "entry=setupProc found=true executed=true frames=1 baseline_modules=2 constructed_objects=3 script_methods=86 script_functions=5 builtin_calls=3 native_obligations=8 native_implemented_calls=0 unique_native_apis=4 engine_object_calls=14 ui_object_calls=16 value_helper_calls=6 value_method_calls=6 module_lifecycle_calls=0 bytecode_state_functions=95 bytecode_state_instructions=4129 root_slot_writes=31 class_slot_writes=405 object_field_writes=186 table_slot_writes=198 typed_call_returns=165 ui_object_mutations=0 service_state_events=41 save_service_queries=4 platform_state_queries=3 audio_service_commands=1 scene_service_commands=1 ui_objects_tracked=20 ui_service_commands=0 value_state_queries=4 decoded_service_arguments=6 optional_unbound_globals=0 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm;FadeIn.*duration=0.7.*MenuObject.*PlayBGM;runtime_service_state.*empty_save_list_queries.: 4.*current_bgm_id.: .3.*fade_in_duration.: .0.7.*created_objects.: 20.*command_count.: 0;runtime_script_state.*object_count.: 26.*class_base_count.: 16.*modTitle.*_nextState.*int.: 0.*_scenes.*modTitle._scenes.0.*TitleScene.*modTitle._scenes.1.*NewGameScene.*modTitle._scenes.2.*LoadScene.*modTitle._scenes.3.*OverwriteSaveScene"
    )

    add_test(
        NAME yuengine_script_run_title_new_game
        COMMAND yuengine_cli script-run samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR} --frames 5 --input-scenario title-new-game
    )
    set_tests_properties(yuengine_script_run_title_new_game PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "entry=setupProc found=true executed=true frames=5 baseline_modules=2 constructed_objects=3 script_methods=101 script_functions=12 builtin_calls=3 native_obligations=18 native_implemented_calls=0 unique_native_apis=11 engine_object_calls=18 ui_object_calls=22 value_helper_calls=6 value_method_calls=8 module_lifecycle_calls=0 bytecode_state_functions=117 bytecode_state_instructions=4515 root_slot_writes=31 class_slot_writes=405 object_field_writes=203 table_slot_writes=206 typed_call_returns=190 ui_object_mutations=0 service_state_events=64 save_service_queries=4 platform_state_queries=4 audio_service_commands=4 scene_service_commands=3 ui_objects_tracked=20 ui_service_commands=0 value_state_queries=14 decoded_service_arguments=21 optional_unbound_globals=0 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm;runtime_input_state.*title-new-game.*menu_selected_index.: 1.*menu_decide.: true;unique_native_api_names.*MakeNewGame.*StartGame;scenario_key_get.*scenario_key=sc01;make_new_game.*mission:sc01/main/ms010_0;start_game.*new_game=true;queue_scene_stage_load.*mission/sc01/main/ms010_0.b64.sqasm.*map/Doujou/doujou.sge.*map/Doujou/doujou.rcm;runtime_service_state.*make_new_game_commands.: 1.*start_game_commands.: 1.*started_mission.: .mission:sc01/main/ms010_0.*queued_stage_loads.: 1.*current_mission_script.: .mission/sc01/main/ms010_0.b64.sqasm.*current_stage.: .map/Doujou/doujou.sge.*current_rail_camera.: .map/Doujou/doujou.rcm;runtime_script_state.*modTitle.*_nextState.*int.: 300.*modTitle._scenes.1.*NewGameScene.*_state.*int.: 1"
    )
endif()
