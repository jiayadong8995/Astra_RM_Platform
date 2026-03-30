# Architecture

**Analysis Date:** 2026-03-30

## Pattern Overview

**Overall:** Layered embedded runtime with a project-configured build surface and a Python SITL orchestration sidecar.

**Key Characteristics:**
- `robot_platform/CMakeLists.txt` builds the same runtime stack against two backend profiles: STM32 hardware and Linux SITL.
- `robot_platform/runtime/` is split into `generated -> bsp -> device -> control -> app`, with reusable primitives in `robot_platform/runtime/module/`.
- `robot_platform/sim/` mirrors the runtime boundary in Python by describing project profiles, launching a SITL bridge, and validating smoke-session outputs.

## Layers

**Build and Project Configuration:**
- Purpose: Select board/project metadata and compose either hardware or SITL binaries.
- Location: `robot_platform/CMakeLists.txt`, `robot_platform/projects/balance_chassis/project.yaml`, `robot_platform/projects/balance_chassis/board.yaml`
- Contains: Target selection flags, source lists, include wiring, project metadata, board metadata.
- Depends on: `robot_platform/runtime/`, `robot_platform/third_party/`, generated CubeMX output in `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/`
- Used by: `python3 -m robot_platform.tools.platform_cli.main build`, `python3 -m robot_platform.tools.platform_cli.main sim`

**Generated and BSP Layer:**
- Purpose: Provide hardware registers/HAL startup on STM32 and stubbed low-level services for Linux SITL.
- Location: `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/`, `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/`, `robot_platform/runtime/bsp/sitl/`
- Contains: CubeMX output, board CAN/UART/PWM/DWT wrappers, SITL `main`, HAL stubs, socket-backed bus shims.
- Depends on: STM32 HAL/CMSIS/FreeRTOS sources referenced in `robot_platform/CMakeLists.txt`
- Used by: `balance_chassis_hw_seed.elf`, `balance_chassis_sitl`, device concrete adapters in `robot_platform/runtime/device/`

**Device Semantics Layer:**
- Purpose: Normalize IMU, remote, and motor access behind stable runtime-facing interfaces.
- Location: `robot_platform/runtime/device/`
- Contains: `device_layer`, backend profiles, IMU/remote/motor semantic interfaces, backend-specific concrete adapters.
- Depends on: BSP handles and driver assets from `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/` or `robot_platform/runtime/bsp/sitl/`
- Used by: State estimation in `robot_platform/runtime/control/state/`, execution in `robot_platform/runtime/control/execution/`, remote intent in `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c`

**Control Pipeline Layer:**
- Purpose: Transform device inputs plus operator intent into robot state and actuator commands.
- Location: `robot_platform/runtime/control/`
- Contains: Runtime contracts, INS/chassis observation, balance controller, actuator gateway, task entrypoints.
- Depends on: `robot_platform/runtime/device/` for input/output and `robot_platform/runtime/module/` for math, filtering, and pub-sub.
- Used by: FreeRTOS app bring-up in `robot_platform/runtime/app/balance_chassis/app_bringup/`

**Application Composition Layer:**
- Purpose: Define task topology, project-specific startup, and intent translation for `balance_chassis`.
- Location: `robot_platform/runtime/app/balance_chassis/`
- Contains: FreeRTOS bootstrap, task registry, remote intent builder, app topic buses, app params/config.
- Depends on: `robot_platform/runtime/control/`, `robot_platform/runtime/device/`, `robot_platform/runtime/module/message_center/`
- Used by: Hardware startup through `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c` and SITL startup through `robot_platform/runtime/bsp/sitl/main_sitl.c`

**Reusable Module Layer:**
- Purpose: Hold cross-robot algorithms, control helpers, and the in-process message bus.
- Location: `robot_platform/runtime/module/`
- Contains: EKF, PID, VMC, kalman, mahony, generic controller helpers, `control_math`, `message_center`.
- Depends on: C runtime and CMSIS DSP includes configured in `robot_platform/CMakeLists.txt`
- Used by: Controllers and state estimators throughout `robot_platform/runtime/control/`

**Simulation Orchestration Layer:**
- Purpose: Describe runtime boundaries, spawn SITL sessions, bridge UDP traffic, and generate smoke reports.
- Location: `robot_platform/sim/`, `robot_platform/tools/platform_cli/`
- Contains: Profiles, runner, backend contract, UDP bridge, report writer, per-project simulation adapter.
- Depends on: Built SITL binary from `robot_platform/CMakeLists.txt` and project descriptors in `robot_platform/sim/projects/`
- Used by: `robot_platform/tools/platform_cli/main.py`

## Data Flow

