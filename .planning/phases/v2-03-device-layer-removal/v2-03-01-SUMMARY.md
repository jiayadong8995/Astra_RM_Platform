---
phase: v2-03
plan: 01
subsystem: bsp
tags: [ports, device-layer, rewire, direct-driver]
dependency-graph:
  requires: [v2-02]
  provides: [bsp-ports-direct-driver-calls]
  affects: [v2-03-02, v2-03-03]
tech-stack:
  added: []
  patterns: [lazy-bind-on-first-call]
key-files:
  created: []
  modified:
    - robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_ports_hw.c
    - robot_platform/runtime/bsp/sitl/bsp_ports_sitl.c
    - robot_platform/runtime/control/execution/actuator_gateway.c
    - robot_platform/runtime/tests/host/test_actuator_gateway.c
decisions:
  - Port implementations own static device structs and lazy-bind on first call
  - Replaced platform_device_command_t with platform_actuator_command_t in gateway (same typedef)
  - actuator_gateway_init is now a no-op since ports self-init
metrics:
  duration: ~7min
  completed: 2026-04-02
---

# Phase v2-03 Plan 01: Rewire BSP Port Implementations Summary

BSP port implementations (HW and SITL) now call device driver bind/ops functions directly instead of delegating through device_layer default API. Each port file owns static device structs with a lazy-bind-on-first-call pattern.

## What Changed

### Task 1: Rewire BSP port implementations
- `bsp_ports_hw.c`: Removed `device_layer.h` include. Added static `platform_imu_device_t`, `platform_remote_device_t`, `platform_motor_device_t` with matching config structs (same values as `device_profile_hw.c`). Lazy-binds via `platform_ports_hw_ensure_bound()` on first port call, then dispatches through device ops directly.
- `bsp_ports_sitl.c`: Same pattern with SITL device configs (null SPI handle, no calibration, null motor config for SITL stub).

### Task 2: Clean actuator_gateway
- Removed `#include "../../device/device_layer.h"` from `actuator_gateway.c`.
- Removed `platform_device_init_default_profile()` call — ports now self-init, so gateway init is a no-op.
- Replaced `platform_device_command_t` with `platform_actuator_command_t` (identical typedef alias) to avoid needing `device_command.h`.
- Updated `test_actuator_gateway.c`: removed the `platform_device_init_default_profile` stub and the init-call-count assertion.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] platform_device_command_t not visible after removing device_layer.h**
- Found during: Task 2
- Issue: `actuator_gateway.c` used `platform_device_command_t` which was pulled in transitively via `device_layer.h`. After removing that include, the type was undefined.
- Fix: Replaced with `platform_actuator_command_t` (same type via typedef in `device_command.h`), which is already available through `actuator_gateway.h` -> `actuator_command.h`.
- Files modified: `actuator_gateway.c`
- Commit: e5f062b6

## Verification

All 12 CTest targets pass:
- test_message_center, test_actuator_gateway, test_balance_safety_path, test_balance_app_startup
- test_device_profile_safety_seams, test_device_profile_sitl_runtime_bindings
- test_safety_mapping, test_safety_sensor_faults, test_safety_arming
- test_safety_saturation, test_safety_wheel_leg, test_bsp_ports_compile

## Commits

| Task | Commit | Message |
|------|--------|---------|
| 1 | dbb2a039 | refactor(bsp): rewire port implementations to call drivers directly |
| 2 | e5f062b6 | refactor(control): remove device_layer references from actuator_gateway |

## Next Phase Readiness

Port implementations no longer depend on device_layer. The device_layer module is now only referenced by:
- `balance_safety_host_runtime` (CMake source list)
- `balance_chassis_device` library (HW/SIM targets)
- `test_device_profile_safety_seams` and `test_device_profile_sitl_runtime_bindings` tests
- `device_layer_stubs.c` (unused)

Plans 02/03 can proceed to remove device_layer from the build and delete the module.
