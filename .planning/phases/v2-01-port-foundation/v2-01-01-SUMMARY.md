---
phase: v2-01-port-foundation
plan: 01
subsystem: runtime-contracts
tags: [c, refactor, contracts, command-unification]
dependency_graph:
  requires: []
  provides: [unified-command-type, indexed-motor-accessors]
  affects: [v2-01-02]
tech_stack:
  added: []
  patterns: [indexed-array-accessors-with-named-enums]
key_files:
  created: []
  modified:
    - robot_platform/runtime/control/contracts/actuator_command.h
    - robot_platform/runtime/control/contracts/device_command.h
    - robot_platform/runtime/control/execution/actuator_gateway.c
    - robot_platform/runtime/control/execution/actuator_gateway.h
    - robot_platform/runtime/device/device_layer.c
    - robot_platform/runtime/device/actuator/motor/motor_device.h
    - robot_platform/runtime/device/actuator/motor/motor_actuator_device_hw.c
    - robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c
    - robot_platform/runtime/control/controllers/balance_controller.c
    - robot_platform/runtime/tests/host/test_actuator_gateway.c
    - robot_platform/runtime/tests/host/test_balance_safety_path.c
    - robot_platform/runtime/tests/host/test_safety_mapping.c
    - robot_platform/runtime/tests/host/test_safety_saturation.c
    - robot_platform/runtime/tests/host/test_safety_arming.c
    - robot_platform/runtime/tests/host/test_safety_sensor_faults.c
    - robot_platform/runtime/tests/host/test_safety_wheel_leg.c
decisions:
  - Replaced named struct fields (left_leg_joint, right_leg_joint, left_wheel, right_wheel) with indexed arrays (joints[N], wheels[N]) plus named enum accessors
  - Made platform_device_command_t a typedef alias for platform_actuator_command_t in device_command.h for backward compatibility
  - Deleted platform_motor_device_command_t (dead type after mapping removal)
  - Deleted platform_map_contract_command from actuator_gateway.c and platform_map_device_command_to_motor_set from device_layer.c
metrics:
  duration: 2min
  completed: 2026-04-01
---

# Phase v2-01 Plan 01: Unify Command Types Summary

Unified `platform_actuator_command_t` and `platform_device_command_t` into a single canonical command type using indexed `joints[PLATFORM_JOINT_MOTOR_COUNT]`/`wheels[PLATFORM_WHEEL_MOTOR_COUNT]` with named enum accessors (PLATFORM_JOINT_LEFT_FRONT, etc.), eliminating ~160 lines of field-by-field mapping code.

## What Was Built

- Restructured `platform_motor_command_set_t` from named fields (`left_leg_joint[2]`, `right_leg_joint[2]`, `left_wheel`, `right_wheel`) to indexed arrays (`joints[4]`, `wheels[2]`) with named enum accessors.
- Reduced `device_command.h` to a single typedef alias: `typedef platform_actuator_command_t platform_device_command_t`.
- Deleted `platform_motor_device_command_t` struct (only used in mapping code).
- Deleted `platform_map_contract_command` (~106 lines) from `actuator_gateway.c` — gateway now passes command directly.
- Deleted `platform_map_device_command_to_motor_set` (~56 lines) from `device_layer.c` — device layer passes motors directly.
- Updated `motor_actuator_device_hw.c`, `motor_actuator_device_sitl.c`, and `balance_controller.c` to use indexed form with enum accessors.
- Updated all 7 affected test files to use indexed form. No test logic or assertions changed.

## Test Coverage

- All 11 CTest targets pass with zero regressions.
- Verified: `grep -r 'left_leg_joint\|right_leg_joint'` returns zero matches.
- Verified: `grep -r 'platform_motor_device_command_t'` returns zero matches.
- Verified: `grep -r 'platform_map_contract_command\|platform_map_device_command_to_motor_set'` returns zero matches.

## Deviations from Plan

None — plan executed exactly as written.

## Commits

| Task | Commit | Description |
|------|--------|-------------|
| 1 | 97135ff5 | refactor(v2-01-01): unify command types with indexed joints[N]/wheels[N] |
| 2 | 69c32502 | test(v2-01-01): update all tests to unified indexed command type |

## Next Phase Readiness

Plan v2-01-02 (BSP port interfaces) can proceed. The unified command type is the foundation for the port interface `platform_motor_write_command()` signature. No blockers introduced.
