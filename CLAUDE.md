<!-- GSD:project-start source:PROJECT.md -->
## Project

**Astra RM Robot Platform**

This project is a Robotmaster-oriented embedded robot platform, with the current platform direction centered on a wheeled-legged robot (`balance_chassis`) as the first real integration target. It aims to unify firmware generation, cross-target builds, host-side verification, and controlled bring-up so the team can evolve robot control software with confidence instead of relying on risky first-on-robot debugging.

The immediate project focus is not full competition readiness. The roadmap is centered on a safe v1 foundation: host-side TDD, fake-link validation, and a reliable build-to-firmware pipeline, while the longer-term project goal points at a constrained real-robot bring-up for the wheeled-legged minimum closed loop.

**Core Value:** Make wheeled-legged Robotmaster control software safe to evolve by catching dangerous control and data-link errors before the robot ever gets a chance to go unstable on hardware.

### Constraints

- **Platform direction**: Build a reusable Robotmaster robot platform, not a one-off robot app — this is a deliberate project choice even though it increases design pressure early
- **Safety**: On-robot testing must be gated by pre-hardware verification — avoiding uncontrolled robot behavior is more important than fast manual bring-up
- **Validation model**: Host-side tests and fake links must catch logic and contract faults, but cannot be treated as final physical proof — the roadmap must preserve staged progression from fake data to constrained hardware validation
- **Architecture**: Existing runtime layering should be preserved where it provides real leverage, but implementation-level coupling and overdesign need active review — current complexity is a project risk
- **Build environment**: Firmware generation currently depends on STM32CubeMX, cross-compilers, and local host setup — the platform must work within that toolchain reality while making the loop more reliable
- **Current target**: `balance_chassis` is the first concrete robot profile — the roadmap should use it as the proving ground without collapsing the entire platform abstraction into this single target
<!-- GSD:project-end -->

<!-- GSD:stack-start source:codebase/STACK.md -->
## Technology Stack

