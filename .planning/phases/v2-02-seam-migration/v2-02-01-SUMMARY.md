---
phase: v2-02
plan: "01"
subsystem: bsp-test-seam
tags: [ports_fake, test-hooks, injection-api, bridge-strategy]
dependency-graph:
  requires: [v2-01-01, v2-01-02]
  provides: [ports-fake-injection-api, test-seam-migration]
  affects: [v2-02-02]
tech-stack:
  added: []
  patterns: [bridge-adapter-pattern]
key-files:
  created:
    - robot_platform/runtime/bsp/ports_fake.h
  modified:
    - robot_platform/runtime/bsp/ports_fake.c
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/tests/host/test_balance_safety_path.c
    - robot_platform/runtime/tests/host/test_safety_mapping.c
    - robot_platform/runtime/tests/host/test_safety_sensor_faults.c
    - robot_platform/runtime/tests/host/test_safety_arming.c
    - robot_platform/runtime/tests/host/test_safety_saturation.c
    - robot_platform/runtime/tests/host/test_safety_wheel_leg.c
    - robot_platform/runtime/tests/host/test_device_profile_safety_seams.c
decisions:
  - id: seam-bridge
    summary: "ports_fake_set_hooks installs bridge adapters into device_layer test hooks so tasks still calling device_layer default API get routed through ports_fake"
  - id: bsp-ports-sitl-swap
    summary: "Replaced bsp_ports_sitl.c with ports_fake.c in balance_safety_host_runtime to avoid duplicate port symbols"
  - id: device-seams-hybrid
    summary: "test_device_profile_safety_seams uses hybrid approach — port hooks via ports_fake, init_default_profile via device_layer (not a port concept)"
metrics:
  duration: ~8min
  completed: 2026-04-02
---

# Phase v2-02 Plan 01: BSP Port Fake Injection API + Test Migration Summary

Migrated test injection from device_layer hooks to BSP port fakes with a bridge strategy that keeps both paths working during transition.

## What Was Done

### Task 1: ports_fake injection API
Created `ports_fake.h` with function pointer hook typedefs matching the device_layer test hook signatures (`read_imu`, `read_remote`, `read_feedback`, `write_command` — all with `void *context`). Added `set_hooks`, `reset_hooks`, and `last_command` accessor.

Updated `ports_fake.c` to dispatch through hooks (defaulting to zeroed-OK when no hooks set). The bridge strategy installs adapter functions into `platform_device_set_test_hooks` so that tasks still calling device_layer default API (`platform_device_read_default_remote`, etc.) get routed through the ports_fake hooks.

Replaced `bsp_ports_sitl.c` with `ports_fake.c` in `balance_safety_host_runtime` to avoid duplicate port symbol definitions. Updated `test_bsp_ports_compile` to link against `balance_safety_host_runtime`.

### Task 2: Migrate 7 test files
Replaced `device_layer.h` include with `ports_fake.h`, `platform_device_test_hooks_t` with `platform_ports_fake_hooks_t`, and `set/reset_test_hooks` with `ports_fake_set/reset_hooks` across all 7 files.

`test_device_profile_safety_seams` uses a hybrid approach: port hooks via ports_fake, `init_default_profile` via device_layer directly (init is a device_layer concept, not a port).

## Verification

All 12 CTest targets pass (100%), including the critical canary `test_balance_safety_path`.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] ports_fake.h missing device_command.h include**
- Found during: Task 1
- Issue: `platform_device_command_t` unknown in ports_fake.h (defined in `device_command.h`, not transitively included by `ports.h`)
- Fix: Added `#include "device_command.h"` to ports_fake.h
- Commit: 22f4cdef

**2. [Rule 3 - Blocking] test_bsp_ports_compile link failure**
- Found during: Task 1
- Issue: Standalone test linked only ports_fake.c which now depends on device_layer symbols
- Fix: Switched test_bsp_ports_compile to link against balance_safety_host_runtime; replaced bsp_ports_sitl.c with ports_fake.c in the runtime library
- Commit: 22f4cdef

## Next Phase Readiness

Plan v2-02-02 (task migration to call ports directly) can proceed. The bridge ensures backward compatibility — tasks still call device_layer, but test injection now flows through ports_fake.
