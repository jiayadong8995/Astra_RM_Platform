# Technology Stack: v2 Platform Simplification

**Project:** Astra RM Robot Platform — v2 Milestone
**Researched:** 2026-04-01
**Focus:** Stack patterns for simplifying from 5-6 layers to 4 layers while preserving host-side verification

## Executive Decision

Keep the same toolchain (C11, CMake, Python CLI, FreeRTOS, STM32CubeMX, arm-none-eabi-gcc). The v2 stack change is not about swapping technologies — it is about removing unnecessary abstraction layers and narrowing the internal wiring patterns. The tools stay; the architecture inside the C runtime gets flatter.

## What To Keep (Unchanged)

| Technology | Version | Purpose | Rationale |
|------------|---------|---------|-----------|
| C11 | Existing | Runtime code | No reason to change. All runtime code is C. |
| CMake 3.22+ / CTest | Existing | Build graph, host tests | v1 already unified HW, SITL, and host tests in one CMake graph. Keep it. |
| Python 3 | Existing | CLI, sim orchestration, reports | Tooling layer only. Not changing. |
| FreeRTOS (vendored) | Existing | Task scheduling on HW and SITL | Tasks remain the execution model. Simplification is about what runs inside tasks, not the scheduler. |
| arm-none-eabi-gcc | Existing | Cross-compilation | Hardware target unchanged. |
| GCC (Linux host) | Existing | SITL and host tests | Host verification lane unchanged. |
| STM32CubeMX 6.17.0 | Existing | Board code generation | Generated code path unchanged. |
| ASan/UBSan | Existing | Host test sanitizers | Already wired in CMakeLists.txt. Keep for all host targets. |
| Unity + FFF | v1 vendored | C unit test assertions and fakes | v1 added these. They become even more important during refactoring. |
| message_center | Existing | Static pub-sub | Keep the module itself. Narrow its usage (see below). |

## What To Remove

### 1. Remove: `device_layer` Aggregate + Profile Binding System

**Current state (30 files, ~430 LOC of indirection):**
```
runtime/device/
  device_layer.c          -- aggregate struct + global singleton + lazy init + test hooks
  device_layer.h          -- 15 public functions for read/write/init/configure/hooks
  device_profile.h        -- vtable-style bind_imu/bind_remote/bind_motor profile
  device_profile_hw.c     -- HW profile: binds 3 device adapters
  device_profile_sitl.c   -- SITL profile: binds 3 device adapters
  device_types.h          -- result enum + backend enum + stamp
```

**Why remove:** This layer exists to aggregate IMU, remote, and motor behind one facade with runtime profile selection. In practice:
- Only one profile is ever active per build (HW or SITL, selected at compile time via `#ifdef SITL_BUILD`)
- The "auto" profile selection in `platform_select_profile()` has dead code paths (double `#ifdef SITL_BUILD` check)
- The lazy-init singleton (`g_platform_default_layer` + `g_platform_default_layer_ready`) adds hidden state
- The test hooks system (`platform_device_test_hooks_t`) duplicates what link-level fakes already provide
- The `platform_map_device_command_to_motor_set()` function is 56 lines of field-by-field copy that exists only because `device_command_t` and `motor_command_set_t` are redundant representations
- Control code calls `platform_device_read_default_imu()` which goes through 3 indirections to reach the actual BSP read

**Replace with:** Direct BSP-backed read/write functions selected at link time (compile-time backend selection). This is the pattern basic_framework and StandardRobotpp use successfully.

**New pattern:**
```c
// bsp/imu_port.h — one narrow interface
typedef struct {
    platform_device_result_t (*read_sample)(platform_imu_sample_t *out);
} platform_imu_port_t;

// bsp/boards/stm32h7_ctrl_board/imu_port_hw.c — HW implementation
// bsp/sitl/imu_port_sitl.c — SITL implementation
// tests/host/test_support/imu_port_fake.c — test fake
```

CMake selects which `.c` file to link. No runtime profile switching. No global singleton. No lazy init.

**Impact on tests:** The existing `device_layer_stubs.c` test support file already bypasses the device_layer by providing stub implementations of `platform_device_init_default_profile()`, `platform_device_read_default_feedback()`, and `platform_device_write_default_command()`. After removal, tests link against fake port implementations directly — same pattern, less indirection.