## Languages
- C (C11 in toolchain) - Main runtime, hardware firmware, BSP, device, control, and SITL executable code under `robot_platform/runtime/` and `robot_platform/CMakeLists.txt`
- Python (version not pinned in repo) - Platform CLI, STM32CubeMX backend, SITL orchestration, smoke reporting, and tests under `robot_platform/tools/` and `robot_platform/sim/`
- CMake (minimum 3.22) - Top-level build definition in `robot_platform/CMakeLists.txt`
- YAML - Declarative board and project configuration in `robot_platform/projects/balance_chassis/project.yaml`, `robot_platform/projects/balance_chassis/board.yaml`, `robot_platform/tools/schema/project.schema.yaml`, and `robot_platform/tools/schema/board.schema.yaml`
- ASM - STM32 startup code compiled into the hardware ELF from `robot_platform/CMakeLists.txt`
- IOC project format - STM32CubeMX source input in `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`
## Runtime
- Embedded ARM Cortex-M7 firmware on STM32H723 hardware, configured in `robot_platform/projects/balance_chassis/board.yaml`
- Linux x86_64 SITL runtime, configured by `robot_platform/cmake/toolchains/linux-gcc.cmake`
- Python CLI execution environment for tooling and tests, invoked from `robot_platform/README.md` and implemented in `robot_platform/tools/platform_cli/main.py`
- None detected for the active project surface under `robot_platform/`
- Lockfile: missing for the active project surface
## Frameworks
- STM32 HAL (vendored) - MCU peripheral access via `robot_platform/third_party/stm32_cube/Drivers/STM32H7xx_HAL_Driver`
- CMSIS and CMSIS-DSP (vendored) - Core and DSP support via `robot_platform/third_party/stm32_cube/Drivers/CMSIS`
- FreeRTOS (vendored) - Tasking and RTOS primitives via `robot_platform/third_party/stm32_cube/Middlewares/Third_Party/FreeRTOS/Source`
- Python `unittest` - Minimal regression coverage for CLI and sim runner in `robot_platform/tools/platform_cli/tests/test_main.py` and `robot_platform/sim/tests/test_runner.py`
- CMake 3.22+ - Primary build system in `robot_platform/CMakeLists.txt`
- Ninja - Hardware build generator selected in `robot_platform/tools/platform_cli/main.py`
- Unix Makefiles - SITL build generator selected in `robot_platform/tools/platform_cli/main.py`
- `arm-none-eabi-gcc` - Cross-compiler in `robot_platform/cmake/toolchains/arm-none-eabi-gcc.cmake`
- GCC/G++ - Linux SITL compiler in `robot_platform/cmake/toolchains/linux-gcc.cmake`
- STM32CubeMX 6.17.0 - Code generation backend discovered in `robot_platform/tools/cubemx_backend/main.py`
## Key Dependencies
- `robot_platform/third_party/stm32_cube` - Vendored STM32Cube assets: HAL, CMSIS, FreeRTOS, and generated-code dependencies used by `robot_platform/CMakeLists.txt`
- `robot_platform/third_party/freertos_port_gcc_cm7_r0p1` - Cortex-M7 FreeRTOS GCC port used by the hardware target in `robot_platform/CMakeLists.txt`
- `robot_platform/third_party/freertos_port_linux` - Linux FreeRTOS port used by the SITL target in `robot_platform/CMakeLists.txt`
- `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc` - Historical board definition consumed by the current `generate` flow in `robot_platform/tools/platform_cli/main.py`
- `robot_platform/tools/platform_cli/main.py` - Unified command entry point for `generate`, `build`, `sim`, and `test`
- `robot_platform/tools/cubemx_backend/main.py` - Wrapper around STM32CubeMX CLI, HOME override, and headless handling
- `robot_platform/sim/core/runner.py` - SITL session orchestration, subprocess launching, and smoke report generation
- `robot_platform/sim/reports/report_writer.py` - JSON report output to `build/sim_reports/`
## Configuration
- No `.env` files detected at repository root or within the active project scan
- `STM32CUBEMX_BIN` can override the STM32CubeMX binary path in `robot_platform/tools/cubemx_backend/main.py`
- `DISPLAY` and `WAYLAND_DISPLAY` are checked to decide headless CubeMX behavior in `robot_platform/tools/cubemx_backend/main.py`
- `HOME` and `JAVA_TOOL_OPTIONS` are set by the CubeMX backend to isolate runtime state in `.cache/stm32_user_home`
- Top-level build graph: `robot_platform/CMakeLists.txt`
- Toolchains: `robot_platform/cmake/toolchains/arm-none-eabi-gcc.cmake` and `robot_platform/cmake/toolchains/linux-gcc.cmake`
- Board/project descriptors: `robot_platform/projects/balance_chassis/board.yaml` and `robot_platform/projects/balance_chassis/project.yaml`
- Config schemas: `robot_platform/tools/schema/board.schema.yaml` and `robot_platform/tools/schema/project.schema.yaml`
- Historical generated baseline: `Astra_RM2025_Balance/Chassis/Core/` and `Astra_RM2025_Balance/Chassis/Drivers/`
## Platform Requirements
- `python3`, `cmake`, `ninja`, `arm-none-eabi-gcc`, `java`, and `STM32CubeMX` are required by `robot_platform/docs/wsl_environment_setup.md`
- Linux or WSL environment for current documented workflows in `robot_platform/docs/wsl_environment_setup.md`
- Writable repository-local cache directories such as `.cache/stm32_user_home` and generated output under `robot_platform/runtime/generated/`
- STM32H723 target board defined by `robot_platform/projects/balance_chassis/board.yaml`
- Cortex-M7 floating-point settings and linker script configured in `robot_platform/CMakeLists.txt` and `robot_platform/cmake/linker/stm32h723_flash.ld`
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->
## Conventions

