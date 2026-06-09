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
    src/yuengine/runtime/FrameRuntime.cpp
    src/yuengine/runtime/RuntimeContext.cpp
    src/yuengine/runtime/SceneRuntime.cpp
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
        PASS_REGULAR_EXPRESSION "entry=setupProc found=true executed=true frames=5 baseline_modules=2 constructed_objects=3 script_methods=101 script_functions=12 builtin_calls=3 native_obligations=18 native_implemented_calls=0 unique_native_apis=11 engine_object_calls=20 ui_object_calls=22 value_helper_calls=6 value_method_calls=6 module_lifecycle_calls=0 bytecode_state_functions=117 bytecode_state_instructions=4515 root_slot_writes=31 class_slot_writes=405 object_field_writes=203 table_slot_writes=206 typed_call_returns=190 ui_object_mutations=0 service_state_events=66 save_service_queries=4 platform_state_queries=4 audio_service_commands=4 scene_service_commands=3 ui_objects_tracked=20 ui_service_commands=0 value_state_queries=14 decoded_service_arguments=23 optional_unbound_globals=0 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm;runtime_input_state.*title-new-game.*menu_selected_index.: 1.*menu_decide.: true;unique_native_api_names.*MakeNewGame.*StartGame;scenario_key_get.*scenario_key=sc01;make_new_game.*mission:sc01/main/ms010_0;start_game.*new_game=true;queue_scene_stage_load.*mission/sc01/main/ms010_0.b64.sqasm.*map/Doujou/doujou.sge.*map/Doujou/doujou.rcm;runtime_service_state.*make_new_game_commands.: 1.*start_game_commands.: 1.*started_mission.: .mission:sc01/main/ms010_0.*queued_stage_loads.: 1.*current_mission_script.: .mission/sc01/main/ms010_0.b64.sqasm.*current_stage.: .map/Doujou/doujou.sge.*current_rail_camera.: .map/Doujou/doujou.rcm;runtime_script_state.*modTitle.*_nextState.*int.: 300.*modTitle._scenes.1.*NewGameScene.*_state.*int.: 1"
    )

    add_test(
        NAME yuengine_script_run_first_mission_setup
        COMMAND yuengine_cli script-run samples/touhou_new_world/project.json mission/sc01/main/ms010_0.b64 setupProcess --repo-root ${CMAKE_SOURCE_DIR} --frames 1
    )
    set_tests_properties(yuengine_script_run_first_mission_setup PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "entry=setupProcess found=true executed=true frames=1 baseline_modules=2 constructed_objects=0 script_methods=0 script_functions=5 builtin_calls=22 native_obligations=21 native_implemented_calls=0 unique_native_apis=19 engine_object_calls=9 ui_object_calls=0 value_helper_calls=0 value_method_calls=3 module_lifecycle_calls=0 bytecode_state_functions=9 bytecode_state_instructions=2245 root_slot_writes=41 class_slot_writes=294 object_field_writes=0 table_slot_writes=200 typed_call_returns=57 ui_object_mutations=0 service_state_events=27 save_service_queries=0 platform_state_queries=0 audio_service_commands=1 scene_service_commands=10 ui_objects_tracked=0 ui_service_commands=0 value_state_queries=0 decoded_service_arguments=19 optional_unbound_globals=0 unresolved_calls=0 truncated=false status=trace_ready_not_full_vm;LoadEventsScriptViaMission.*load_events_script_via_mission.*PushPlayerChara.*push_player_chara.*LoadRailCamera.*load_rail_camera;runtime_service_state.*loaded_stage_commands.: 1.*current_stage.: .map/Doujou/doujou.sge.*current_event_script.: .sc01/main/ms010_0.*push_player_chara_commands.: 1.*current_player_chara.: .reimuEx.*rail_camera_path.: .map/Doujou/doujou.rcm.*checkpoint_commands.: 1.*current_checkpoint.: .toVec3\\(marker:sc01/main/ms010_0:eventMap._pos\\)"
    )

    add_test(
        NAME yuengine_scene_entry_title_new_game
        COMMAND yuengine_cli scene-entry samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_scene_entry_title_new_game PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true title_new_game_executed=true mission_setup_executed=true stage_ready=true actor_ready=true camera_ready=true event_ready=true resource_bindings=6 missing_resources=0 script_bindings=5 missing_scripts=0;started_mission.: .mission:sc01/main/ms010_0.*queued_mission_script.: .mission/sc01/main/ms010_0.b64.sqasm.*stage.: .map/Doujou/doujou.sge.*stage_model.: .map/Doujou/doujou.mdl.*stage_collision.: .map/Doujou/doujou.col.*rail_camera.: .map/Doujou/doujou.rcm;player_chara.: .reimuEx.*player_script_asset.: .player/reimuex.b64.*player_pcg_asset.: .player/reimuex_pcg.b64;role.: .stage_sge.*found.: true;role.: .rail_camera.*found.: true"
    )

    add_test(
        NAME yuengine_scene_runtime_materialization
        COMMAND yuengine_cli scene-runtime samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_scene_runtime_materialization PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_entry_ok=true stage_handle_ready=true actor_handle_ready=true camera_handle_ready=true event_marker_ready=true stage_dependencies=42 missing_stage_dependencies=0 model_meshes=111 collision_triangles=150 rail_nodes=3;stage_byte_size.: 10176.*model_dependency_count.: 2.*collision_dependency_count.: 1.*texture_dependency_count.: 39;collision_vertex_count.: 272.*collision_index_count.: 450.*collision_triangle_count.: 150;player_chara.: .reimuEx.*player_script_bytes.: 406.*player_pcg_bytes.: 13645;rail_node_count_candidate.: 3;marker.: .marker:sc01/main/ms010_0:eventMap"
    )

    add_test(
        NAME yuengine_first_frame_runtime_contract
        COMMAND yuengine_cli first-frame samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_first_frame_runtime_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true renderer_frame_ready=true actor_frame_ready=true camera_frame_ready=true input_frame_ready=true event_frame_ready=true mesh_draw_candidates=111 texture_bindings=39 collision_triangles=150 actor_instances=1 rail_nodes=3 event_markers=1;renderer_profile.: .d3d9_compatible.*material_bindings.: 16.*stage_dependency_count.: 42;player_chara.: .reimuEx.*spawn_position_expression.: .toVec3\\(marker:sc01/main/ms010_0:eventMap._pos\\);camera_source.: .map/doujou/doujou.rcm.*default_target.: .ev_sc01_main_ms010_0;owner_service.: .Actor And Task Service.*control_scope.: .player_actor_camera_scene;marker.: .marker:sc01/main/ms010_0:eventMap"
    )

    add_test(
        NAME yuengine_title_ui_command_payload_contract
        COMMAND yuengine_cli title-ui samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_title_ui_command_payload_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true title_setup_found=true title_setup_executed=true title_render_executed=true entry=setupProc created_objects=26 command_count=55 draw_commands=9 graph_string_commands=5 string_size_queries=5 text_draw_commands=6 graph_draw_commands=3 color_commands=11 localized_menu_text_commands=10 draw_list_item_commands=5 background_resource_bound=true logo_resource_bound=true unresolved_calls=0 truncated=false;native_obligations.: 30.*unique_native_apis.: 8.*service_state_events.: 158.*ui_object_calls.: 72.*ui_service_commands.: 55;last_command.: .menu/title/logo_sc.dds"
    )

    add_test(
        NAME yuengine_title_branches_runtime_contract
        COMMAND yuengine_cli title-branches samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_title_branches_runtime_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scenario_count=8 executed_scenarios=8 start_game_scenarios=3 load_auto_save_scenarios=2 make_new_game_scenarios=1 shutdown_permission_scenarios=2 shutdown_game_scenarios=1 option_ui_mutations=2 unresolved_calls=0 truncated=false;title-continue.*load_auto_save_commands.: 2.*start_game_commands.: 1.*queued_stage_loads.: 1;title-new-game.*scenario_key_get_queries.: 1.*make_new_game_commands.: 1.*start_game_commands.: 1.*start_new_game.: .true;title-load.*load_auto_save_commands.: 1.*start_game_commands.: 1.*queued_stage_loads.: 1;title-option.*ui_object_mutations.: 2.*start_game_commands.: 0;title-exit-denied.*shutdown_permission.: .false.*shutdown_game_commands.: 0;title-exit-allowed.*shutdown_permission.: .true.*shutdown_game_commands.: 3.*shutdown_requested.: .requested"
    )

    add_test(
        NAME yuengine_gameplay_frame_runtime_contract
        COMMAND yuengine_cli gameplay-frame samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_gameplay_frame_runtime_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true title_ui_ok=true title_branches_ok=true mission_event_thread_ok=true mission_tutorial_ok=true frame_updates=2 renderer_frame_ready=true ui_frame_ready=true save_frame_ready=true actor_frame_ready=true camera_frame_ready=true input_frame_ready=true event_frame_ready=true audio_frame_ready=true gameplay_command_count=221;mesh_draw_candidates.: 111.*material_bindings.: 16.*texture_bindings.: 39;title_ui_commands.: 55.*title_ui_draw_commands.: 9;start_game_scenarios.: 3.*load_auto_save_scenarios.: 2.*make_new_game_scenarios.: 1;actor_instances.: 1.*player_control_commands.: 6;camera_commands.: 2.*rail_nodes.: 3;event_commands.: 16.*tutorial_update_commands.: 1;audio_commands.: 28"
    )

    add_test(
        NAME yuengine_mission_event_thread_contract
        COMMAND yuengine_cli mission-event-thread samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_mission_event_thread_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true event_thread_found=true event_thread_executed=true entry=threadEvent0000_00 player_control_commands=2 player_control_enabled=true reset_menu_button_holding_times_commands=1 dialog_reset_commands=1 dialog_hide_commands=1 reset_player_action_commands=1 event_unit_queries=1 event_page_setup_commands=1 event_page_done_commands=1 event_volume_creates=0 event_volume_activation_commands=1 last_event_volume_enabled=false set_game_camera_if_not_commands=1 unresolved_calls=0 truncated=false;event_page.*done_commands.: 1.*camera.*set_game_camera_if_not_commands.: 1"
    )

    add_test(
        NAME yuengine_mission_tutorial_contract
        COMMAND yuengine_cli mission-tutorial samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_mission_tutorial_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true tutorial_thread_found=true tutorial_thread_executed=true update_units_executed=true entry=threadEvent0020_00 event_flag_add_commands=1 current_player_name_queries=3 dialog_show_commands=3 dialog_speak_commands=3 dialog_wait_commands=3 dialog_hide_commands=4 tutorial_actor_creates=1 tutorial_page_creates=1 push_actor_commands=1 wait_actor_commands=1 player_control_commands=4 player_control_enabled=true set_player_angle_y_commands=1 land_player_commands=1 update_units_commands=1 enter_transition_commands=1 leave_transition_commands=1 unresolved_calls=0 truncated=false;last_dialog_text.: .arg0=0.*localized:A69E:55F2.*tutorial_actor.*last_page.: .tutorial_page:actor:tutorial::135:0.*event_state.*update_units_commands.: 1"
    )
endif()