**Runtime Control Loop:**

1. `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c` starts the scheduler-facing app and delegates task creation to `balance_chassis_start_tasks()` in `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c`.
2. `robot_platform/runtime/control/state/ins_task.c` reads IMU samples through `platform_device_read_default_imu()` from `robot_platform/runtime/device/device_layer.c`, updates the estimator, and publishes `ins_data` through `robot_platform/runtime/control/state/ins_topics.c`.
3. `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` reads remote input through `platform_device_read_default_remote()`, pulls `robot_state`, builds `robot_intent`, and publishes it via `robot_platform/runtime/app/balance_chassis/app_io/remote_topics.c`.
4. `robot_platform/runtime/control/state/observe_task.c` consumes `robot_intent` and `device_feedback`, produces `chassis_observe`, and publishes it through `robot_platform/runtime/control/state/observe_topics.c`.
5. `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c` waits for `ins_data` and `device_feedback`, pulls `robot_intent` and `chassis_observe`, runs `platform_balance_controller_step()` from `robot_platform/runtime/control/controllers/balance_controller.c`, then publishes `robot_state` and `actuator_command`.
6. `robot_platform/runtime/control/execution/motor_control_task.c` pulls `actuator_command`, captures feedback through `platform_actuator_gateway_capture_feedback()` in `robot_platform/runtime/control/execution/actuator_gateway.c`, republishes `device_feedback`, and dispatches the mapped `platform_device_command_t` back into `robot_platform/runtime/device/device_layer.c`.

**Message Transport Inside Runtime:**

1. Topic publishers and subscribers are registered against string topic names in `robot_platform/runtime/module/message_center/message_center.c`.
2. Every bus helper such as `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c` or `robot_platform/runtime/control/execution/actuator_topics.c` wraps raw pub-sub names in typed accessors.
3. Delivery is overwrite-only and latest-value-wins via `PubPushMessage()` and `SubGetMessage()` in `robot_platform/runtime/module/message_center/message_center.c`.

**Device Backend Selection:**

1. `robot_platform/runtime/device/device_layer.c` resolves a default backend profile using compile-time markers `PLATFORM_DEVICE_BACKEND_HW` or `PLATFORM_DEVICE_BACKEND_SITL`.
2. `robot_platform/runtime/device/device_profile_hw.c` binds concrete hardware implementations with STM32 handles like `hspi2`, `hfdcan1`, and `hfdcan2`.
3. `robot_platform/runtime/device/device_profile_sitl.c` binds the same semantic device nodes to SITL-capable adapters with null config, relying on SITL-specific implementations under `robot_platform/runtime/device/*_sitl.c`.

**Simulation Session Flow:**

1. `robot_platform/tools/platform_cli/main.py` parses the `sim` command, optionally builds `balance_chassis_sitl`, and resolves the project smoke runner from `robot_platform/sim/projects/__init__.py`.
2. `robot_platform/sim/projects/balance_chassis/smoke.py` passes `BALANCE_CHASSIS_PROFILE` from `robot_platform/sim/projects/balance_chassis/profile.py` into `run_profile_session()` in `robot_platform/sim/core/runner.py`.
3. `robot_platform/sim/backends/sitl_bridge.py` loads a `SitlBackendAdapter` implementation from `robot_platform/sim/projects/balance_chassis/bridge_adapter.py`, emits runtime-boundary metadata, and runs UDP IMU/motor/stats threads.
4. `robot_platform/sim/core/runner.py` captures bridge output, compares observed runtime boundary and transport ports against the declared profile, summarizes validation targets, and writes a report through `robot_platform/sim/reports/report_writer.py`.

**State Management:**
- Runtime state is mostly task-local structs such as `platform_balance_controller_t` in `robot_platform/runtime/control/controllers/balance_controller.c` and `platform_ins_state_estimator_t` in `robot_platform/runtime/control/state/ins_task.c`.
- Cross-task state is transported through named message-center topics rather than shared global state.
- Backend selection state is stored once inside `g_platform_default_layer` and `g_platform_default_profile` in `robot_platform/runtime/device/device_layer.c`.
- Simulation state is immutable configuration in dataclasses from `robot_platform/sim/core/profile.py` plus per-session summaries built in `robot_platform/sim/core/runner.py`.

## Key Abstractions

**Device Layer:**
- Purpose: Aggregate semantic device endpoints and hide backend-specific driver binding.
- Examples: `robot_platform/runtime/device/device_layer.h`, `robot_platform/runtime/device/device_profile_hw.c`, `robot_platform/runtime/device/device_profile_sitl.c`
- Pattern: One semantic aggregate struct with profile-based binding of IMU, remote, and motor nodes.

