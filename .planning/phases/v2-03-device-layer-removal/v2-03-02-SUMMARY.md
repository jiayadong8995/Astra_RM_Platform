---
phase: v2-03
plan: "02"
subsystem: runtime
tags: [device-layer, deletion, cmake, bsp-drivers]
dependency-graph:
  requires: [v2-03-01]
  provides: [device-layer-removed, drivers-relocated-to-bsp]
  affects: [v2-03-03, v2-04]
tech-stack:
  added: []
  patterns: [bsp-drivers-directory]
key-files:
  created:
    - robot_platform/runtime/bsp/device_types.h
    - robot_platform/runtime/bsp/drivers/imu/imu_device.h
    - robot_platform/runtime/bsp/drivers/imu/bmi088_device.h
    - robot_platform/runtime/bsp/drivers/imu/bmi088_device_hw.c
    - robot_platform/runtime/bsp/drivers/imu/bmi088_device_sitl.c
    - robot_platform/runtime/bsp/drivers/remote/remote_device.h
    - robot_platform/runtime/bsp/drivers/remote/dbus_remote_device.h
    - robot_platform/runtime/bsp/drivers/remote/dbus_remote_device_hw.c
    - robot_platform/runtime/bsp/drivers/remote/dbus_remote_device_sitl.c
    - robot_platform/runtime/bsp/drivers/actuator/motor/motor_device.h
    - robot_platform/runtime/bsp/drivers/actuator/motor/motor_actuator_device.h
    - robot_platform/runtime/bsp/drivers/actuator/motor/motor_actuator_device_hw.c
    - robot_platform/runtime/bsp/drivers/actuator/motor/motor_actuator_device_sitl.c
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/bsp/ports.h
    - robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_ports_hw.c
    - robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/bsp_uart.c
    - robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/can_bsp.c
    - robot_platform/runtime/bsp/sitl/bsp_ports_sitl.c
    - robot_platform/runtime/bsp/sitl/BMI088driver_sitl.c
    - robot_platform/runtime/bsp/sitl/remote_control_sitl.c
    - robot_platform/runtime/bsp/sitl/dm4310_drv_sitl.c
    - robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.h
  deleted:
    - robot_platform/runtime/device/ (entire directory, 29+ files)
    - robot_platform/runtime/tests/host/test_device_profile_safety_seams.c
    - robot_platform/runtime/tests/host/test_device_profile_sitl_runtime_bindings.c
    - robot_platform/runtime/tests/host/test_support/device_layer_stubs.c
    - robot_platform/runtime/tests/host/test_support/device_layer_stubs.h
decisions:
  - id: D-v2-03-02-01
    summary: "Relocated driver adapters to bsp/drivers/ instead of control/contracts/"
    rationale: "Driver adapters (bmi088_device, dbus_remote_device, motor_actuator_device) are BSP-level code that bind hardware drivers to port implementations. bsp/drivers/ is the natural home."
  - id: D-v2-03-02-02
    summary: "Deleted test_device_profile_safety_seams and test_device_profile_sitl_runtime_bindings"
    rationale: "Both tests exercised device_layer concepts (init_default_profile, layer init_profile). With device_layer deleted, these tests have no subject. Port-level testing is covered by test_bsp_ports_compile and test_actuator_gateway."
  - id: D-v2-03-02-03
    summary: "Merged CMake cleanup into Task 1 commit"
    rationale: "CMake changes were required to verify the build after deletion. Splitting into a separate commit would leave an unbuildable intermediate state."
metrics:
  duration: "~11 minutes"
  completed: "2026-04-02"
---

# Phase v2-03 Plan 02: Delete Device Layer and Clean CMake Summary

Deleted the entire `runtime/device/` directory (29+ files of vtable indirection, profile selection, and device_layer orchestration) and relocated the still-needed driver adapters and low-level drivers to `bsp/drivers/`.

## What Was Done

The device_layer was dead code after v2-03-01 rewired BSP ports to call drivers directly. This plan removed it:

1. Moved `device_types.h` to `bsp/device_types.h` (defines `platform_device_result_t` used by ports.h)
2. Relocated driver adapter headers and implementations to `bsp/drivers/{imu,remote,actuator}/` — these are the bind functions that BSP port implementations still call
3. Relocated low-level driver code (bmi088, dbus, dm4310) to `bsp/drivers/` subdirectories
4. Updated all include paths across BSP port files, SITL stubs, and app code
5. Removed the `balance_chassis_device` CMake library target and all `PLATFORM_DEVICE_*` variables
6. Deleted dead tests that exercised device_layer concepts
7. Deleted unused `device_layer_stubs.c/h`

## Files Deleted (from runtime/device/)

- `device_layer.c/h` — vtable orchestration layer (336 lines)
- `device_profile.h`, `device_profile_hw.c`, `device_profile_sitl.c` — profile selection
- `actuator_device.h` — unused actuator abstraction
- 4 README files
- All driver files (relocated, not lost)

## Verification

- Host tests: 10/10 pass (down from 12 — removed 2 dead device_layer tests)
- SITL build: compiles and links successfully
- Zero stale `device/` references in CMakeLists.txt or runtime code

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] CMake cleanup merged into Task 1**
- **Found during:** Task 1
- **Issue:** Cannot verify device/ deletion without updating CMake simultaneously
- **Fix:** Combined file deletion, driver relocation, and CMake cleanup into a single atomic commit
- **Commit:** 02f6b0d8

## Next Phase Readiness

Device layer is fully removed. The runtime architecture is now: `bsp/ports.h` -> `bsp/drivers/` -> low-level drivers. No vtable indirection remains. Ready for v2-03-03 (if applicable) or Phase 4 consolidation.