**Confidence:** HIGH. The codebase evidence is clear: runtime profile selection is compile-time, the aggregate adds no runtime value, and tests already bypass it.

### 2. Remove: `device/actuator/`, `device/imu/`, `device/remote/` Semantic Wrappers

**Current state (14 files):**
```
device/imu/imu_device.h                    -- ops vtable for IMU
device/imu/bmi088_device.h                 -- config struct for BMI088
device/imu/bmi088_device_hw.c              -- HW adapter
device/imu/bmi088_device_sitl.c            -- SITL adapter
device/remote/remote_device.h              -- ops vtable for remote
device/remote/dbus_remote_device.h         -- config struct for DBUS
device/remote/dbus_remote_device_hw.c      -- HW adapter
device/remote/dbus_remote_device_sitl.c    -- SITL adapter
device/actuator/actuator_device.h          -- (empty/minimal)
device/actuator/motor/motor_device.h       -- ops vtable for motor
device/actuator/motor/motor_actuator_device.h  -- config struct
device/actuator/motor/motor_actuator_device_hw.c   -- HW adapter
device/actuator/motor/motor_actuator_device_sitl.c -- SITL adapter
```

**Why remove:** These are vtable wrappers around BSP functions. Each "device" struct holds a name, context pointer, stamp, and ops table — but the ops are always bound once at init and never swapped. The vtable pattern would matter if devices were hot-pluggable or if multiple IMU types coexisted at runtime. Neither is true.

**Replace with:** Move the actual driver logic (BMI088 read, DBUS parse, DM4310 command) into BSP-level files. The BSP already contains the real implementations (`bmi088/BMI088driver.c`, `dbus/remote_control.c`, `dm4310/dm4310_drv.c`). The semantic wrappers just forward calls.

**New structure:**
```
bsp/
  boards/stm32h7_ctrl_board/
    bsp_imu.c       -- reads BMI088, returns platform_imu_sample_t
    bsp_remote.c     -- reads DBUS, returns platform_rc_input_t
    bsp_motor.c      -- reads/writes DM4310+chassis motors
    bsp_dwt.c        -- (existing)
    bsp_uart.c       -- (existing)
    can_bsp.c        -- (existing)
  sitl/
    bsp_imu_sitl.c   -- SITL IMU stub
    bsp_remote_sitl.c -- SITL remote stub
    bsp_motor_sitl.c  -- SITL motor stub
```

Each BSP file implements the same port interface. CMake links the right one.

**Confidence:** HIGH. Direct code inspection shows the vtable indirection is never exercised dynamically.

### 3. Remove: Redundant Contract Types

**Current state:** Two parallel command representations exist:
- `platform_actuator_command_t` (control-level, with `motors.left_leg_joint[0..1]`, `motors.right_leg_joint[0..1]`, `motors.left_wheel`, `motors.right_wheel`)
- `platform_device_command_t` (device-level, with `joints[0..3]`, `wheels[0..1]`)

The `actuator_gateway.c` contains 106 lines of field-by-field mapping between them. Similarly, `device_layer.c` has `platform_map_device_command_to_motor_set()` with 56 lines of field-by-field copy.

**Why remove:** These are the same data in different shapes. The "semantic" naming (left_leg_joint vs joints[0]) is a robot-profile concern, not a platform concern. Having two representations forces two mapping functions and doubles the test surface for what is logically one command.

**Replace with:** One canonical command type. Use the indexed form (`joints[N]`, `wheels[N]`) as the platform type. Robot-profile code (in `app/balance_chassis/`) can define named accessors or enums for semantic indexing:

```c
// contracts/actuator_command.h — single canonical type
typedef struct {
    platform_motor_command_t joints[PLATFORM_JOINT_MOTOR_COUNT];
    platform_motor_command_t wheels[PLATFORM_WHEEL_MOTOR_COUNT];
} platform_motor_command_set_t;

// app/balance_chassis/motor_map.h — robot-specific semantic names
enum balance_chassis_joint_id {
    BC_LEFT_LEG_UPPER  = 0,
    BC_LEFT_LEG_LOWER  = 1,
    BC_RIGHT_LEG_UPPER = 2,
    BC_RIGHT_LEG_LOWER = 3,
};
```

