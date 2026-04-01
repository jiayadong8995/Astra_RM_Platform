---
phase: v2-02
plan: "02"
subsystem: control-seam
tags: [bsp-ports, device-layer, migration, test-seam]
dependency-graph:
  requires: [v2-02-01]
  provides: [control-tasks-on-ports, ports-fake-standalone]
  affects: [v2-03, v2-04]
tech-stack:
  added: []
  patterns: [port-based-device-access]
key-files:
  created: []
  modified:
    - robot_platform/runtime/control/state/ins_task.c
    - robot_platform/runtime/app/balance_chassis/app_bringup/remote_task.c
    - robot_platform/runtime/control/execution/actuator_gateway.c
    - robot_platform/runtime/control/execution/actuator_gateway.h
    - robot_platform/runtime/bsp/ports_fake.c
    - robot_platform/runtime/tests/host/test_actuator_gateway.c
    - robot_platform/runtime/tests/host/test_device_profile_safety_seams.c
    - robot_platform/runtime/tests/host/test_support/device_layer_stubs.c
    - robot_platform/CMakeLists.txt
decisions:
  - id: D-v2-02-02-01
    summary: "Keep platform_device_init_default_profile call in actuator_gateway with device_layer.h include — HW/SITL port implementations still delegate to device_layer"
  - id: D-v2-02-02-02
    summary: "device_layer_stubs.c/h are now unused by any test target — candidate for deletion in Phase 3"
metrics:
  duration: "7 minutes"
  completed: "2026-04-02"
---

# Phase v2-02 Plan 02: Migrate Control Tasks to BSP Port Calls Summary

Control tasks (ins_task, remote_task, actuator_gateway) now call BSP port functions instead of device_layer default API. ports_fake no longer bridges to device_layer — it intercepts port calls natively.

## Task 1: Migrate control tasks to BSP port calls

Replaced device_layer includes and calls in three control-path files:

- `ins_task.c`: `platform_device_read_default_imu` -> `platform_imu_read`
- `remote_task.c`: `platform_device_read_default_remote` -> `platform_remote_read`
- `actuator_gateway.c`: feedback/command via `platform_motor_read_feedback` / `platform_motor_write_command`
- `actuator_gateway.h`: include switched from `device_layer.h` to `ports.h`

`platform_device_init_default_profile()` kept in `actuator_gateway_init()` with device_layer.h include — HW/SITL port implementations still delegate to device_layer internally.

Fixed `ports_fake.c` `platform_motor_write_command` to capture last command and call hooks (was a no-op stub). Added temporary port stubs to `device_layer_stubs.c` for `test_actuator_gateway` linkage.

Commit: `65a4d094`

## Task 2: Remove device_layer bridge from ports_fake, migrate test_actuator_gateway

- Removed all bridge adapter functions and `device_layer.h` dependency from `ports_fake.c`
- `platform_ports_fake_set_hooks` no longer calls `platform_device_set_test_hooks`
- `platform_ports_fake_reset_hooks` no longer calls `platform_device_reset_test_hooks`
- Migrated `test_actuator_gateway.c` from `device_layer_stubs` to `ports_fake` injection with local hooks
- Updated `test_device_profile_safety_seams.c` to verify port calls directly instead of device_layer bridge
- Reverted `device_layer_stubs.c` to original (no longer linked by any target)
- Updated CMakeLists.txt: `test_actuator_gateway` links `ports_fake.c` instead of `device_layer_stubs.c`

Commit: `8570d1ba`

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] ports_fake motor_write_command was a no-op**

- Found during: Task 1
- Issue: `platform_motor_write_command` in `ports_fake.c` discarded the command and never called hooks
- Fix: Implemented proper wrapping, last-command capture, and hook dispatch
- Files modified: `robot_platform/runtime/bsp/ports_fake.c`
- Commit: `65a4d094`

**2. [Rule 3 - Blocking] test_actuator_gateway link failure after port migration**

- Found during: Task 1
- Issue: `test_actuator_gateway` linked `device_layer_stubs.c` which didn't provide port symbols
- Fix: Added temporary port stubs to `device_layer_stubs.c` (removed in Task 2)
- Files modified: `robot_platform/runtime/tests/host/test_support/device_layer_stubs.c`, `robot_platform/CMakeLists.txt`
- Commit: `65a4d094`

**3. [Rule 1 - Bug] test_device_profile_safety_seams asserted device_layer bridge behavior**

- Found during: Task 2
- Issue: Test verified that ports_fake hooks bridged to device_layer default API — no longer valid after bridge removal
- Fix: Updated test to verify port calls directly via ports_fake hooks
- Files modified: `robot_platform/runtime/tests/host/test_device_profile_safety_seams.c`
- Commit: `8570d1ba`

## Verification

All 12 CTest targets pass:

```
100% tests passed, 0 tests failed out of 12
```

Critical canary `test_balance_safety_path` green throughout.

## Next Phase Readiness

- Control tasks fully decoupled from device_layer default API
- ports_fake is standalone (no device_layer dependency)
- `device_layer_stubs.c/h` unused — ready for cleanup in Phase 3
- `actuator_gateway.c` still includes `device_layer.h` for `platform_device_init_default_profile` — will be removed when device_layer is deleted
