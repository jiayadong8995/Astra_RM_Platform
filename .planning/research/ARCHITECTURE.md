# Architecture Patterns: v2 Platform Simplification

**Domain:** Simplifying Astra RM embedded platform from 5-6 layers to 4 layers
**Researched:** 2026-04-01
**Confidence:** HIGH (synthesized from STACK.md, FEATURES.md, PITFALLS.md codebase analysis)

## Current Architecture (Before)

```
runtime/
  generated/stm32h7_ctrl_board_raw/   -- CubeMX output
  bsp/
    boards/stm32h7_ctrl_board/         -- HW drivers (CAN, UART, PWM, DWT)
    sitl/                               -- SITL stubs + UDP adapters
  device/                               -- ★ REMOVE: 29 files, 2806 lines
    device_layer.c/h                    -- aggregate singleton + test hooks + profile dispatch
    device_profile.h                    -- vtable bind pattern
    device_profile_hw.c/sitl.c          -- HW/SITL profiles
    imu/, remote/, actuator/motor/      -- vtable wrappers for 3 devices
  control/
    contracts/                          -- typed messages (KEEP)
    state/, controllers/, constraints/  -- control logic (KEEP)
    execution/                          -- actuator gateway + motor control
    task_registry/, control_config/     -- control-owned registration
  app/balance_chassis/
    app_startup/, app_bringup/          -- startup + task registry
    app_io/                             -- ★ SIMPLIFY: 5 topic wrapper files
    app_intent/, app_config/            -- intent + params
  module/
    message_center/                     -- pub-sub (KEEP)
    algorithm/, lib/                    -- algorithms + math
```

**Problems:**
- `device/`: 29 files of vtable indirection for 3 devices. Runtime profile dispatch is always compile-time.
- `*_topics.c` wrappers: 10 files, 329 lines of ceremony around PubRegister/SubGetMessage
- Two parallel command types with 160 lines of field-by-field mapping
- 5 hops from control to hardware

## Target Architecture (After)

```
runtime/
  bsp/
    boards/stm32h7_ctrl_board/
      generated/                        -- CubeMX output (moved here)
      bsp_imu.c, bsp_remote.c, bsp_motor.c  -- HW port implementations
      bsp_dwt.c, bsp_uart.c, can_bsp.c      -- existing BSP
    sitl/
      bsp_imu_sitl.c, bsp_remote_sitl.c, bsp_motor_sitl.c
    ports.h                             -- narrow port interfaces
  control/
    contracts/                          -- typed messages (unchanged)
    topics.h                            -- topic name constants (replaces 10 wrapper files)
    state/, controllers/, constraints/  -- control logic (unchanged)
    execution/                          -- actuator gateway (simplified)
    task_registry/                      -- control-owned registration
  app/balance_chassis/
    app_startup/                        -- shared startup API
    task_registry.c                     -- FreeRTOS task creation + remote task
    motor_map.h                         -- robot-specific motor indexing
    app_config/                         -- app params
  module/
    message_center/                     -- pub-sub (unchanged)
    algorithm/, lib/                    -- algorithms + math
  tests/host/                           -- host tests (migrated)
```

**Result:** 4 layers (bsp → control → app → module), 2 hops from control to hardware.

## Key Changes

### 1. Device Layer → BSP Ports (Link-Time Polymorphism)

**Before:** control → device_layer singleton → device_profile vtable → device adapter → BSP (5 hops)
**After:** control → BSP port function → BSP implementation (2 hops)

```c
// bsp/ports.h
platform_device_result_t platform_imu_read(platform_imu_sample_t *out);
platform_device_result_t platform_remote_read(platform_rc_input_t *out);
platform_device_result_t platform_motor_read_feedback(platform_device_feedback_t *out);
platform_device_result_t platform_motor_write_command(const platform_motor_command_set_t *cmd);
```

CMake links HW, SITL, or test-fake `.c` per target. No runtime dispatch. No singleton.

### 2. Message Center: Keep Bus, Remove Wrappers

All 6 topics are genuinely cross-task — keep them all:
- `ins_data` (1 pub, 3 subs), `robot_intent` (1 pub, 2 subs), `device_feedback` (1 pub, 2 subs)
- `chassis_observe` (1 pub, 1 sub), `robot_state` (1 pub, 1 sub), `actuator_command` (1 pub, 1 sub)

Remove 10 `*_topics.c/h` wrapper files. Tasks call PubRegister/SubGetMessage directly.
Add single `control/topics.h` with topic name constants.
Extract `wait_ready` gates into explicit named functions before removing wrappers.
Migrate observation globals from `chassis_topics.c` to dedicated location.

### 3. Command Type Unification

Merge `platform_actuator_command_t` + `platform_device_command_t` into one `platform_motor_command_set_t` with indexed access. Robot-specific semantic names via enum in `app/balance_chassis/motor_map.h`. Eliminates 160 lines of mapping.

### 4. Backend Selection: Compile-Time via CMake

Replace runtime `platform_select_profile()` with CMake link-time source selection. Same pattern `device_layer_stubs.c` already uses for tests.

## Migration Order (Risk-Minimizing)

From PITFALLS.md — each step keeps all tests green:

1. **Unify command types** — Pure refactor, eliminates 160 lines of mapping
2. **Introduce BSP port interfaces** — Add `ports.h`, implement alongside old path
3. **Migrate test seams** — Replace device_layer test hooks with link-time BSP port fakes
4. **Migrate control code to ports** — ins_task, remote_task, motor_control_task use port functions
5. **Extract readiness gates** — Move `wait_ready` from topic wrappers to explicit functions
6. **Delete device layer** — Remove 29 files of indirection
7. **Consolidate topic wrappers** — Replace 10 files with direct pub/sub + topics.h
8. **Flatten directories** — Move generated into BSP, update CMake

Critical invariant: `test_balance_safety_path` never goes red.

## Complexity Reduction

| Metric | Before (v1) | After (v2) | Delta |
|--------|-------------|------------|-------|
| Runtime layers | 5-6 | 4 | -2 |
| Device layer files | 29 | 0 | -29 |
| Topic wrapper files | 10 | 1 | -9 |
| Command mapping LOC | ~160 | 0 | -160 |
| Indirection depth | 5 hops | 2 hops | -3 |
| Max directory nesting | 4 levels | 2 levels | -2 |

## Sources

- STACK.md: device_layer analysis, topic map, command type audit, migration strategy
- FEATURES.md: complexity baseline, feature dependencies, outcome metrics
- PITFALLS.md: test seam dependencies, readiness gate locations, scope creep risks
- Reference: basic_framework (4-layer, restrained pub/sub), StandardRobotpp (direct state), XRobot (flat modules), ROS 2 (standardized interfaces)
