# Codebase Concerns

**Analysis Date:** 2026-03-30

## Tech Debt

**Message bus payload sizing is unsafe:**
- Issue: `message_center` stores every topic payload in a fixed `uint8_t data_buf[64]`, but it is used for large contracts such as `platform_robot_state_t`, `platform_actuator_command_t`, and `platform_device_feedback_t`.
- Files: `robot_platform/runtime/module/message_center/message_center.h`, `robot_platform/runtime/module/message_center/message_center.c`, `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/app/balance_chassis/app_io/remote_topics.c`, `robot_platform/runtime/control/execution/actuator_topics.c`, `robot_platform/runtime/control/contracts/robot_state.h`, `robot_platform/runtime/control/contracts/actuator_command.h`, `robot_platform/runtime/control/contracts/device_feedback.h`, `robot_platform/runtime/control/contracts/device_input.h`
- Impact: Publishing large structs can overrun subscriber buffers and corrupt adjacent state, which can manifest as unstable control behavior, random task faults, or hard-to-reproduce SITL/hardware divergence.
- Fix approach: Replace the fixed-size payload buffer with per-topic sizing or explicit compile-time size checks; reject topic registration when `data_len > MSG_MAX_DATA_LEN`; add tests that assert every published contract fits.

**Legacy and platformized control paths are partially fused:**
- Issue: `robot_platform` still boots legacy app tasks and BSP code while routing core I/O through the newer `runtime/device`, `runtime/control`, and contract layers.
- Files: `robot_platform/CMakeLists.txt`, `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c`, `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c`, `robot_platform/runtime/control/README.md`, `robot_platform/runtime/device/README.md`
- Impact: Ownership boundaries are unclear, behavior depends on cross-layer assumptions, and regressions are easy when changing either the old app code or the new device/control abstractions.
- Fix approach: Finish the migration by removing duplicate responsibilities, documenting the single authoritative control path, and shrinking the legacy app layer to orchestration only.

