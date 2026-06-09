# L17 Frame Scheduler Status

L17 creates a service-owned frame scheduler/update graph on top of the L15 gameplay-frame contract
and L16 renderer submission contract. It is still a runtime contract, not a live OS window or GPU
device backend.

## Contract

The runtime command is:

```text
project.json
-> L15 gameplay-frame inputs
-> L16 renderer backend submission
-> service-owned frame scheduler nodes
-> update graph readiness
```

The scheduler graph contains 10 service-owned nodes:

- project lifecycle;
- script tick/render;
- save/profile update;
- scene/resource update;
- actor/task update;
- camera update;
- event/tutorial update;
- audio update;
- renderer submission;
- backend obligation tracking.

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe frame-scheduler samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_frame_scheduler_update_graph_contract --output-on-failure
```

Current metric:

```text
ok=true
gameplay_frame_ok=true
renderer_submission_ok=true
update_graph_ready=true
frame_index=0
node_count=10
executed_nodes=10
service_node_count=10
scheduler_edges=12
gameplay_commands=221
renderer_backend_commands=181
scheduled_work_items=469
backend_obligations=6
unresolved_nodes=0
```

Node command counts:

```text
project_lifecycle=1
script_tick_render=55
save_profile_update=6
scene_resource_update=166
actor_task_update=7
camera_update=2
event_tutorial_update=17
audio_update=28
renderer_submission=181
backend_obligation_tracking=6
```

## Boundary

L17 is not a full game loop. It does not yet provide:

- fixed/variable timestep scheduling;
- real input polling;
- real GPU backend;
- animation/skinning update;
- save serialization update;
- multi-scene streaming;
- original-frame oracle parity.

It does replace the previous ad hoc report composition with one runtime-owned frame graph. Future
subsystems should attach to scheduler nodes instead of creating standalone diagnostics.

## Next Edges

- L18 has since resolved texture upload format and material binding contracts while keeping
  shader/effect, font, device/swapchain, and oracle parity as tracked open obligations.
- L19 has since materialized shader/effect tracking and material semantics behind the 16 material
  tags and 111 mesh submissions.
- L20 has since defined the device/swapchain and render-state presentation contract that consumes
  the scheduler, renderer submission, backend obligations, and material semantics.
- L21: split texture upload records, sampler/blend/depth state, font atlas/glyph metrics, and
  original-frame oracle parity into runtime gates.