This eliminates both mapping functions entirely.

**Confidence:** HIGH. The mapping code is visible in `actuator_gateway.c` and `device_layer.c`.

## What To Change

### 1. Narrow message_center Usage

**Current topic map (6 topics, 22 pub/sub registrations):**

| Topic | Publisher | Subscribers | Cross-task? |
|-------|-----------|-------------|-------------|
| `ins_data` | INS_task (ins_topics.c) | observe_task, chassis_task, motor_control_task | YES — 3 separate tasks consume INS state |
| `robot_intent` | remote_task (remote_topics.c) | observe_task, chassis_task | YES — 2 tasks consume intent |
| `chassis_observe` | observe_task (observe_topics.c) | chassis_task | YES — but only 1 consumer |
| `device_feedback` | motor_control_task (actuator_topics.c) | observe_task, chassis_task | YES — 2 tasks consume feedback |
| `robot_state` | chassis_task (chassis_topics.c) | remote_task | YES — but only 1 consumer |
| `actuator_command` | chassis_task (chassis_topics.c) | motor_control_task | YES — but only 1 consumer |

**Analysis:** All 6 topics are genuinely cross-task (different FreeRTOS threads). The message_center is doing real work here — it provides safe cross-task data sharing without mutexes. This is the same pattern basic_framework uses.

**Recommendation:** Keep all 6 topics. The message_center is not over-used — it is correctly scoped to cross-task communication. The simplification opportunity is not in removing topics but in:

1. Removing the per-subsystem `*_topics.c` / `*_topics.h` wrapper files (10 files total). These add a function-call layer over `PubRegister`/`SubRegister`/`PubPushMessage`/`SubGetMessage` that provides no safety benefit. Each task should register its own pub/sub handles directly.

2. Centralizing topic name definitions in one header instead of scattering string literals across 6 files:

```c
// control/topics.h
#define TOPIC_INS_DATA        "ins_data"
#define TOPIC_ROBOT_INTENT    "robot_intent"
#define TOPIC_CHASSIS_OBSERVE "chassis_observe"
#define TOPIC_DEVICE_FEEDBACK "device_feedback"
#define TOPIC_ROBOT_STATE     "robot_state"
#define TOPIC_ACTUATOR_CMD    "actuator_command"
```

3. Moving the observation/instrumentation code out of `chassis_topics.c` (the `g_actuator_command_observed` globals and `[RuntimeOutput]` printf) into a dedicated observation module.

**What NOT to do:** Do not replace message_center with direct struct sharing or global variables. The generation-based staleness detection (`SubGetMessage` returns 0 if no new data) is genuinely useful for the control loop. StandardRobotpp's direct-shared-state approach works but loses staleness detection, which matters for safety.

**Confidence:** HIGH. Complete topic map extracted from codebase grep.

### 2. Flatten to 4 Layers: bsp -> control -> app -> module

**Current layers (5-6):**
```
generated -> bsp -> device -> control -> app -> module (cross-cutting)
```

**Target layers (4):**
```
bsp -> control -> app -> module (cross-cutting)
```

Where:
- `generated/` merges into `bsp/` (it is already BSP-level code, just CubeMX-produced)
- `device/` is eliminated (BSP implements port interfaces directly, control consumes ports)
- `control/` keeps state estimation, controllers, execution, constraints, contracts
- `app/` keeps robot-profile composition, task wiring, intent translation
- `module/` keeps algorithms, message_center, math utilities

**New directory structure:**
```
runtime/
  bsp/
    boards/stm32h7_ctrl_board/     -- HW BSP + generated code
    sitl/                           -- SITL BSP
    ports.h                         -- port interface definitions
  control/
    contracts/                      -- typed messages (keep)
    state/                          -- INS estimation, chassis observer
    controllers/                    -- balance controller
    constraints/                    -- actuator constraints, safety
    execution/                      -- actuator gateway (simplified)
    topics.h                        -- topic name constants
  app/
    balance_chassis/
      task_registry.c              -- FreeRTOS task creation
      remote_task.c                -- intent translation
      chassis_task.c               -- main control task
      motor_map.h                  -- robot-specific motor indexing
  module/
    algorithm/                     -- EKF, PID, VMC, kalman, mahony
    controller/                    -- generic controller helpers
    lib/                           -- math, user_lib
    message_center/                -- pub-sub (keep)
  tests/
    host/                          -- host-side C tests (keep)
```