**Generated-code dependency is operationally fragile:**
- Issue: the build depends on `runtime/generated/stm32h7_ctrl_board_raw`, which is regenerated from `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` through an external CubeMX CLI flow.
- Files: `robot_platform/CMakeLists.txt`, `robot_platform/tools/platform_cli/main.py`, `robot_platform/tools/cubemx_backend/main.py`, `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
- Impact: runtime sources can drift from the checked-in IOC, reproducibility depends on a local proprietary toolchain, and failures show up late during build or hardware bring-up.
- Fix approach: add a generated-artifact freshness check, pin the supported CubeMX path/version in automation, and document regeneration as a mandatory step whenever the IOC changes.

## Known Bugs

**SITL device profile is wired to stubs instead of the UDP-backed BSP adapters:**
- Symptoms: the control pipeline reads invalid IMU/remote data and reports motor feedback unavailable even though the SITL build also compiles UDP bridge drivers.
- Files: `robot_platform/runtime/device/device_profile_sitl.c`, `robot_platform/runtime/device/imu/bmi088_device_sitl.c`, `robot_platform/runtime/device/remote/dbus_remote_device_sitl.c`, `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c`, `robot_platform/runtime/bsp/sitl/BMI088driver_sitl.c`, `robot_platform/runtime/bsp/sitl/dm4310_drv_sitl.c`, `robot_platform/CMakeLists.txt`
- Trigger: run `python3 -m robot_platform.tools.platform_cli.main sim` or boot `balance_chassis_sitl`.
- Workaround: None inside the current runtime path; the SITL profile must be rebound to real SITL adapters or the control loop will stay on unavailable/stubbed data.

**Declared sim outputs can never be observed by validation:**
- Symptoms: the smoke report can only mark validation targets as `declared_only` because the adapter returns no runtime observations, while the declared output topics do not match the actual published topics.
- Files: `robot_platform/sim/projects/balance_chassis/profile.py`, `robot_platform/sim/projects/balance_chassis/bridge_adapter.py`, `robot_platform/sim/projects/balance_chassis/validation.py`, `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`
- Trigger: any SITL smoke run that relies on `chassis_state`, `leg_left`, and `leg_right` validation targets.
- Workaround: None in-tree; validation requires either topic renaming to match published topics or an adapter that translates `robot_state` into the declared outputs and emits observations.

**Remote input error handling mutates a global singleton and ignores the caller buffer:**
- Symptoms: `sbus_to_rc()` decodes into the caller-provided struct but `RC_data_is_error()` validates and zeroes the global `rc_ctrl`, not the passed-in `rc_ctrl_local`.
- Files: `robot_platform/runtime/device/remote/dbus/remote_control.c`
- Trigger: decode any invalid or partially corrupted SBUS frame through `sbus_to_rc()`.
- Workaround: Use the global singleton only and avoid alternate decode buffers; the safer fix is to validate the same struct that was decoded.

## Security Considerations

**Unsafe fixed-size memcpy on control messages:**
- Risk: oversized topic payloads are copied with `memcpy()` into 64-byte subscriber buffers without bounds checks.
- Files: `robot_platform/runtime/module/message_center/message_center.h`, `robot_platform/runtime/module/message_center/message_center.c`
- Current mitigation: None beyond a comment that the buffer "must >= sizeof(largest topic struct)".
- Recommendations: add explicit bounds checks, static assertions for every registered topic, and fail-fast registration instead of silent overflow.

**Unauthenticated local UDP control surface in SITL:**
- Risk: any local process can inject IMU packets or motor commands/feedback on ports `9001`-`9003`, affecting smoke runs and potentially masking control defects.
- Files: `robot_platform/runtime/bsp/sitl/BMI088driver_sitl.c`, `robot_platform/runtime/bsp/sitl/dm4310_drv_sitl.c`, `robot_platform/sim/backends/sitl_bridge.py`, `robot_platform/sim/projects/balance_chassis/profile.py`
- Current mitigation: sockets bind only to `127.0.0.1`.
- Recommendations: add a session token or protocol header check, reject malformed packet sizes and source addresses explicitly, and surface dropped-packet counters in smoke reports.

## Performance Bottlenecks

**Topic registration walks linked lists and fixed pools linearly:**
- Problem: topic lookup and subscriber append are O(topics + subscribers) and happen against very small static limits.
- Files: `robot_platform/runtime/module/message_center/message_center.c`, `robot_platform/runtime/module/message_center/message_center.h`
- Cause: the bus is optimized for bootstrap simplicity rather than deterministic scaling.
- Improvement path: predeclare topic IDs or use indexed topic tables; add instrumentation for pool saturation and publish latency before increasing task count.

**SITL bridge spins at high frequency without backpressure:**
- Problem: the IMU thread sends packets every 1 ms and the motor thread loops forever on blocking `recvfrom()` with no shutdown coordination or rate controls.
- Files: `robot_platform/sim/backends/sitl_bridge.py`
- Cause: the bridge is a minimal smoke harness, not a paced simulation backend.
- Improvement path: add explicit tick scheduling, thread stop signals, and a bounded transport contract that can report overruns or late consumers.

## Fragile Areas

**Motor command dispatch drops control semantics:**
- Files: `robot_platform/runtime/control/execution/actuator_gateway.c`, `robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c`
- Why fragile: actuator commands are flattened into device commands, but hardware write always sends MIT torque commands with zero position/velocity targets and ignores per-motor control mode semantics beyond wheel current.
- Safe modification: change the contract mapping and hardware backend together, and verify each joint/wheel mode path end-to-end on hardware or a realistic adapter.
- Test coverage: No C tests cover `platform_map_contract_command()` or `platform_motor_actuator_write()`.

**Task startup depends on busy-wait readiness loops with stale cached messages:**
- Files: `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/control/state/observe_topics.c`, `robot_platform/runtime/control/execution/actuator_topics.c`
- Why fragile: readiness is inferred from message contents, tasks poll with `osDelay(1)`, and message delivery is overwrite-only, so startup behavior depends on task ordering and whether a "good" message arrives before loops block.
- Safe modification: introduce explicit initialization state/events rather than implicit topic readiness flags.
- Test coverage: No tests exercise task startup ordering or missed-message behavior.

**Legacy gimbal tree contains placeholder instructions and mojibake:**
- Files: `Astra_RM2025_Balance/Gimbal/application/gimbal_behaviour.c`, `Astra_RM2025_Balance/Gimbal/application/gimbal_behaviour.h`
- Why fragile: the files still embed `GIMBAL_XXX_XXX` scaffolding text and corrupted non-ASCII comments, which makes behavior extension error-prone and obscures whether the tree is production-maintained or archival.
- Safe modification: treat the tree as legacy until ownership is explicit; clean encoding and remove template instructions before further feature work.
- Test coverage: Not detected.

## Scaling Limits

**Message center capacity is hard capped at bootstrap values:**
- Current capacity: `MSG_MAX_TOPICS = 8`, `MSG_MAX_SUBSCRIBERS = 16`, `MSG_MAX_TOPIC_NAME = 24`, `MSG_MAX_DATA_LEN = 64`.
- Limit: adding more runtime topics, larger contracts, or more subscribers will fail or corrupt memory rather than degrade gracefully.
- Scaling path: move capacities to validated configuration, add compile-time and runtime saturation checks, and size payload storage per topic.

## Dependencies at Risk

**STM32CubeMX CLI is an external, host-local build dependency:**
- Risk: code generation depends on a locally installed binary and Java environment discovered from host-specific paths.
- Impact: onboarding and CI portability remain fragile, and codegen can fail for environment reasons unrelated to the source tree.
- Migration plan: encapsulate codegen in a reproducible tool container or provide a pinned installer/bootstrap step that can be validated automatically.

## Missing Critical Features

**No end-to-end verification for the C control pipeline:**
- Problem: current tests cover only Python CLI parsing and SITL smoke-report summarization, not the C runtime tasks, controller outputs, topic transport, or hardware abstraction logic.
- Blocks: safe refactoring of `runtime/control`, `runtime/device`, and task wiring.

**No real runtime-output exporter for sim validation:**
- Problem: the sim profile declares output boundaries and required validation targets, but `collect_runtime_output_observations()` returns an empty tuple.
- Blocks: proving that published control outputs match the declared runtime contract during smoke runs.

## Test Coverage Gaps

**Runtime C modules are effectively untested:**
- What's not tested: `message_center`, `device_layer`, `actuator_gateway`, `balance_controller`, `observe_task`, `INS_task`, and the app task wiring.
- Files: `robot_platform/runtime/module/message_center/message_center.c`, `robot_platform/runtime/device/device_layer.c`, `robot_platform/runtime/control/execution/actuator_gateway.c`, `robot_platform/runtime/control/controllers/balance_controller.c`, `robot_platform/runtime/control/state/observe_task.c`, `robot_platform/runtime/control/state/ins_task.c`
- Risk: regressions in control math, message sizing, backend binding, or startup sequencing will only surface in manual SITL/hardware runs.
- Priority: High

**Smoke tests validate report plumbing more than actual robot behavior:**
- What's not tested: real runtime outputs, command-to-feedback closed loop, task timing, and topic-name consistency between the runtime and sim declarations.
- Files: `robot_platform/sim/tests/test_runner.py`, `robot_platform/tools/platform_cli/tests/test_main.py`, `robot_platform/sim/projects/balance_chassis/profile.py`, `robot_platform/sim/projects/balance_chassis/bridge_adapter.py`
- Risk: CI can stay green while the SITL control loop is disconnected or semantically wrong.
- Priority: High

---

*Concerns audit: 2026-03-30*
