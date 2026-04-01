---
phase: v2-04
plan: "03"
subsystem: build-system
tags: [cmake, interface-library, validate, closure-loop]
dependency-graph:
  requires: [v2-04-01, v2-04-02]
  provides: [cmake-dedup, validate-pipeline-green]
  affects: []
tech-stack:
  added: []
  patterns: [cmake-interface-library-for-shared-config]
key-files:
  created: []
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/tools/platform_cli/main.py
decisions:
  - id: D-v2-04-03-01
    summary: platform_test_config INTERFACE library carries compile options, sanitizers, and c_std_11 for all test targets
  - id: D-v2-04-03-02
    summary: balance_safety_host_runtime links platform_test_config PUBLIC so downstream test executables inherit config automatically
  - id: D-v2-04-03-03
    summary: Removed stale test_device_profile_* from ALL_HOST_TEST_TARGETS, added test_bsp_ports_compile
metrics:
  duration: ~8min
  completed: 2026-04-02
---

# Phase v2-04 Plan 03: CMake Cleanup and Validate Pipeline Summary

CMake interface library `platform_test_config` eliminates ~110 lines of duplicated compile/sanitizer/feature config across 10 test targets. Full validate pipeline confirmed green after all v2 structural changes.

## Task 1: CMake Interface Library

Created `platform_test_config` INTERFACE library carrying:
- `c_std_11` compile feature
- `-Wall -Wextra -Wpedantic -Wno-unused-parameter -O0 -g3` compile options
- Conditional `-fsanitize=address,undefined` compile and link options

`balance_safety_host_runtime` links it PUBLIC, so the 7 test executables that link the runtime library inherit everything automatically. The 3 standalone tests (`test_message_center`, `test_actuator_gateway`, `test_balance_app_startup`) link it PRIVATE directly.

Net result: 141 lines removed, 32 added. All 10 CTest targets pass.

Commit: `e46278ba`

## Task 2: Full Validate Pipeline

Initial run failed at `host_tests` stage because `ALL_HOST_TEST_TARGETS` still referenced two test targets deleted in v2-03-02 (`test_device_profile_sitl_runtime_bindings`, `test_device_profile_safety_seams`). Fixed by removing stale entries and adding `test_bsp_ports_compile`.

After fix, all 5 validate stages pass:
1. build_sitl - SITL binary builds clean
2. host_tests - 10/10 CTest targets pass
3. python_tests - 40/40 unittest cases pass
4. smoke - SITL smoke session passes with runtime output observations
5. verify_phase3 - Phase 3 verification matrix passes

Closure artifact: `build/closure_reports/closure_balance_chassis.json` with `overall_status: passed`.

Commit: `1184cc68`

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Stale test targets in ALL_HOST_TEST_TARGETS**
- Found during: Task 2
- Issue: `test_device_profile_sitl_runtime_bindings` and `test_device_profile_safety_seams` were deleted in v2-03-02 but still listed in the validate pipeline's target list
- Fix: Removed stale entries, added missing `test_bsp_ports_compile`
- Files modified: `robot_platform/tools/platform_cli/main.py`
- Commit: `1184cc68`

## Commits

| Hash | Message |
|------|---------|
| e46278ba | build(cmake): add interface libraries to eliminate test config duplication |
| 1184cc68 | test(validate): confirm full validate pipeline after v2 restructuring |
