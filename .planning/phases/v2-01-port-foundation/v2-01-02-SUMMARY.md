---
phase: v2-01
plan: "02"
subsystem: bsp
tags: [bsp, ports, device-layer, cmake]
dependency-graph:
  requires: [v2-01-01]
  provides: [bsp-port-interfaces, fake-port-implementation]
  affects: [v2-02, v2-03]
tech-stack:
  added: []
  patterns: [port-interface-delegation]
key-files:
  created:
    - robot_platform/runtime/bsp/ports.h
    - robot_platform/runtime/bsp/ports_fake.c
    - robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_ports_hw.c
    - robot_platform/runtime/bsp/sitl/bsp_ports_sitl.c
    - robot_platform/runtime/tests/host/test_bsp_ports_compile.c
  modified:
    - robot_platform/CMakeLists.txt
decisions:
  - HW and SITL port implementations delegate to device_layer default API as transitional bridge
  - Fake implementation returns OK with zeroed output for isolated test use
  - motor_write_command wraps motor_command_set into actuator_command_t for device_layer compatibility
metrics:
  duration: ~3min
  completed: 2026-04-01
---

# Phase v2-01 Plan 02: BSP Port Interfaces Summary

BSP port header (`ports.h`) declaring 4 functions (`platform_imu_read`, `platform_remote_read`, `platform_motor_write_command`, `platform_motor_read_feedback`) with HW, SITL, and test-fake implementations that delegate to device_layer.

## Tasks Completed

| Task | Name | Commit | Status |
|------|------|--------|--------|
| 1 | Create BSP port header and three implementation files | 6a0d7056 | Done |
| 2 | Wire BSP port files into CMake and verify all targets build | 1d67e0dd | Done |

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added device_layer include paths to balance_chassis_bsp_seed**

- **Found during:** Task 2
- **Issue:** `bsp_ports_hw.c` includes `device_layer.h` which transitively includes device subdirectory headers. The `balance_chassis_bsp_seed` target lacked `${PLATFORM_DEVICE_DIR}` and its subdirectories in its include paths.
- **Fix:** Added `${PLATFORM_DEVICE_DIR}`, `${PLATFORM_DEVICE_DIR}/imu`, `${PLATFORM_DEVICE_DIR}/remote`, `${PLATFORM_DEVICE_DIR}/actuator`, `${PLATFORM_DEVICE_DIR}/actuator/motor`, and `${PLATFORM_DEVICE_IMU_DIR}` to `balance_chassis_bsp_seed` include directories.
- **Files modified:** `robot_platform/CMakeLists.txt`
- **Commit:** 1d67e0dd

## Verification

- All 12 host tests pass (11 original + 1 new `test_bsp_ports_compile`)
- `test_bsp_ports_compile` exercises all 4 port functions through the fake implementation
- Port symbols coexist with device_layer — no conflicts

## Next Phase Readiness

Phase v2-01 is now complete. Both plans delivered:
- Plan 01: Unified command type (`platform_actuator_command_t` / `platform_device_command_t`)
- Plan 02: BSP port interfaces with HW/SITL/fake implementations

Phase v2-02 can proceed to migrate task code from device_layer calls to port calls.