**Runtime Contracts:**
- Purpose: Define formal data shapes between device, control, and app layers.
- Examples: `robot_platform/runtime/control/contracts/device_input.h`, `robot_platform/runtime/control/contracts/robot_state.h`, `robot_platform/runtime/control/contracts/actuator_command.h`, `robot_platform/runtime/control/contracts/runtime_contracts.h`
- Pattern: Header-only contract boundary shared by app, controller, device, and execution code.

**Typed Topic Buses:**
- Purpose: Prevent raw string topic usage from leaking across most business logic.
- Examples: `robot_platform/runtime/control/state/ins_topics.c`, `robot_platform/runtime/control/state/observe_topics.c`, `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/control/execution/actuator_topics.c`
- Pattern: Small bus wrappers around `PubRegister`, `SubRegister`, `PubPushMessage`, and `SubGetMessage`.

**Project Simulation Profile:**
- Purpose: Declare one project’s SITL target, bridge module, runtime topic boundaries, ports, and validation targets.
- Examples: `robot_platform/sim/core/profile.py`, `robot_platform/sim/projects/balance_chassis/profile.py`
- Pattern: Frozen dataclass configuration injected into generic runner and bridge code.

## Entry Points

**Hardware Build Entry:**
- Location: `robot_platform/CMakeLists.txt`
- Triggers: `python3 -m robot_platform.tools.platform_cli.main build hw_elf`
- Responsibilities: Assemble STM32 HAL, generated CubeMX sources, board BSP, FreeRTOS, app, device, and control layers into `balance_chassis_hw_seed.elf`.

**SITL Binary Entry:**
- Location: `robot_platform/runtime/bsp/sitl/main_sitl.c`
- Triggers: Executing `balance_chassis_sitl`
- Responsibilities: Start logging, call `MX_FREERTOS_Init()`, and hand execution to the FreeRTOS POSIX scheduler.

**App Bootstrap Entry:**
- Location: `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c`
- Triggers: Called from hardware-generated `freertos.c` or SITL `main_sitl.c`
- Responsibilities: Create the idle/default task and register project runtime tasks.

**CLI Entry:**
- Location: `robot_platform/tools/platform_cli/main.py`
- Triggers: `python3 -m robot_platform.tools.platform_cli.main ...`
- Responsibilities: Route generate/build/sim/test commands and keep build/run conventions in one place.

**Simulation Backend Entry:**
- Location: `robot_platform/sim/backends/sitl_bridge.py`
- Triggers: Spawned by the Python smoke runner using the profile’s `backend_module`
- Responsibilities: Open UDP ports, emit bridge metadata, synthesize IMU/motor traffic, and report stats/runtime-output observations.

## Error Handling

**Strategy:** Fail-soft at runtime task boundaries, fail-fast in build/sim orchestration.

**Patterns:**
- Device functions in `robot_platform/runtime/device/device_layer.c` return `platform_device_result_t`, with `UNAVAILABLE` treated as non-fatal for partial reads inside `platform_device_layer_read_input()`.
- Runtime tasks such as `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` and `robot_platform/runtime/control/execution/motor_control_task.c` mostly ignore non-critical device return codes and keep the scheduler alive.
- Startup synchronization uses busy-wait readiness loops in `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/control/state/observe_topics.c`, and `robot_platform/runtime/control/execution/actuator_topics.c`.
- Python orchestration returns process exit codes and serializes failures into smoke-report summaries in `robot_platform/sim/core/runner.py`.
- The bridge emits explicit `startup_error` events in `robot_platform/sim/backends/sitl_bridge.py` when adapter loading or socket binding fails.

## Cross-Cutting Concerns

**Logging:** Runtime logging is mostly `printf` in `robot_platform/runtime/bsp/sitl/main_sitl.c`. Simulation logging is structured around stdout lines and JSON-ish bridge events from `robot_platform/sim/backends/sitl_bridge.py`.

**Validation:** Runtime readiness is validated by waiting on topic state (`ins_data.ready`, `device_feedback.actuator_feedback.valid`). Simulation validation is declared per project in `robot_platform/sim/projects/balance_chassis/profile.py` and summarized in `robot_platform/sim/projects/balance_chassis/validation.py`.

**Authentication:** Not applicable. No auth layer is present in first-party runtime or simulation code.

**Configuration:** Project and board identity live in `robot_platform/projects/balance_chassis/project.yaml` and `robot_platform/projects/balance_chassis/board.yaml`; build-mode selection lives in `robot_platform/CMakeLists.txt`; sim profile configuration lives in `robot_platform/sim/projects/balance_chassis/profile.py`.

---

*Architecture analysis: 2026-03-30*
