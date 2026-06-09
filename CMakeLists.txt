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

if(WIN32)
    target_link_libraries(yuengine_runtime PRIVATE user32)
endif()

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
        NAME yuengine_renderer_backend_submission_contract
        COMMAND yuengine_cli renderer-submit samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_renderer_backend_submission_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true title_ui_ok=true gameplay_frame_ok=true backend_frame_ready=true title_pass_ready=true world_pass_ready=true resource_upload_ready=true camera_submission_ready=true actor_submission_ready=true event_submission_ready=true submission_passes=3 backend_command_count=181 draw_submissions=121;renderer_profile.: .d3d9_compatible.*resource_upload_submissions.: 57.*backend_obligations.: 6;title_2d_submissions.: 55.*title_graph_submissions.: 3.*title_text_submissions.: 11;scene_mesh_submissions.: 111.*material_bindings.: 16.*texture_bindings.: 39.*collision_debug_submissions.: 1;camera_submissions.: 1.*actor_submissions.: 1.*event_marker_submissions.: 1"
    )

    add_test(
        NAME yuengine_frame_scheduler_update_graph_contract
        COMMAND yuengine_cli frame-scheduler samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_frame_scheduler_update_graph_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true gameplay_frame_ok=true renderer_submission_ok=true update_graph_ready=true frame_index=0 node_count=10 executed_nodes=10 service_node_count=10 scheduler_edges=12 gameplay_commands=221 renderer_backend_commands=181 scheduled_work_items=469 backend_obligations=6 unresolved_nodes=0;project_lifecycle.*commands.: 1;script_tick_render.*commands.: 55;save_profile_update.*commands.: 6;scene_resource_update.*commands.: 166;actor_task_update.*commands.: 7;camera_update.*commands.: 2;event_tutorial_update.*commands.: 17;audio_update.*commands.: 28;renderer_submission.*commands.: 181;backend_obligation_tracking.*commands.: 6"
    )

    add_test(
        NAME yuengine_backend_obligations_contract
        COMMAND yuengine_cli backend-obligations samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_backend_obligations_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true frame_scheduler_ok=true renderer_submission_ok=true texture_upload_contract_ready=true material_binding_contract_ready=true shader_effect_contract_tracked=true font_contract_tracked=true device_contract_tracked=true oracle_parity_contract_tracked=true texture_dependencies=39 texture_bytes_found=39 dds_textures=39 material_bindings=16 mesh_submissions=111 resolved_backend_contracts=2 tracked_backend_obligations=4 open_backend_obligations=4;texture_upload.*byte_total.: [1-9][0-9]+;material_binding.*title_text_submissions.: 11;texture_upload_format_semantics.*contract_ready;material_binding_semantics.*contract_ready;shader_effect_semantics.*tracked_open;font_atlas_and_glyph_metrics.*tracked_open;device_swapchain_presentation.*tracked_open;original_frame_oracle_parity.*tracked_open"
    )

    add_test(
        NAME yuengine_material_semantics_contract
        COMMAND yuengine_cli material-semantics samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_material_semantics_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true renderer_submission_ok=true backend_obligations_ok=true material_semantics_contract_ready=true texture_slot_contract_ready=true mesh_material_contract_ready=true shader_effect_contract_tracked=true post_effect_source_tracked=true materials=16 material_parameter_blocks=16 texture_slots=39 resolved_texture_slots=39 mesh_submissions=111 named_mesh_submissions=110 mesh_material_bindings=110 unresolved_mesh_material_bindings=1 post_effect_techniques=5 post_effect_passes=5 post_effect_samplers=7;material_summary.*unresolved_texture_slots.: 0.*unresolved_mesh_material_bindings.: 1;post_effect.*techniques.: 5.*passes.: 5.*samplers.: 7;kabe1.*mesh_binding_count.: 22.*kabeb_d.dds;ki:tree.*lightmap_candidate.*t_treeb_lm.dds;material_shader_program_semantics.*tracked_open;unnamed_mesh_material_binding.*tracked_open"
    )

    add_test(
        NAME yuengine_device_presentation_contract
        COMMAND yuengine_cli device-presentation samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_device_presentation_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true frame_scheduler_ok=true renderer_submission_ok=true backend_obligations_ok=true material_semantics_ok=true device_profile_ready=true swapchain_contract_tracked=true resource_upload_plan_ready=true render_state_contract_tracked=true draw_queue_contract_ready=true present_contract_tracked=true renderer_profile=d3d9_compatible backbuffer_width_candidate=1280 backbuffer_height_candidate=720 renderer_backend_commands=181 resource_upload_submissions=57 draw_submissions=121 material_texture_slots=39 texture_bytes_found=39 post_effect_samplers=7 resolved_device_contracts=3 tracked_device_obligations=5 open_device_obligations=5;device.*backbuffer_width_candidate.: 1280.*backbuffer_height_candidate.: 720;submission.*resource_upload_submissions.: 57.*draw_submissions.: 121.*world_mesh_submissions.: 111;render_state.*post_effect_techniques.: 5.*post_effect_passes.: 5.*post_effect_samplers.: 7;d3d9_device_profile.*contract_ready;resource_upload_plan.*contract_ready;draw_queue_submission.*contract_ready;swapchain_backbuffer_candidate.*tracked_open;present_call_and_window_surface.*tracked_open"
    )

    add_test(
        NAME yuengine_texture_upload_contract
        COMMAND yuengine_cli texture-upload samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_texture_upload_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true scene_runtime_ok=true backend_obligations_ok=true material_semantics_ok=true device_presentation_ok=true texture_upload_runtime_ready=true dds_header_contract_ready=true payload_layout_contract_ready=true material_consumer_contract_ready=true sampler_state_gate_tracked=true blend_depth_state_gate_tracked=true font_atlas_gate_tracked=true oracle_parity_gate_tracked=true stage_texture_dependencies=39 texture_upload_records=39 valid_dds_headers=39 dxt1_textures=31 dxt5_textures=8 cube_map_textures=1 cube_map_faces=6 material_slot_consumers=39 unique_material_texture_uploads=38 duplicate_material_consumers=1 stage_only_texture_uploads=1 compressed_payload_matches=39 payload_byte_total=23768416 expected_payload_byte_total=23768416 mip9_textures=2 mip10_textures=14 mip11_textures=23 texture_width_min=256 texture_width_max=1024 post_effect_samplers=7 title_text_submissions=11 resolved_upload_contracts=4 tracked_upload_obligations=4 open_upload_obligations=4;dds_summary.*unsupported_texture_formats.: 0.*texture_height_max.: 1024;payload_layout.*texture_byte_total.: 23773408.*compressed_payload_matches.: 39;material_consumers.*cube_map_faces.: 6;gates.*resource_upload_submissions.: 57.*title_string_size_queries.: 5.*localized_menu_text_commands.: 10;cubeenvmap/doujou_1.dds.*cube_environment_candidate.*format.: .DXT1.*width.: 256.*cube_faces.: 6;kabe_d.dds.*base_color_candidate.*format.: .DXT5.*width.: 1024.*material_consumer_count.: 1;kamidana_d.dds.*base_color_candidate.*format.: .DXT5.*material_consumer_count.: 2;dds_header_decode.*contract_ready;dds_compressed_payload_layout.*contract_ready;texture_material_consumer_map.*contract_ready;sampler_state_values.*tracked_open;font_atlas_glyph_metrics.*tracked_open;original_frame_oracle_trace.*tracked_open"
    )

    add_test(
        NAME yuengine_backend_state_contract
        COMMAND yuengine_cli backend-state samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_backend_state_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true texture_upload_ok=true device_presentation_ok=true material_semantics_ok=true title_ui_ok=true backend_state_runtime_ready=true sampler_state_records_ready=true pass_render_state_records_ready=true font_atlas_records_ready=true material_shader_program_gate_tracked=true gpu_state_binding_gate_tracked=true oracle_parity_gate_tracked=true sampler_state_records=7 sampler_texture_bindings=7 sampler_clamp_address_records=7 sampler_linear_min_filters=6 sampler_point_min_filters=1 sampler_srgb_true_records=1 sampler_srgb_false_records=6 pass_state_records=5 pass_vs30_shaders=5 pass_ps30_shaders=5 z_disabled_passes=5 alpha_blend_disabled_passes=5 alpha_test_disabled_passes=5 srgb_write_enabled_passes=1 srgb_write_disabled_passes=4 stencil_enabled_passes=4 stencil_disabled_passes=1 stencil_replace_passes=3 stencil_keep_passes=1 stencil_equal_passes=1 font_query_records=6 text_draw_commands=6 graph_string_commands=5 string_size_queries=5 localized_menu_text_commands=10 texture_upload_records=39 material_texture_consumers=39 resolved_backend_state_contracts=4 tracked_backend_state_obligations=4 open_backend_state_obligations=4;sampler_summary.*linear_min_filters.: 6.*point_min_filters.: 1.*srgb_true_records.: 1;pass_state_summary.*records.: 5.*stencil_replace_passes.: 3.*stencil_keep_passes.: 1.*stencil_equal_passes.: 1;font_atlas_record.*font_queries.: 6.*font_scale_limits.: 0.*glyph_metric_inputs_ready.: true.*atlas_implementation_tracked.: true;colorTex.*colorTex2D.*mip_filter.: .Point.*srgb_texture.: .true;searchTex.*searchTex2D.*address_w.: .Clamp.*min_filter.: .Point.*mag_filter.: .Point;BlendWeightCalculation.*stencil_pass.: .KEEP.*stencil_func.: .EQUAL;NeighborhoodBlending.*srgb_write_enable.: .true.*stencil_enable.: .false;smaa_sampler_state_records.*contract_ready;smaa_pass_render_state_records.*contract_ready;title_font_glyph_metric_inputs.*contract_ready;backend_state_consumes_texture_uploads.*contract_ready;gpu_state_binding_execution.*tracked_open;font_atlas_texture_implementation.*tracked_open;original_frame_oracle_trace.*tracked_open"
    )

    add_test(
        NAME yuengine_resource_allocation_contract
        COMMAND yuengine_cli resource-allocation samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_resource_allocation_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true texture_upload_ok=true backend_state_ok=true resource_allocation_runtime_ready=true stage_texture_allocation_records_ready=true smaa_lookup_allocation_records_ready=true transient_surface_allocation_gate_tracked=true font_atlas_allocation_gate_tracked=true d3d_resource_creation_gate_tracked=true oracle_parity_gate_tracked=true allocation_records=46 ready_allocation_records=41 tracked_open_allocation_records=5 stage_texture_allocations=39 d3d_dxt1_allocations=31 d3d_dxt5_allocations=8 cube_texture_allocations=1 smaa_lookup_allocations=2 lookup_l8_allocations=1 lookup_a8l8_allocations=1 transient_surface_candidates=4 font_atlas_placeholders=1 sampler_texture_declarations=6 material_texture_consumers=39 ready_allocation_byte_total=23955042 ready_allocation_payload_bytes=23949794 ready_expected_payload_bytes=23949794 resolved_allocation_contracts=2 tracked_allocation_obligations=5 open_allocation_obligations=5;allocation_summary.*stage_texture_allocations.: 39.*smaa_lookup_allocations.: 2.*transient_surface_candidates.: 4.*font_atlas_placeholders.: 1;format_summary.*d3d_dxt1_allocations.: 31.*d3d_dxt5_allocations.: 8.*lookup_l8_allocations.: 1.*lookup_a8l8_allocations.: 1.*cube_texture_allocations.: 1;ready_bytes.*ready_allocation_byte_total.: 23955042.*ready_allocation_payload_bytes.: 23949794.*ready_expected_payload_bytes.: 23949794;cubeenvmap/doujou_1.dds.*cube_texture.*stage_environment_cube.*D3DFMT_DXT1.*cube_faces.: 6;system/glsl/smaa/areatexdx9.dds.*smaa_lookup_texture.*D3DFMT_A8L8.*width.: 160.*height.: 560.*payload_bytes.: 179200;system/glsl/smaa/searchtex.dds.*D3DFMT_L8.*width.: 66.*height.: 33.*payload_bytes.: 2178;edgesTex2D.*smaa_transient_surface.*tracked_open.*width.: 1280.*height.: 720;font_atlas_placeholder.*font_atlas_texture.*tracked_open;stage_texture_allocation_records.*contract_ready;smaa_lookup_texture_allocation_records.*contract_ready;smaa_transient_surface_allocation_candidates.*tracked_open;font_atlas_allocation_placeholder.*tracked_open;d3d_resource_creation_execution.*tracked_open;material_shader_resource_binding.*tracked_open;original_frame_oracle_trace.*tracked_open"
    )

    add_test(
        NAME yuengine_device_execution_contract
        COMMAND yuengine_cli device-execution samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_device_execution_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true resource_allocation_ok=true backend_state_ok=true device_presentation_ok=true device_execution_runtime_ready=true resource_creation_records_ready=true upload_execution_records_ready=true state_binding_records_ready=true lookup_texture_binding_records_ready=true material_texture_binding_gate_tracked=true transient_surface_binding_gate_tracked=true font_atlas_execution_gate_tracked=true d3d_api_call_submission_gate_tracked=true present_oracle_gate_tracked=true resource_creation_records=46 ready_resource_creation_records=41 tracked_open_resource_creation_records=5 create_texture_records=40 create_cube_texture_records=1 render_target_creation_candidates=3 depth_stencil_creation_candidates=1 font_atlas_creation_placeholders=1 texture_upload_execution_records=41 upload_subresource_records=458 ready_upload_payload_bytes=23949794 binding_records=57 ready_binding_records=14 tracked_open_binding_records=43 material_texture_binding_records=38 material_texture_binding_slots=39 sampler_texture_binding_records=7 lookup_texture_binding_records=2 transient_sampler_binding_candidates=5 sampler_state_binding_records=7 render_state_binding_records=5 resolved_device_execution_contracts=4 tracked_device_execution_obligations=5 open_device_execution_obligations=5;resource_creation_summary.*create_texture_records.: 40.*create_cube_texture_records.: 1.*render_target_candidates.: 3.*depth_stencil_candidates.: 1.*font_atlas_placeholders.: 1;upload_execution_summary.*texture_upload_execution_records.: 41.*upload_subresource_records.: 458.*ready_upload_payload_bytes.: 23949794;binding_summary.*material_texture_binding_records.: 38.*material_texture_binding_slots.: 39.*sampler_texture_binding_records.: 7.*lookup_texture_binding_records.: 2.*transient_sampler_binding_candidates.: 5.*sampler_state_binding_records.: 7.*render_state_binding_records.: 5;cubeenvmap/doujou_1.dds.*CreateCubeTexture.*D3DFMT_DXT1.*cube_faces.: 6.*subresource_count.: 54;system/glsl/smaa/areatexdx9.dds.*CreateTexture.*D3DFMT_A8L8.*subresource_count.: 1;system/glsl/smaa/searchtex.dds.*CreateTexture.*D3DFMT_L8.*subresource_count.: 1;colorTex2D.*CreateRenderTargetCandidate.*tracked_open.*width.: 1280.*height.: 720;depthTex2D.*CreateDepthStencilSurfaceCandidate.*tracked_open.*width.: 1280.*height.: 720;kamidana_d.dds.*SetTextureCandidate.*material_texture_slot.*binding_slots.: 2;areaTex.*SetTexture.*areaTex2D.*contract_ready;searchTex.*SetTexture.*searchTex2D.*contract_ready;colorTex.*SetTextureCandidate.*colorTex2D.*tracked_open;BlendWeightCalculation.BlendWeightCalculation.*SetRenderStateBundle.*contract_ready;device_resource_creation_records.*contract_ready;texture_upload_subresource_records.*contract_ready;smaa_lookup_texture_bindings.*contract_ready;sampler_and_render_state_value_records.*contract_ready;material_texture_shader_binding.*tracked_open;smaa_transient_surface_creation_and_binding.*tracked_open;font_atlas_device_resource.*tracked_open;d3d_api_call_submission.*tracked_open;present_and_original_frame_oracle.*tracked_open"
    )

    add_test(
        NAME yuengine_present_oracle_contract
        COMMAND yuengine_cli present-oracle samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_present_oracle_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true device_execution_ok=true device_presentation_ok=true presentation_oracle_runtime_ready=true backbuffer_extent_records_ready=true device_execution_input_records_ready=true window_surface_gate_tracked=true swapchain_creation_gate_tracked=true present_call_gate_tracked=true frame_capture_gate_tracked=true original_frame_oracle_gate_tracked=true presentation_records=7 ready_presentation_records=2 tracked_open_presentation_records=5 backbuffer_extent_records=1 backbuffer_width=1280 backbuffer_height=720 device_execution_frame_inputs=1 linked_device_execution_records=103 linked_device_ready_records=55 linked_device_open_records=48 window_surface_candidates=1 swapchain_creation_candidates=1 present_call_candidates=1 frame_capture_candidates=1 oracle_trace_candidates=1 renderer_backend_commands=181 draw_submissions=121 resolved_presentation_contracts=2 tracked_presentation_obligations=5 open_presentation_obligations=5;presentation_summary.*records.: 7.*ready_records.: 2.*tracked_open_records.: 5.*backbuffer_width.: 1280.*backbuffer_height.: 720.*renderer_backend_commands.: 181.*draw_submissions.: 121;device_execution_link.*device_execution_frame_inputs.: 1.*linked_device_execution_records.: 103.*linked_device_ready_records.: 55.*linked_device_open_records.: 48;oracle_gates.*window_surface_candidates.: 1.*swapchain_creation_candidates.: 1.*present_call_candidates.: 1.*frame_capture_candidates.: 1.*oracle_trace_candidates.: 1;swapchain_backbuffer_extent.*BackbufferExtent.*SMAA_PIXEL_SIZE.*contract_ready.*width.: 1280.*height.: 720;device_execution_frame_input.*DeviceExecutionFrameInput.*device-execution.*contract_ready.*linked_record_count.: 103;window_surface_candidate.*CreateWindowSurfaceCandidate.*tracked_open;swapchain_creation_candidate.*CreateSwapChainCandidate.*tracked_open;present_call_candidate.*PresentCandidate.*tracked_open.*linked_record_count.: 181;frame_capture_candidate.*FrameCaptureCandidate.*tracked_open.*linked_record_count.: 121;original_frame_oracle_trace.*OriginalFrameOracleTraceCandidate.*tracked_open;backbuffer_extent_record.*contract_ready;device_execution_frame_input.*contract_ready;window_surface_creation.*tracked_open;swapchain_creation.*tracked_open;present_call_execution.*tracked_open;frame_capture_artifact.*tracked_open;original_frame_oracle_trace.*tracked_open"
    )

    add_test(
        NAME yuengine_platform_bridge_contract
        COMMAND yuengine_cli platform-bridge samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_platform_bridge_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true device_execution_ok=true presentation_oracle_ok=true platform_bridge_runtime_ready=true diagnostic_bridge_ready=true creation_submission_queue_ready=true upload_submission_queue_ready=true state_submission_queue_ready=true presentation_submission_queue_tracked=true d3d_concrete_backend_gate_tracked=true frame_capture_gate_tracked=true original_oracle_gate_tracked=true bridge_call_records=10 ready_bridge_call_records=4 tracked_open_bridge_call_records=6 diagnostic_call_batches=1 platform_surface_records=1 d3d_interface_records=1 create_device_records=1 resource_creation_call_records=46 texture_create_calls=40 cube_texture_create_calls=1 render_target_create_candidates=3 depth_stencil_create_candidates=1 font_atlas_create_candidates=1 upload_call_records=41 upload_subresource_calls=458 state_binding_call_records=57 ready_state_binding_calls=14 tracked_open_state_binding_calls=43 set_texture_calls=45 set_sampler_state_calls=7 set_render_state_bundle_calls=5 draw_submission_calls=121 present_call_records=1 capture_oracle_call_records=2 linked_device_execution_records=103 linked_presentation_records=7 ready_platform_input_records=57 tracked_open_platform_input_records=53 renderer_backend_commands=181 draw_submissions=121 backbuffer_width=1280 backbuffer_height=720 resolved_platform_bridge_contracts=4 tracked_platform_bridge_obligations=6 open_platform_bridge_obligations=6;bridge_summary.*records.: 10.*ready_records.: 4.*tracked_open_records.: 6.*diagnostic_call_batches.: 1.*renderer_backend_commands.: 181.*draw_submissions.: 121.*backbuffer_width.: 1280.*backbuffer_height.: 720;input_link.*linked_device_execution_records.: 103.*linked_presentation_records.: 7.*ready_platform_input_records.: 57.*tracked_open_platform_input_records.: 53;platform_calls.*platform_surface_records.: 1.*d3d_interface_records.: 1.*create_device_records.: 1.*resource_creation_call_records.: 46.*texture_create_calls.: 40.*cube_texture_create_calls.: 1.*upload_subresource_calls.: 458.*state_binding_call_records.: 57.*draw_submission_calls.: 121.*present_call_records.: 1.*capture_oracle_call_records.: 2;diagnostic_backend_bridge.*BackendBridge::submitFrame.*contract_ready.*input_records.: 110.*ready_inputs.: 57.*tracked_open_inputs.: 53;platform_window_surface.*CreateWindowSurface.*tracked_open.*width.: 1280.*height.: 720;d3d9_interface_creation.*Direct3DCreate9.*tracked_open;d3d9_device_creation.*CreateDevice.*tracked_open.*width.: 1280.*height.: 720;d3d9_resource_creation_queue.*CreateTexture/CreateCubeTexture/CreateRenderTarget/CreateDepthStencilSurface.*contract_ready.*call_count.: 46;d3d9_texture_upload_queue.*LockRect/UnlockRect.*contract_ready.*call_count.: 458;d3d9_state_binding_queue.*SetTexture/SetSamplerState/SetRenderState.*contract_ready.*call_count.: 57;d3d9_draw_submission_queue.*DrawIndexedPrimitive/DrawPrimitiveUP.*tracked_open.*call_count.: 121;d3d9_present_call.*Present.*tracked_open.*call_count.: 1;frame_capture_oracle_queue.*GetFrontBufferData/oracle_compare.*tracked_open.*call_count.: 2;diagnostic_backend_bridge.*contract_ready;resource_creation_submission_queue.*contract_ready;texture_upload_submission_queue.*contract_ready;state_binding_submission_queue.*contract_ready;platform_window_surface.*tracked_open;d3d9_interface_and_device_creation.*tracked_open;draw_submission_backend.*tracked_open;present_execution.*tracked_open;frame_capture_artifact.*tracked_open;original_frame_oracle_parity.*tracked_open"
    )

    add_test(
        NAME yuengine_backend_executor_contract
        COMMAND yuengine_cli backend-executor samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_backend_executor_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true platform_bridge_ok=true executor_runtime_ready=true diagnostic_executor_ready=true one_to_one_bridge_mapping_ready=true ready_call_execution_results_ready=true tracked_open_call_results_ready=true concrete_d3d_execution_gate_tracked=true hwnd_device_gate_tracked=true draw_execution_gate_tracked=true present_execution_gate_tracked=true frame_capture_gate_tracked=true original_oracle_gate_tracked=true execution_result_records=10 diagnostic_success_records=4 tracked_open_execution_records=6 blocked_execution_records=0 consumed_bridge_call_records=10 ready_bridge_call_records=4 tracked_open_bridge_call_records=6 result_call_count_total=689 diagnostic_executed_calls=562 preserved_open_calls=127 submitted_diagnostic_batches=1 executed_resource_creation_calls=46 executed_upload_subresource_calls=458 executed_state_binding_calls=57 preserved_platform_surface_gates=1 preserved_device_creation_gates=2 preserved_draw_submission_gates=121 preserved_present_gates=1 preserved_capture_oracle_gates=2 linked_platform_input_records=110 ready_platform_input_records=57 tracked_open_platform_input_records=53 backbuffer_width=1280 backbuffer_height=720 resolved_executor_contracts=4 tracked_executor_obligations=6 open_executor_obligations=6;executor_summary.*execution_result_records.: 10.*diagnostic_success_records.: 4.*tracked_open_execution_records.: 6.*blocked_execution_records.: 0.*result_call_count_total.: 689.*diagnostic_executed_calls.: 562.*preserved_open_calls.: 127;bridge_link.*consumed_bridge_call_records.: 10.*ready_bridge_call_records.: 4.*tracked_open_bridge_call_records.: 6.*linked_platform_input_records.: 110.*ready_platform_input_records.: 57.*tracked_open_platform_input_records.: 53;diagnostic_execution.*submitted_diagnostic_batches.: 1.*executed_resource_creation_calls.: 46.*executed_upload_subresource_calls.: 458.*executed_state_binding_calls.: 57.*backbuffer_width.: 1280.*backbuffer_height.: 720;preserved_gates.*preserved_platform_surface_gates.: 1.*preserved_device_creation_gates.: 2.*preserved_draw_submission_gates.: 121.*preserved_present_gates.: 1.*preserved_capture_oracle_gates.: 2;diagnostic_backend_bridge_result.*BackendBridge::submitFrame.*diagnostic_d3d9_adapter.*diagnostic_success.*executed_calls.: 1;platform_window_surface_result.*CreateWindowSurface.*tracked_open.*preserved_open_calls.: 1;d3d9_interface_creation_result.*Direct3DCreate9.*tracked_open.*preserved_open_calls.: 1;d3d9_device_creation_result.*CreateDevice.*tracked_open.*preserved_open_calls.: 1;d3d9_resource_creation_queue_result.*CreateTexture/CreateCubeTexture/CreateRenderTarget/CreateDepthStencilSurface.*diagnostic_success.*executed_calls.: 46;d3d9_texture_upload_queue_result.*LockRect/UnlockRect.*diagnostic_success.*executed_calls.: 458;d3d9_state_binding_queue_result.*SetTexture/SetSamplerState/SetRenderState.*diagnostic_success.*executed_calls.: 57;d3d9_draw_submission_queue_result.*DrawIndexedPrimitive/DrawPrimitiveUP.*tracked_open.*preserved_open_calls.: 121;d3d9_present_call_result.*Present.*tracked_open.*preserved_open_calls.: 1;frame_capture_oracle_queue_result.*GetFrontBufferData/oracle_compare.*tracked_open.*preserved_open_calls.: 2;executor_consumes_platform_bridge.*contract_ready;diagnostic_success_result_mapping.*contract_ready;ready_call_execution_results.*contract_ready;tracked_open_result_preservation.*contract_ready;concrete_hwnd_and_d3d_device_execution.*tracked_open;draw_execution_backend.*tracked_open;present_execution.*tracked_open;frame_capture_artifact.*tracked_open;original_frame_oracle_parity.*tracked_open;real_d3d9_adapter.*tracked_open"
    )

    add_test(
        NAME yuengine_backend_device_adapter_contract
        COMMAND yuengine_cli backend-device-adapter samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_backend_device_adapter_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true backend_executor_ok=true device_adapter_runtime_ready=true executor_results_consumed_ready=true platform_device_preconditions_tracked=true downstream_execution_blocked_until_device=true backbuffer_extent_carried=true real_window_surface_gate_tracked=true real_d3d_interface_gate_tracked=true real_d3d_device_gate_tracked=true real_device_handle_ready=false resource_execution_requires_device=true draw_present_capture_requires_device=true adapter_record_count=10 consumed_executor_result_records=10 source_diagnostic_success_records=4 source_tracked_open_records=6 diagnostic_context_records=1 platform_device_adapter_records=3 window_surface_adapter_records=1 d3d_interface_adapter_records=1 create_device_adapter_records=1 downstream_blocked_records=6 resource_queue_blocked_records=3 render_queue_blocked_records=3 platform_device_precondition_calls=3 downstream_real_calls_blocked_until_device=685 blocked_real_calls_total=688 real_executed_calls=0 real_resource_creation_calls_blocked=46 real_upload_subresource_calls_blocked=458 real_state_binding_calls_blocked=57 real_draw_calls_blocked=121 real_present_calls_blocked=1 real_capture_oracle_calls_blocked=2 inherited_diagnostic_executed_calls=562 inherited_preserved_open_calls=127 linked_platform_input_records=110 ready_platform_input_records=57 tracked_open_platform_input_records=53 backbuffer_width=1280 backbuffer_height=720 resolved_device_adapter_contracts=4 tracked_device_adapter_obligations=6 open_device_adapter_obligations=6;adapter_summary.*adapter_record_count.: 10.*consumed_executor_result_records.: 10.*source_diagnostic_success_records.: 4.*source_tracked_open_records.: 6.*diagnostic_context_records.: 1.*platform_device_adapter_records.: 3.*downstream_blocked_records.: 6;device_preconditions.*window_surface_adapter_records.: 1.*d3d_interface_adapter_records.: 1.*create_device_adapter_records.: 1.*platform_device_precondition_calls.: 3.*real_device_handle_ready.: false.*backbuffer_width.: 1280.*backbuffer_height.: 720;blocked_execution.*resource_queue_blocked_records.: 3.*render_queue_blocked_records.: 3.*downstream_real_calls_blocked_until_device.: 685.*blocked_real_calls_total.: 688.*real_executed_calls.: 0.*real_resource_creation_calls_blocked.: 46.*real_upload_subresource_calls_blocked.: 458.*real_state_binding_calls_blocked.: 57.*real_draw_calls_blocked.: 121.*real_present_calls_blocked.: 1.*real_capture_oracle_calls_blocked.: 2;executor_link.*inherited_diagnostic_executed_calls.: 562.*inherited_preserved_open_calls.: 127.*linked_platform_input_records.: 110.*ready_platform_input_records.: 57.*tracked_open_platform_input_records.: 53;diagnostic_backend_bridge_adapter.*diagnostic_context.*contract_ready.*real_executed_calls.: 0;platform_window_surface_adapter.*window_surface.*tracked_open.*blocked_real_calls.: 1;d3d9_interface_creation_adapter.*d3d9_interface.*tracked_open.*blocked_real_calls.: 1;d3d9_device_creation_adapter.*d3d9_device.*tracked_open.*blocked_real_calls.: 1;d3d9_resource_creation_queue_adapter.*downstream_resource_queue.*blocked_until_device.*blocked_real_calls.: 46;d3d9_texture_upload_queue_adapter.*downstream_resource_queue.*blocked_until_device.*blocked_real_calls.: 458;d3d9_state_binding_queue_adapter.*downstream_resource_queue.*blocked_until_device.*blocked_real_calls.: 57;d3d9_draw_submission_queue_adapter.*downstream_render_queue.*blocked_until_device.*blocked_real_calls.: 121;d3d9_present_call_adapter.*downstream_render_queue.*blocked_until_device.*blocked_real_calls.: 1;frame_capture_oracle_queue_adapter.*downstream_render_queue.*blocked_until_device.*blocked_real_calls.: 2;device_adapter_consumes_executor_results.*contract_ready;platform_device_preconditions_tracked.*contract_ready;downstream_execution_blocked_until_device.*contract_ready;backbuffer_extent_carried_to_adapter.*contract_ready;real_hwnd_surface_creation.*tracked_open;real_direct3d9_interface_creation.*tracked_open;real_d3d9_device_creation.*tracked_open;real_resource_execution_after_device.*tracked_open;real_draw_present_capture_after_device.*tracked_open;original_frame_oracle_after_capture.*tracked_open"
    )

    add_test(
        NAME yuengine_backend_device_creation_contract
        COMMAND yuengine_cli backend-device-create samples/touhou_new_world/project.json --repo-root ${CMAKE_SOURCE_DIR}
    )
    set_tests_properties(yuengine_backend_device_creation_contract PROPERTIES
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        PASS_REGULAR_EXPRESSION "ok=true device_adapter_ok=true device_creation_runtime_ready=true adapter_preconditions_consumed_ready=true platform_execution_attempted=true window_surface_result_recorded=true d3d_interface_result_recorded=true d3d_device_result_recorded=true downstream_queues_preserved=true backbuffer_extent_carried=true platform_supported=true real_window_surface_ready=(true|false) real_d3d_interface_ready=(true|false) real_device_handle_ready=(true|false) execution_result_records=3 adapter_record_count=10 adapter_precondition_records_consumed=3 source_downstream_blocked_records=6 downstream_real_calls_deferred=685 window_surface_attempts=[0-9]+ d3d_interface_attempts=[0-9]+ d3d_device_attempts=[0-9]+ real_success_records=[0-9]+ real_failed_records=[0-9]+ blocked_by_dependency_records=[0-9]+ linked_platform_input_records=110 ready_platform_input_records=57 tracked_open_platform_input_records=53 backbuffer_width=1280 backbuffer_height=720 resolved_device_creation_contracts=5 tracked_device_creation_obligations=5 open_device_creation_obligations=5;execution_summary.*execution_result_records.: 3.*adapter_precondition_records_consumed.: 3.*platform_execution_attempted.: true.*platform_supported.: true;handle_summary.*real_window_surface_ready.: (true|false).*real_d3d_interface_ready.: (true|false).*real_device_handle_ready.: (true|false).*backbuffer_width.: 1280.*backbuffer_height.: 720;downstream_state.*source_downstream_blocked_records.: 6.*downstream_real_calls_deferred.: 685.*linked_platform_input_records.: 110.*ready_platform_input_records.: 57.*tracked_open_platform_input_records.: 53;real_hwnd_surface_execution.*source_adapter_record.: .platform_window_surface_adapter.*status.: .(real_success|real_failed|unsupported_platform);real_direct3d9_interface_execution.*source_adapter_record.: .d3d9_interface_creation_adapter.*status.: .(real_success|real_failed|unsupported_platform);real_d3d9_device_execution.*source_adapter_record.: .d3d9_device_creation_adapter.*status.: .(real_success|real_failed|blocked_by_dependency|unsupported_platform);device_creation_consumes_adapter_preconditions.*contract_ready;window_surface_execution_result.*contract_ready;direct3d9_interface_execution_result.*contract_ready;d3d9_device_creation_execution_result.*contract_ready;downstream_queues_preserved_after_device_attempt.*contract_ready;persistent_device_service_handle.*tracked_open;real_resource_execution_after_device.*tracked_open;real_draw_present_capture_after_device.*tracked_open;frame_capture_oracle_after_present.*tracked_open;original_frame_oracle_after_capture.*tracked_open"
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
