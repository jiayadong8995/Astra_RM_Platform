# Codebase Structure

**Analysis Date:** 2026-03-30

## Directory Layout

```text
Astra_RM2025_Balance/
├── robot_platform/                 # First-party platform build, runtime, sim, tools, and project metadata
├── Astra_RM2025_Balance/           # Legacy STM32Cube project assets kept alongside the platformized runtime
├── references/                     # External reference repos and examples, not the primary implementation surface
├── build/                          # Generated build output created by CLI/CMake workflows
└── .planning/codebase/             # Generated codebase mapping documents
```

## Directory Purposes

**`robot_platform/`:**
- Purpose: Primary implementation root for current platformized architecture.
- Contains: CMake build graph, embedded runtime, SITL orchestration, Python CLI, project descriptors, vendored third-party dependencies.
- Key files: `robot_platform/CMakeLists.txt`, `robot_platform/runtime/README.md`, `robot_platform/sim/README.md`

**`robot_platform/runtime/`:**
- Purpose: First-party embedded runtime split by architectural layer.
- Contains: `generated/`, `bsp/`, `device/`, `control/`, `app/`, `module/`
- Key files: `robot_platform/runtime/README.md`, `robot_platform/runtime/device/device_layer.c`

**`robot_platform/runtime/generated/`:**
- Purpose: Hold generated STM32Cube output imported into the platform build.
- Contains: Raw generated board code under `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/`
- Key files: `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/main.c`, `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c`

**`robot_platform/runtime/bsp/`:**
- Purpose: Board-specific or backend-specific low-level access.
- Contains: Hardware BSP under `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/` and Linux/SITL stubs under `robot_platform/runtime/bsp/sitl/`
- Key files: `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/can_bsp.c`, `robot_platform/runtime/bsp/sitl/main_sitl.c`

**`robot_platform/runtime/device/`:**
- Purpose: Semantic device layer between drivers/BSP and control code.
- Contains: Backend profile binding, common device types, IMU/remote/actuator interfaces and implementations.
- Key files: `robot_platform/runtime/device/device_layer.h`, `robot_platform/runtime/device/device_profile_hw.c`, `robot_platform/runtime/device/device_profile_sitl.c`

**`robot_platform/runtime/control/`:**
- Purpose: Main control pipeline.
- Contains: `contracts/`, `state/`, `controllers/`, `constraints/`, `execution/`
- Key files: `robot_platform/runtime/control/controllers/balance_controller.c`, `robot_platform/runtime/control/execution/actuator_gateway.c`

**`robot_platform/runtime/app/`:**
- Purpose: Project-specific task composition and intent/business assembly.
- Contains: Current app implementation under `robot_platform/runtime/app/balance_chassis/`
- Key files: `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c`, `robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c`

**`robot_platform/runtime/module/`:**
- Purpose: Reusable utilities and algorithms shared across runtime layers.
- Contains: `algorithm/`, `controller/`, `lib/`, `message_center/`
- Key files: `robot_platform/runtime/module/message_center/message_center.c`, `robot_platform/runtime/module/lib/control/control_math.c`

**`robot_platform/sim/`:**
- Purpose: Python SITL orchestration and smoke-validation surface.
- Contains: `core/`, `backends/`, `projects/`, `reports/`, `tests/`, compatibility wrappers in `bridge/`
- Key files: `robot_platform/sim/core/runner.py`, `robot_platform/sim/backends/sitl_bridge.py`, `robot_platform/sim/projects/balance_chassis/profile.py`

**`robot_platform/projects/`:**
- Purpose: Project and board metadata, not business-logic implementation.
- Contains: Project descriptors such as `project.yaml` and `board.yaml`
- Key files: `robot_platform/projects/balance_chassis/project.yaml`, `robot_platform/projects/balance_chassis/board.yaml`

**`robot_platform/tools/`:**
- Purpose: Developer-facing automation and code-generation hooks.
- Contains: CLI entrypoint, CubeMX backend, schemas, codegen docs.
- Key files: `robot_platform/tools/platform_cli/main.py`, `robot_platform/tools/cubemx_backend/main.py`

**`Astra_RM2025_Balance/`:**
- Purpose: Legacy CubeMX project assets that still feed generation inputs.
- Contains: Board `.ioc` files and older generated firmware trees for chassis/gimbal.
- Key files: `Astra_RM2025_Balance/Chassis/CtrlBoard-H7_IMU.ioc`, `Astra_RM2025_Balance/Chassis/Core/Src/main.c`

## Key File Locations

**Entry Points:**
- `robot_platform/CMakeLists.txt`: Root build entry for hardware and SITL targets.
- `robot_platform/tools/platform_cli/main.py`: Unified command entry for generate/build/sim/test.
- `robot_platform/runtime/bsp/sitl/main_sitl.c`: Linux SITL executable entry.
- `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c`: Runtime app bootstrap called by both hardware and SITL startup paths.

