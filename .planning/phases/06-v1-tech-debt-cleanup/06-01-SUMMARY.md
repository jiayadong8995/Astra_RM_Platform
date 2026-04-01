---
phase: 06-v1-tech-debt-cleanup
plan: 01
subsystem: runtime-cli
tags: [c, python, cli, tech-debt, decoupling, sitl]
dependency_graph:
  requires: [phase-01, phase-02, phase-03, phase-04, phase-05]
  provides: [widened-validate-host-tests, control-task-params-decoupling, sitl-newline-fix]
  affects: [06-02]
tech_stack:
  added: []
  patterns: [backward-compatible-macro-aliases, module-level-target-registry]
key_files:
  created: []
  modified:
    - robot_platform/runtime/bsp/sitl/main_sitl.c
    - robot_platform/runtime/control/control_config/control_task_params.h
    - robot_platform/runtime/control/task_registry/control_task_registry.c
    - robot_platform/runtime/control/state/ins_task.c
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
decisions:
  - Add backward-compatible unprefixed macro aliases in control_task_params.h so existing control task code compiles without app_params.h
  - Redirect ins_task.c to control_task_params.h alongside control_task_registry.c to fully eliminate app_params.h from control/
metrics:
  duration: 5min
  completed: 2026-04-01
---

# Phase 6 Plan 1: SITL Newline Fix, Control Decoupling, and Validate Pipeline Widening Summary

Fixed SITL printf double-backslash escaping, decoupled all control/ sources from app_params.h via CONTROL_-prefixed defines with backward-compatible aliases, and widened validate host_tests stage from 6 Phase 2 targets to all 11 CTest targets across Phases 1-4.

## What Was Built

- Fixed 6 printf calls in `main_sitl.c` that had `\\n` (literal backslash-n) instead of `\n` (real newline).
- Replaced passthrough `#include` of `app_params.h` in `control_task_params.h` with direct `CONTROL_`-prefixed defines for stack sizes, priorities, periods, and startup delay.
- Added backward-compatible aliases (`INS_TASK_PERIOD_MS`, `CHASSIS_TASK_PERIOD_MS`, etc.) so existing control task code compiles without changes.
- Redirected `control_task_registry.c` and `ins_task.c` to include `control_task_params.h` instead of `app_params.h`.
- Added `ALL_HOST_TEST_TARGETS` (11 targets) and `ALL_HOST_TEST_REGEX` module-level constants in `main.py`.
- Updated validate Stage 2 to use widened target list instead of PHASE2_CASES-derived subset.
- Added `test_host_tests_stage_runs_all_targets` test verifying the widened target list is passed to `_run_host_ctest`.

## Test Coverage

- All 40 Python tests pass (39 existing + 1 new).
- SITL binary compiles successfully with the decoupled headers.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Redirected ins_task.c from app_params.h to control_task_params.h**

- **Found during:** Task 1 verification
- **Issue:** `ins_task.c` also included `app_params.h` directly, causing the `grep -r 'app_params.h' runtime/control/` verification to fail.
- **Fix:** Redirected its include to `control_task_params.h` and added backward-compatible unprefixed macro aliases.
- **Files modified:** `robot_platform/runtime/control/state/ins_task.c`, `robot_platform/runtime/control/control_config/control_task_params.h`
- **Commit:** 0c9c8579

## Commits

| Task | Commit | Description |
|------|--------|-------------|
| 1 | 0c9c8579 | fix: decouple control_task_registry from app_params.h and fix SITL newlines |
| 2 | a0ca7e7c | feat: widen validate host_tests to all 11 CTest targets |

## Next Phase Readiness

Plan 06-02 can proceed. No blockers introduced. The control/ tree no longer depends on app_params.h, and the validate pipeline now gates on all host test targets.