**Key moves:**
- `device/imu/bmi088/` driver files stay in `bsp/boards/stm32h7_ctrl_board/` (they are BSP)
- `device/remote/dbus/` driver files stay in `bsp/boards/stm32h7_ctrl_board/` (they are BSP)
- `device/actuator/motor/dm4310/` driver files stay in `bsp/boards/stm32h7_ctrl_board/` (they are BSP)
- `device_layer.c`, `device_profile*.c`, all `*_device.h` vtable headers are deleted
- `generated/stm32h7_ctrl_board_raw/` becomes `bsp/boards/stm32h7_ctrl_board/generated/`

**Confidence:** HIGH. This matches basic_framework's proven 4-layer structure and eliminates the device layer that code inspection shows is pure indirection.

### 3. Simplify CMakeLists.txt

**Current pain:** The CMakeLists.txt is 989 lines with massive include directory lists repeated 4+ times (HW target, SITL target, host test library, individual test executables). The `balance_chassis_device` library exists solely to package the device layer.

**Changes:**
- Remove `balance_chassis_device` library target entirely
- BSP source selection becomes: `if(PLATFORM_TARGET_HW)` adds HW BSP sources, `if(PLATFORM_TARGET_SIM)` adds SITL BSP sources, `if(PLATFORM_HOST_TESTS)` adds fake BSP sources
- Consolidate include directories into CMake interface libraries to avoid repetition:

```cmake
add_library(platform_includes INTERFACE)
target_include_directories(platform_includes INTERFACE
    ${PLATFORM_RUNTIME_DIR}/control/contracts
    ${PLATFORM_RUNTIME_DIR}/module/message_center
    ${PLATFORM_RUNTIME_DIR}/module/lib
    # ... other shared includes
)
```

- Each test executable links `platform_includes` instead of repeating 30+ include paths

**Confidence:** HIGH. Direct observation of CMakeLists.txt duplication.

### 4. Simplify actuator_gateway.c

**Current state:** 142 lines, mostly field-by-field mapping between `platform_actuator_command_t` and `platform_device_command_t`.

**After contract unification:** The gateway becomes ~30 lines:
- Validate command (control mode checks)
- Check dispatch enabled (control_enable && actuator_enable)
- Write command to BSP motor port

The 106-line `platform_map_contract_command()` function disappears because there is only one command type.

**Confidence:** HIGH. The mapping code is mechanically eliminable once the duplicate types merge.

## What NOT To Do

### Do not remove message_center
The pub-sub is correctly scoped. All 6 topics are genuinely cross-task. Replacing it with direct shared state loses staleness detection. Keep it.

### Do not merge control/ into app/
Control logic (state estimation, balance controller, constraints) must remain robot-agnostic. App is where robot-specific composition lives. This boundary is correct and should be preserved.

### Do not introduce runtime plugin loading
XRobot's composable module pattern is elegant but adds complexity this platform does not need. The platform has one robot profile active per build. Compile-time selection is sufficient.

### Do not add a hardware abstraction layer on top of BSP
The device layer was already an HAL-over-HAL. Going from `control -> device_layer -> device_profile -> device_adapter -> BSP -> HAL` to `control -> BSP port -> BSP -> HAL` is the right direction. Do not re-introduce a middle layer.

### Do not refactor message_center internals
The static-pool implementation is fine for the current scale (8 topics, 16 subscribers, 2KB payload). Rewriting it to be "cleaner" adds risk without value. Just narrow the wrapper code around it.

### Do not change the FreeRTOS task structure
The 5-task topology (INS, observe, chassis, motor_control, remote) is correct for the control loop. Simplification is about what code runs inside each task, not about merging or splitting tasks.

## Verification Impact Assessment

**Tests that need updating after simplification:**

| Test File | Current Dependency | Change Needed |
|-----------|-------------------|---------------|
| `test_actuator_gateway.c` | Links `device_layer_stubs.c` | Link BSP port fakes instead |
| `test_device_profile_safety_seams.c` | Tests device_layer profile binding | Delete — the concept being tested no longer exists |
| `test_device_profile_sitl_runtime_bindings.c` | Tests SITL profile binding | Delete — replaced by BSP port integration test |
| `test_balance_safety_path.c` | Links `balance_safety_host_runtime` which includes device_layer | Update library to use BSP port fakes |
| `test_safety_*.c` (5 files) | Link `balance_safety_host_runtime` | Update library sources list |
| `test_message_center.c` | Independent | No change needed |
| `test_balance_app_startup.c` | Independent | No change needed |