## Naming Patterns
- Use `snake_case` for Python modules and C source/header files such as `robot_platform/sim/core/runner.py`, `robot_platform/tools/platform_cli/main.py`, `robot_platform/runtime/control/execution/motor_control_task.c`, and `robot_platform/runtime/control/controllers/balance_controller.h`.
- Use directory names to encode subsystem ownership, for example `robot_platform/runtime/control/controllers`, `robot_platform/runtime/device/imu`, and `robot_platform/sim/projects/balance_chassis`.
- Use `snake_case` for Python functions such as `run_sitl_session` in `robot_platform/sim/runner.py`, `write_report` in `robot_platform/sim/reports/report_writer.py`, and `_parse_sim_args` in `robot_platform/tools/platform_cli/main.py`.
- Prefix Python internal helpers with `_` when they are module-private, as in `robot_platform/sim/core/runner.py` and `robot_platform/tools/platform_cli/main.py`.
- Use `platform_` prefixes for reusable C APIs such as `platform_balance_controller_init` in `robot_platform/runtime/control/controllers/balance_controller.c`, `platform_float_clamp` in `robot_platform/runtime/module/lib/control/control_math.c`, and `platform_motor_actuator_device_bind` in `robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c`.
- Preserve legacy task and HAL callback names where framework integration requires them, for example `INS_task` in `robot_platform/runtime/control/state/ins_task.c`, `Chassis_task` via `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c`, and `HAL_UARTEx_RxEventCallback` in `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_uart.c`.
- Use descriptive `snake_case` for Python locals and arguments such as `duration_s`, `supported_projects`, and `runtime_output_observations` in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/core/runner.py`.
- C locals and fields are also predominantly `snake_case`, for example `runtime_bus`, `device_feedback`, `turn_torque`, and `leg_length_target` across `robot_platform/runtime/control/execution/motor_control_task.c` and `robot_platform/runtime/control/controllers/balance_controller.c`.
- Keep macro-style all-caps names for constants and task handles supplied by embedded frameworks, such as `INS_TASKHandle` in `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c` and `MSG_MAX_TOPICS` in `robot_platform/runtime/module/message_center/message_center.h`.
- Python uses `PascalCase` dataclasses and type aliases such as `SimProjectProfile`, `RuntimeTopicBoundary`, `TransportPorts`, and `ValidationStatus` in `robot_platform/sim/core/profile.py`.
- C uses `_t` suffixed typedefs for most project-owned structs, such as `platform_balance_controller_t` in `robot_platform/runtime/control/controllers/balance_controller.h` and `platform_motor_device_t` in `robot_platform/runtime/device/actuator/motor/motor_device.h`.
- Keep vendor or legacy types unchanged, for example `PidTypeDef` and `Subscriber_t` in `robot_platform/runtime/control/controllers/balance_controller.h` and `robot_platform/runtime/module/message_center/message_center.h`.
## Code Style
- No first-party formatter config was detected under `robot_platform/`; `.clang-format`, `.clang-tidy`, `.prettierrc`, `eslint.config.*`, and `pyproject.toml` are not present in the active project tree.
- C code uses K&R-style braces with function braces on the next line, as in `robot_platform/runtime/control/controllers/balance_controller.c` and `robot_platform/runtime/control/state/ins_task.c`.
- Indentation is not fully uniform. New code should match the surrounding file rather than forcing a global style shift. `robot_platform/runtime/control/controllers/balance_controller.c` uses mostly four-space indentation, while `robot_platform/runtime/control/state/ins_task.c` has inconsistent spacing.
- Python uses four-space indentation, type annotations, and compact module headers, as in `robot_platform/tools/platform_cli/main.py`, `robot_platform/sim/core/profile.py`, and `robot_platform/sim/reports/report_writer.py`.
- No dedicated lint configuration is detected for either C or Python in the first-party tree.
- The enforced quality gate is currently compiler warnings in `robot_platform/CMakeLists.txt`: `-Wall`, `-Wextra`, and `-Wno-unused-parameter` on C targets.
- Python quality relies on standard-library discipline and tests rather than a configured linter.
## Import Organization
- Python uses package-relative absolute imports rooted at `robot_platform`, for example `from robot_platform.sim.core.profile import SimProjectProfile` in `robot_platform/sim/core/runner.py`.
- C uses relative include paths within each subsystem, for example `../constraints/actuator_constraints.h` in `robot_platform/runtime/control/controllers/balance_controller.c` and `../../device/device_layer.h` in `robot_platform/runtime/control/state/ins_task.c`.
- Framework and generated includes are referenced by bare header name when injected via CMake include directories, such as `cmsis_os.h` and `bsp_dwt.h` in `robot_platform/runtime/control/state/ins_task.c`.
## Error Handling
- Python favors return-code based command orchestration and explicit early returns. `_run`, `_build_hw_seed`, `_build_sitl`, and `_run_tests` in `robot_platform/tools/platform_cli/main.py` return process exit codes instead of raising application-specific exceptions.
- Python uses `ValueError` for argument validation, as in `_parse_sim_args` in `robot_platform/tools/platform_cli/main.py`.
- Python summary/report code is defensive around partial data, using `isinstance` checks and silent skips for malformed payloads in `robot_platform/sim/core/runner.py`.
- C runtime code usually handles device failures by skipping an update and continuing the task loop, for example `platform_device_read_default_imu(...) == PLATFORM_DEVICE_RESULT_OK` in `robot_platform/runtime/control/state/ins_task.c` and `platform_actuator_gateway_capture_feedback(...) == PLATFORM_DEVICE_RESULT_OK` in `robot_platform/runtime/control/execution/motor_control_task.c`.
- Low-level BSP code often checks HAL return codes but does not surface rich diagnostics, as seen in `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/can_bsp.c`.
- Static-memory utilities return null pointers or sentinel values instead of asserting, for example `PubRegister` and `SubRegister` in `robot_platform/runtime/module/message_center/message_center.c` return `(void *)0` on pool exhaustion.
## Logging
- Python CLI and simulation tooling use `print(...)` for command echoing, JSON summaries, and simple terminal status output in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/backends/sitl_bridge.py`.
- SITL entrypoints use tagged `printf` lines such as `[SITL] ...` in `robot_platform/runtime/bsp/sitl/main_sitl.c`.
- There is no centralized logging abstraction in first-party code. New logs should follow the existing tagged plain-text style rather than introducing a second logging framework.
## Comments
- Comments are sparse in runtime control code and are mainly used around non-obvious framework behavior or memory constraints.
- Use brief explanatory comments around static-allocation assumptions and integration boundaries, following `robot_platform/runtime/module/message_center/message_center.h` and `robot_platform/runtime/module/message_center/message_center.c`.
- Avoid narrating straightforward assignments; most control and task code such as `robot_platform/runtime/control/controllers/balance_controller.c` and `robot_platform/runtime/control/execution/motor_control_task.c` is left uncommented.
- Not applicable.
- C uses Doxygen-style header comments selectively in reusable modules, for example `robot_platform/runtime/module/message_center/message_center.h`.
- Python generally does not use docstrings beyond the module-level summary in `robot_platform/tools/platform_cli/main.py`.
## Function Design
- Public entrypoints are small coordinators that delegate to helpers. Examples include `platform_balance_controller_step` in `robot_platform/runtime/control/controllers/balance_controller.c` and `main` in `robot_platform/tools/platform_cli/main.py`.
- Large data-processing modules keep many private helpers in one file rather than splitting aggressively, as in `robot_platform/sim/core/runner.py` and `robot_platform/runtime/control/controllers/balance_controller.c`.
- Python signatures are annotated and often keyword-oriented for externally used functions, such as `run_sitl_session(*, repo_root: Path, duration_s: float = 3.0)` in `robot_platform/sim/runner.py`.
- C APIs pass output/state through pointers and avoid heap allocation, as in `platform_balance_controller_build_outputs` in `robot_platform/runtime/control/controllers/balance_controller.c`.
- Python command functions return integer shell-style status codes in `robot_platform/tools/platform_cli/main.py`.
- Python reporting helpers usually mutate a provided summary dict in place and return `None`, as in `_summarize_bridge_stats` and `_build_smoke_result` in `robot_platform/sim/core/runner.py`.
- C task functions are `void` infinite loops, and device/helper functions return enums, integers, or booleans when a success/failure signal is needed.
## Module Design
- Python modules expose a small public surface and leave implementation helpers private with `_` prefixes, as in `robot_platform/sim/runner.py` re-exporting selected helpers from `robot_platform/sim/core/runner.py`.
- C headers declare the subsystem API and keep internal helpers `static` inside the `.c` file, as in `robot_platform/runtime/control/controllers/balance_controller.c`.
- Minimal Python package barrels are used where discovery matters. `robot_platform/sim/projects/__init__.py` centralizes project lookup functions.
- No C barrel-header pattern is present beyond subsystem headers and CMake include directory aggregation.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->
## Architecture