**Configuration:**
- `robot_platform/projects/balance_chassis/project.yaml`: Project identity, board binding, default app, and supported runtime modes.
- `robot_platform/projects/balance_chassis/board.yaml`: MCU, peripheral, memory, and device inventory.
- `robot_platform/sim/projects/balance_chassis/profile.py`: Simulation boundary, ports, backend module, and validation declarations.
- `robot_platform/runtime/app/balance_chassis/app_config/app_params.h`: Task periods, priorities, and startup timing constants.

**Core Logic:**
- `robot_platform/runtime/device/device_layer.c`: Device aggregation and profile selection.
- `robot_platform/runtime/control/state/ins_task.c`: IMU-to-INS state task.
- `robot_platform/runtime/control/state/observe_task.c`: Intent/feedback observation task.
- `robot_platform/runtime/control/controllers/balance_controller.c`: Main balance control logic and contract output assembly.
- `robot_platform/runtime/control/execution/motor_control_task.c`: Actuator dispatch loop.
- `robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c`: Central app control task wiring the state/controller/output flow.

**Testing:**
- `robot_platform/sim/tests/test_runner.py`: Simulation smoke-summary and metadata parsing tests.
- `robot_platform/tools/platform_cli/tests/test_main.py`: CLI argument/command coverage.

## Naming Conventions

**Files:**
- C runtime source files use snake_case filenames such as `device_layer.c`, `balance_controller.c`, `motor_control_task.c`.
- Runtime contract headers use snake_case nouns such as `robot_state.h`, `device_feedback.h`, `actuator_command.h`.
- Python simulation files also use snake_case such as `sitl_bridge.py`, `report_writer.py`, `validation.py`.
- Project profile constants are uppercase in Python, but file names stay lowercase, for example `BALANCE_CHASSIS_PROFILE` in `robot_platform/sim/projects/balance_chassis/profile.py`.

**Directories:**
- Layer directories are lowercase nouns: `runtime/device`, `runtime/control`, `runtime/app`, `sim/core`, `sim/backends`.
- Project-specific directories use the project slug: `robot_platform/runtime/app/balance_chassis/`, `robot_platform/projects/balance_chassis/`, `robot_platform/sim/projects/balance_chassis/`.
- Backend-specific runtime directories are explicit about target type: `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/`, `robot_platform/runtime/bsp/sitl/`.

## Where to Add New Code

**New Feature:**
- Primary code: Put new robot-runtime behavior in the layer that matches its responsibility under `robot_platform/runtime/`.
- Tests: Put simulation or CLI regressions in `robot_platform/sim/tests/` or `robot_platform/tools/platform_cli/tests/` depending on ownership.

**New Control Logic:**
- State estimation additions: `robot_platform/runtime/control/state/`
- Control law additions: `robot_platform/runtime/control/controllers/`
- Output limiting or safety shaping: `robot_platform/runtime/control/constraints/`
- Device dispatch changes: `robot_platform/runtime/control/execution/`

**New Device Support:**
- Stable semantic interfaces: `robot_platform/runtime/device/imu/`, `robot_platform/runtime/device/remote/`, `robot_platform/runtime/device/actuator/`
- Backend profile binding: `robot_platform/runtime/device/device_profile_hw.c` and `robot_platform/runtime/device/device_profile_sitl.c`
- Backend-specific low-level access: `robot_platform/runtime/bsp/boards/<board>/` for hardware, `robot_platform/runtime/bsp/sitl/` for SITL stubs

**New App/Project Wiring:**
- Task orchestration and app buses: `robot_platform/runtime/app/<project>/app_bringup/` and `robot_platform/runtime/app/<project>/app_io/`
- Intent translation or mode-specific behavior: `robot_platform/runtime/app/<project>/app_intent/`
- Static app config: `robot_platform/runtime/app/<project>/app_config/`

**New Reusable Utilities:**
- Shared algorithms: `robot_platform/runtime/module/algorithm/<name>/`
- Shared control helpers: `robot_platform/runtime/module/lib/control/`
- Shared runtime messaging helpers: `robot_platform/runtime/module/message_center/`

**New Simulation Support:**
- Generic simulation infrastructure: `robot_platform/sim/core/`, `robot_platform/sim/backends/`, `robot_platform/sim/reports/`
- Project-specific simulation declarations or adapters: `robot_platform/sim/projects/<project>/`
- CLI routing for new workflows: `robot_platform/tools/platform_cli/main.py`

## Special Directories

**`robot_platform/runtime/generated/`:**
- Purpose: Imported generated firmware assets.
- Generated: Yes
- Committed: Yes

**`robot_platform/third_party/`:**
- Purpose: Vendored FreeRTOS port, STM32Cube, and other external low-level dependencies.
- Generated: No
- Committed: Yes

**`robot_platform/projects/`:**
- Purpose: Project metadata registry used by build and sim layers.
- Generated: No
- Committed: Yes

**`robot_platform/sim/projects/`:**
- Purpose: Per-project simulation configuration surface.
- Generated: No
- Committed: Yes

**`references/`:**
- Purpose: External reference codebases and experiments.
- Generated: No
- Committed: Yes

**`.planning/codebase/`:**
- Purpose: Generated codebase maps for later planning/execution agents.
- Generated: Yes
- Committed: Yes

---

*Structure analysis: 2026-03-30*
