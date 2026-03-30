# Coding Conventions

**Analysis Date:** 2026-03-30

## Naming Patterns

**Files:**
- Use `snake_case` for Python modules and C source/header files such as `robot_platform/sim/core/runner.py`, `robot_platform/tools/platform_cli/main.py`, `robot_platform/runtime/control/execution/motor_control_task.c`, and `robot_platform/runtime/control/controllers/balance_controller.h`.
- Use directory names to encode subsystem ownership, for example `robot_platform/runtime/control/controllers`, `robot_platform/runtime/device/imu`, and `robot_platform/sim/projects/balance_chassis`.

**Functions:**
- Use `snake_case` for Python functions such as `run_sitl_session` in `robot_platform/sim/runner.py`, `write_report` in `robot_platform/sim/reports/report_writer.py`, and `_parse_sim_args` in `robot_platform/tools/platform_cli/main.py`.
- Prefix Python internal helpers with `_` when they are module-private, as in `robot_platform/sim/core/runner.py` and `robot_platform/tools/platform_cli/main.py`.
- Use `platform_` prefixes for reusable C APIs such as `platform_balance_controller_init` in `robot_platform/runtime/control/controllers/balance_controller.c`, `platform_float_clamp` in `robot_platform/runtime/module/lib/control/control_math.c`, and `platform_motor_actuator_device_bind` in `robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c`.
- Preserve legacy task and HAL callback names where framework integration requires them, for example `INS_task` in `robot_platform/runtime/control/state/ins_task.c`, `Chassis_task` via `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c`, and `HAL_UARTEx_RxEventCallback` in `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_uart.c`.