## Pattern Overview
- `robot_platform/CMakeLists.txt` builds the same runtime stack against two backend profiles: STM32 hardware and Linux SITL.
- `robot_platform/runtime/` is split into `generated -> bsp -> device -> control -> app`, with reusable primitives in `robot_platform/runtime/module/`.
- `robot_platform/sim/` mirrors the runtime boundary in Python by describing project profiles, launching a SITL bridge, and validating smoke-session outputs.
## Layers
- Purpose: Select board/project metadata and compose either hardware or SITL binaries.
- Location: `robot_platform/CMakeLists.txt`, `robot_platform/projects/balance_chassis/project.yaml`, `robot_platform/projects/balance_chassis/board.yaml`
- Contains: Target selection flags, source lists, include wiring, project metadata, board metadata.
- Depends on: `robot_platform/runtime/`, `robot_platform/third_party/`, generated CubeMX output in `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/`
- Used by: `python3 -m robot_platform.tools.platform_cli.main build`, `python3 -m robot_platform.tools.platform_cli.main sim`
- Purpose: Provide hardware registers/HAL startup on STM32 and stubbed low-level services for Linux SITL.
- Location: `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/`, `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/`, `robot_platform/runtime/bsp/sitl/`
- Contains: CubeMX output, board CAN/UART/PWM/DWT wrappers, SITL `main`, HAL stubs, socket-backed bus shims.
- Depends on: STM32 HAL/CMSIS/FreeRTOS sources referenced in `robot_platform/CMakeLists.txt`
- Used by: `balance_chassis_hw_seed.elf`, `balance_chassis_sitl`, device concrete adapters in `robot_platform/runtime/device/`
- Purpose: Normalize IMU, remote, and motor access behind stable runtime-facing interfaces.
- Location: `robot_platform/runtime/device/`
- Contains: `device_layer`, backend profiles, IMU/remote/motor semantic interfaces, backend-specific concrete adapters.
- Depends on: BSP handles and driver assets from `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/` or `robot_platform/runtime/bsp/sitl/`
- Used by: State estimation in `robot_platform/runtime/control/state/`, execution in `robot_platform/runtime/control/execution/`, remote intent in `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c`
- Purpose: Transform device inputs plus operator intent into robot state and actuator commands.
- Location: `robot_platform/runtime/control/`
- Contains: Runtime contracts, INS/chassis observation, balance controller, actuator gateway, task entrypoints.
- Depends on: `robot_platform/runtime/device/` for input/output and `robot_platform/runtime/module/` for math, filtering, and pub-sub.
- Used by: FreeRTOS app bring-up in `robot_platform/runtime/app/balance_chassis/app_bringup/`
- Purpose: Define task topology, project-specific startup, and intent translation for `balance_chassis`.
- Location: `robot_platform/runtime/app/balance_chassis/`
- Contains: FreeRTOS bootstrap, task registry, remote intent builder, app topic buses, app params/config.
- Depends on: `robot_platform/runtime/control/`, `robot_platform/runtime/device/`, `robot_platform/runtime/module/message_center/`
- Used by: Hardware startup through `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c` and SITL startup through `robot_platform/runtime/bsp/sitl/main_sitl.c`
- Purpose: Hold cross-robot algorithms, control helpers, and the in-process message bus.
- Location: `robot_platform/runtime/module/`
- Contains: EKF, PID, VMC, kalman, mahony, generic controller helpers, `control_math`, `message_center`.
- Depends on: C runtime and CMSIS DSP includes configured in `robot_platform/CMakeLists.txt`
- Used by: Controllers and state estimators throughout `robot_platform/runtime/control/`
- Purpose: Describe runtime boundaries, spawn SITL sessions, bridge UDP traffic, and generate smoke reports.
- Location: `robot_platform/sim/`, `robot_platform/tools/platform_cli/`
- Contains: Profiles, runner, backend contract, UDP bridge, report writer, per-project simulation adapter.
- Depends on: Built SITL binary from `robot_platform/CMakeLists.txt` and project descriptors in `robot_platform/sim/projects/`
- Used by: `robot_platform/tools/platform_cli/main.py`
## Data Flow
- Runtime state is mostly task-local structs such as `platform_balance_controller_t` in `robot_platform/runtime/control/controllers/balance_controller.c` and `platform_ins_state_estimator_t` in `robot_platform/runtime/control/state/ins_task.c`.
- Cross-task state is transported through named message-center topics rather than shared global state.
- Backend selection state is stored once inside `g_platform_default_layer` and `g_platform_default_profile` in `robot_platform/runtime/device/device_layer.c`.
- Simulation state is immutable configuration in dataclasses from `robot_platform/sim/core/profile.py` plus per-session summaries built in `robot_platform/sim/core/runner.py`.
## Key Abstractions
- Purpose: Aggregate semantic device endpoints and hide backend-specific driver binding.
- Examples: `robot_platform/runtime/device/device_layer.h`, `robot_platform/runtime/device/device_profile_hw.c`, `robot_platform/runtime/device/device_profile_sitl.c`
- Pattern: One semantic aggregate struct with profile-based binding of IMU, remote, and motor nodes.
- Purpose: Define formal data shapes between device, control, and app layers.
- Examples: `robot_platform/runtime/control/contracts/device_input.h`, `robot_platform/runtime/control/contracts/robot_state.h`, `robot_platform/runtime/control/contracts/actuator_command.h`, `robot_platform/runtime/control/contracts/runtime_contracts.h`
- Pattern: Header-only contract boundary shared by app, controller, device, and execution code.
- Purpose: Prevent raw string topic usage from leaking across most business logic.
- Examples: `robot_platform/runtime/control/state/ins_topics.c`, `robot_platform/runtime/control/state/observe_topics.c`, `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/control/execution/actuator_topics.c`
- Pattern: Small bus wrappers around `PubRegister`, `SubRegister`, `PubPushMessage`, and `SubGetMessage`.
- Purpose: Declare one project’s SITL target, bridge module, runtime topic boundaries, ports, and validation targets.
- Examples: `robot_platform/sim/core/profile.py`, `robot_platform/sim/projects/balance_chassis/profile.py`
- Pattern: Frozen dataclass configuration injected into generic runner and bridge code.
## Entry Points
- Location: `robot_platform/CMakeLists.txt`
- Triggers: `python3 -m robot_platform.tools.platform_cli.main build hw_elf`
- Responsibilities: Assemble STM32 HAL, generated CubeMX sources, board BSP, FreeRTOS, app, device, and control layers into `balance_chassis_hw_seed.elf`.
- Location: `robot_platform/runtime/bsp/sitl/main_sitl.c`
- Triggers: Executing `balance_chassis_sitl`
- Responsibilities: Start logging, call `MX_FREERTOS_Init()`, and hand execution to the FreeRTOS POSIX scheduler.
- Location: `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c`
- Triggers: Called from hardware-generated `freertos.c` or SITL `main_sitl.c`
- Responsibilities: Create the idle/default task and register project runtime tasks.
- Location: `robot_platform/tools/platform_cli/main.py`
- Triggers: `python3 -m robot_platform.tools.platform_cli.main ...`
- Responsibilities: Route generate/build/sim/test commands and keep build/run conventions in one place.
- Location: `robot_platform/sim/backends/sitl_bridge.py`
- Triggers: Spawned by the Python smoke runner using the profile’s `backend_module`
- Responsibilities: Open UDP ports, emit bridge metadata, synthesize IMU/motor traffic, and report stats/runtime-output observations.
## Error Handling
- Device functions in `robot_platform/runtime/device/device_layer.c` return `platform_device_result_t`, with `UNAVAILABLE` treated as non-fatal for partial reads inside `platform_device_layer_read_input()`.
- Runtime tasks such as `robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c` and `robot_platform/runtime/control/execution/motor_control_task.c` mostly ignore non-critical device return codes and keep the scheduler alive.
- Startup synchronization uses busy-wait readiness loops in `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c`, `robot_platform/runtime/control/state/observe_topics.c`, and `robot_platform/runtime/control/execution/actuator_topics.c`.
- Python orchestration returns process exit codes and serializes failures into smoke-report summaries in `robot_platform/sim/core/runner.py`.
- The bridge emits explicit `startup_error` events in `robot_platform/sim/backends/sitl_bridge.py` when adapter loading or socket binding fails.
## Cross-Cutting Concerns
<!-- GSD:architecture-end -->

<!-- GSD:workflow-start source:GSD defaults -->
## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd:quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd:debug` for investigation and bug fixing
- `/gsd:execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->



<!-- GSD:profile-start -->
## Developer Profile

> Profile not yet configured. Run `/gsd:profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