**Net test count:** Lose 2 device-profile tests, gain equivalent BSP port tests. Total test count stays roughly the same. Safety tests are preserved — they test control logic, not device plumbing.

**Host verification capability preserved:** The key v1 achievement — host-side TDD with ASan/UBSan catching control and data-link errors before hardware — is fully preserved. The simplification removes indirection between tests and the code they exercise, making tests more direct.

**Confidence:** HIGH. Test file inventory and dependency analysis from CMakeLists.txt.

## Migration Strategy

**Order matters.** Do not attempt all removals simultaneously.

1. **Unify command types first** — Merge `platform_device_command_t` into `platform_actuator_command_t`. This is a pure refactor with no behavioral change. All existing tests validate the result.

2. **Introduce BSP port interfaces** — Add `ports.h` with `platform_imu_port_t`, `platform_remote_port_t`, `platform_motor_port_t`. Have existing device adapters implement them alongside the old vtable path temporarily.

3. **Migrate control code to use ports** — Change `ins_task.c`, `remote_task.c`, `motor_control_task.c` to call port functions instead of `platform_device_read_default_*()`.

4. **Delete device layer** — Remove `device_layer.c`, `device_profile*.c`, all `*_device.h` vtable headers, and the `balance_chassis_device` CMake target.

5. **Consolidate topic wrappers** — Replace 10 `*_topics.c`/`*_topics.h` files with direct pub/sub calls and a single `topics.h` constants header.

6. **Flatten directory structure** — Move driver files into BSP, merge generated into BSP, update CMake include paths.

Each step is independently testable. If any step breaks host tests, stop and fix before proceeding.

## Estimated Complexity Reduction

| Metric | Before (v1) | After (v2) | Delta |
|--------|-------------|------------|-------|
| Runtime directories | 18 | 12 | -6 |
| Device layer files | 30 | 0 | -30 |
| Topic wrapper files | 10 | 1 | -9 |
| Command mapping LOC | ~160 | 0 | -160 |
| CMake include path entries | ~120 (repeated) | ~40 (via interface lib) | -80 |
| Indirection depth (control->HW) | 5 hops | 2 hops | -3 |
| Public device API functions | 15 | 3 (per port) | -6 net |

## Sources

- `robot_platform/runtime/device/device_layer.c` — 429 lines, global singleton, lazy init, test hooks, dual command mapping
- `robot_platform/runtime/device/device_layer.h` — 15 public functions
- `robot_platform/runtime/device/device_profile.h` — vtable bind pattern
- `robot_platform/runtime/device/device_profile_hw.c` — HW profile (3 bind functions)
- `robot_platform/runtime/device/device_profile_sitl.c` — SITL profile (3 bind functions)
- `robot_platform/runtime/control/execution/actuator_gateway.c` — 142 lines, 106 lines of mapping
- `robot_platform/runtime/control/execution/actuator_topics.c` — topic wrapper
- `robot_platform/runtime/control/state/ins_topics.c` — topic wrapper
- `robot_platform/runtime/control/state/observe_topics.c` — topic wrapper
- `robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c` — topic wrapper + observation globals
- `robot_platform/runtime/app/balance_chassis/app_io/remote_topics.c` — topic wrapper
- `robot_platform/runtime/tests/host/test_support/device_layer_stubs.c` — test stubs bypassing device_layer
- `robot_platform/CMakeLists.txt` — 989 lines, repeated include paths, device library target
- `robot_platform/runtime/module/message_center/message_center.c` — static pub-sub (keep)
- `robot_platform/runtime/module/message_center/message_center.h` — 8 topics, 16 subscribers capacity
- Reference: basic_framework 4-layer pattern (bsp -> modules -> application -> runtime)
- Reference: StandardRobotpp direct shared state pattern (considered, rejected for losing staleness detection)
- Reference: XRobot composable modules (considered, rejected as over-engineered for single-profile builds)