**Variables:**
- Use descriptive `snake_case` for Python locals and arguments such as `duration_s`, `supported_projects`, and `runtime_output_observations` in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/core/runner.py`.
- C locals and fields are also predominantly `snake_case`, for example `runtime_bus`, `device_feedback`, `turn_torque`, and `leg_length_target` across `robot_platform/runtime/control/execution/motor_control_task.c` and `robot_platform/runtime/control/controllers/balance_controller.c`.
- Keep macro-style all-caps names for constants and task handles supplied by embedded frameworks, such as `INS_TASKHandle` in `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c` and `MSG_MAX_TOPICS` in `robot_platform/runtime/module/message_center/message_center.h`.

**Types:**
- Python uses `PascalCase` dataclasses and type aliases such as `SimProjectProfile`, `RuntimeTopicBoundary`, `TransportPorts`, and `ValidationStatus` in `robot_platform/sim/core/profile.py`.
- C uses `_t` suffixed typedefs for most project-owned structs, such as `platform_balance_controller_t` in `robot_platform/runtime/control/controllers/balance_controller.h` and `platform_motor_device_t` in `robot_platform/runtime/device/actuator/motor/motor_device.h`.
- Keep vendor or legacy types unchanged, for example `PidTypeDef` and `Subscriber_t` in `robot_platform/runtime/control/controllers/balance_controller.h` and `robot_platform/runtime/module/message_center/message_center.h`.

## Code Style

**Formatting:**
- No first-party formatter config was detected under `robot_platform/`; `.clang-format`, `.clang-tidy`, `.prettierrc`, `eslint.config.*`, and `pyproject.toml` are not present in the active project tree.
- C code uses K&R-style braces with function braces on the next line, as in `robot_platform/runtime/control/controllers/balance_controller.c` and `robot_platform/runtime/control/state/ins_task.c`.
- Indentation is not fully uniform. New code should match the surrounding file rather than forcing a global style shift. `robot_platform/runtime/control/controllers/balance_controller.c` uses mostly four-space indentation, while `robot_platform/runtime/control/state/ins_task.c` has inconsistent spacing.
- Python uses four-space indentation, type annotations, and compact module headers, as in `robot_platform/tools/platform_cli/main.py`, `robot_platform/sim/core/profile.py`, and `robot_platform/sim/reports/report_writer.py`.

**Linting:**
- No dedicated lint configuration is detected for either C or Python in the first-party tree.
- The enforced quality gate is currently compiler warnings in `robot_platform/CMakeLists.txt`: `-Wall`, `-Wextra`, and `-Wno-unused-parameter` on C targets.
- Python quality relies on standard-library discipline and tests rather than a configured linter.

## Import Organization

**Order:**
1. Future imports first in Python modules, for example `from __future__ import annotations` in `robot_platform/sim/core/profile.py` and `robot_platform/sim/tests/test_runner.py`.
2. Standard-library imports next, such as `json`, `subprocess`, `sys`, `time`, and `Path` in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/core/runner.py`.
3. Project imports last, grouped from broader packages to narrower modules, as in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/projects/balance_chassis/profile.py`.

**Path Aliases:**
- Python uses package-relative absolute imports rooted at `robot_platform`, for example `from robot_platform.sim.core.profile import SimProjectProfile` in `robot_platform/sim/core/runner.py`.
- C uses relative include paths within each subsystem, for example `../constraints/actuator_constraints.h` in `robot_platform/runtime/control/controllers/balance_controller.c` and `../../device/device_layer.h` in `robot_platform/runtime/control/state/ins_task.c`.
- Framework and generated includes are referenced by bare header name when injected via CMake include directories, such as `cmsis_os.h` and `bsp_dwt.h` in `robot_platform/runtime/control/state/ins_task.c`.

## Error Handling

**Patterns:**
- Python favors return-code based command orchestration and explicit early returns. `_run`, `_build_hw_seed`, `_build_sitl`, and `_run_tests` in `robot_platform/tools/platform_cli/main.py` return process exit codes instead of raising application-specific exceptions.
- Python uses `ValueError` for argument validation, as in `_parse_sim_args` in `robot_platform/tools/platform_cli/main.py`.
- Python summary/report code is defensive around partial data, using `isinstance` checks and silent skips for malformed payloads in `robot_platform/sim/core/runner.py`.
- C runtime code usually handles device failures by skipping an update and continuing the task loop, for example `platform_device_read_default_imu(...) == PLATFORM_DEVICE_RESULT_OK` in `robot_platform/runtime/control/state/ins_task.c` and `platform_actuator_gateway_capture_feedback(...) == PLATFORM_DEVICE_RESULT_OK` in `robot_platform/runtime/control/execution/motor_control_task.c`.
- Low-level BSP code often checks HAL return codes but does not surface rich diagnostics, as seen in `robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/can_bsp.c`.
- Static-memory utilities return null pointers or sentinel values instead of asserting, for example `PubRegister` and `SubRegister` in `robot_platform/runtime/module/message_center/message_center.c` return `(void *)0` on pool exhaustion.

## Logging

**Framework:** `print` / `printf`

**Patterns:**
- Python CLI and simulation tooling use `print(...)` for command echoing, JSON summaries, and simple terminal status output in `robot_platform/tools/platform_cli/main.py` and `robot_platform/sim/backends/sitl_bridge.py`.
- SITL entrypoints use tagged `printf` lines such as `[SITL] ...` in `robot_platform/runtime/bsp/sitl/main_sitl.c`.
- There is no centralized logging abstraction in first-party code. New logs should follow the existing tagged plain-text style rather than introducing a second logging framework.

## Comments

**When to Comment:**
- Comments are sparse in runtime control code and are mainly used around non-obvious framework behavior or memory constraints.
- Use brief explanatory comments around static-allocation assumptions and integration boundaries, following `robot_platform/runtime/module/message_center/message_center.h` and `robot_platform/runtime/module/message_center/message_center.c`.
- Avoid narrating straightforward assignments; most control and task code such as `robot_platform/runtime/control/controllers/balance_controller.c` and `robot_platform/runtime/control/execution/motor_control_task.c` is left uncommented.

**JSDoc/TSDoc:**
- Not applicable.
- C uses Doxygen-style header comments selectively in reusable modules, for example `robot_platform/runtime/module/message_center/message_center.h`.
- Python generally does not use docstrings beyond the module-level summary in `robot_platform/tools/platform_cli/main.py`.

## Function Design

**Size:** 
- Public entrypoints are small coordinators that delegate to helpers. Examples include `platform_balance_controller_step` in `robot_platform/runtime/control/controllers/balance_controller.c` and `main` in `robot_platform/tools/platform_cli/main.py`.
- Large data-processing modules keep many private helpers in one file rather than splitting aggressively, as in `robot_platform/sim/core/runner.py` and `robot_platform/runtime/control/controllers/balance_controller.c`.

**Parameters:**
- Python signatures are annotated and often keyword-oriented for externally used functions, such as `run_sitl_session(*, repo_root: Path, duration_s: float = 3.0)` in `robot_platform/sim/runner.py`.
- C APIs pass output/state through pointers and avoid heap allocation, as in `platform_balance_controller_build_outputs` in `robot_platform/runtime/control/controllers/balance_controller.c`.

**Return Values:**
- Python command functions return integer shell-style status codes in `robot_platform/tools/platform_cli/main.py`.
- Python reporting helpers usually mutate a provided summary dict in place and return `None`, as in `_summarize_bridge_stats` and `_build_smoke_result` in `robot_platform/sim/core/runner.py`.
- C task functions are `void` infinite loops, and device/helper functions return enums, integers, or booleans when a success/failure signal is needed.

## Module Design

**Exports:**
- Python modules expose a small public surface and leave implementation helpers private with `_` prefixes, as in `robot_platform/sim/runner.py` re-exporting selected helpers from `robot_platform/sim/core/runner.py`.
- C headers declare the subsystem API and keep internal helpers `static` inside the `.c` file, as in `robot_platform/runtime/control/controllers/balance_controller.c`.

**Barrel Files:**
- Minimal Python package barrels are used where discovery matters. `robot_platform/sim/projects/__init__.py` centralizes project lookup functions.
- No C barrel-header pattern is present beyond subsystem headers and CMake include directory aggregation.

---

*Convention analysis: 2026-03-30*
