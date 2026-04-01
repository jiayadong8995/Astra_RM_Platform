---
phase: v2-04
plan: "02"
subsystem: bsp
tags: [directory-structure, generated-code, cmake]
dependency_graph:
  requires: [v2-04-01]
  provides: [4-layer-runtime-structure, generated-under-bsp]
  affects: [v2-04-03]
tech_stack:
  added: []
  patterns: [generated-code-colocated-with-board-bsp]
key_files:
  created: []
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
    - robot_platform/tools/platform_cli/README.md
  moved:
    - from: robot_platform/runtime/generated/stm32h7_ctrl_board_raw/
      to: robot_platform/runtime/bsp/boards/stm32h7_ctrl_board/generated/
  deleted:
    - robot_platform/runtime/generated/README.md
decisions:
  - id: D-v2-04-02-01
    summary: "GENERATED_RAW_DIR variable moved after PLATFORM_BOARD_BSP_DIR in CMakeLists.txt to fix evaluation order"
metrics:
  duration: ~4min
  completed: "2026-04-02"
---

# Phase v2-04 Plan 02: Move generated/ into bsp/boards/ Summary

Relocated CubeMX generated code from `runtime/generated/stm32h7_ctrl_board_raw/` into `runtime/bsp/boards/stm32h7_ctrl_board/generated/`, eliminating the top-level `generated/` directory and achieving a clean 4-layer runtime structure.

## What Was Done

### Task 1: Move generated/ into bsp/boards/

- Moved `runtime/generated/stm32h7_ctrl_board_raw/` to `runtime/bsp/boards/stm32h7_ctrl_board/generated/`
- Removed empty `runtime/generated/` directory and its README
- Updated `CMakeLists.txt`: reordered variable definitions so `GENERATED_RAW_DIR` is set after `PLATFORM_BOARD_BSP_DIR` (was using undefined variable)
- Updated `platform_cli/main.py` generate and freshness paths
- Updated `platform_cli/tests/test_main.py` (3 occurrences)
- Updated `platform_cli/README.md` documentation

### Task 2: Verify 4-layer structure

Runtime top-level directories after move:
```
runtime/
  app/
  bsp/
  control/
  module/
  tests/
```

Max nesting depth is 5 levels, all under `bsp/` (generated code and hardware drivers) — expected and acceptable. No stray directories found.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] CMake variable evaluation order**
- **Found during:** Task 1
- **Issue:** `GENERATED_RAW_DIR` was defined at line 42 using `${PLATFORM_BOARD_BSP_DIR}`, but `PLATFORM_BOARD_BSP_DIR` wasn't set until line 45. CMake evaluated it as empty, causing `main.h: No such file or directory`.
- **Fix:** Moved `GENERATED_RAW_DIR` definition after `PLATFORM_BOARD_BSP_DIR`
- **Files modified:** `robot_platform/CMakeLists.txt`
- **Commit:** 825fb8c5

## Verification

- Host tests: 10/10 passed (clean rebuild after path change)
- Python CLI tests: 40/40 passed
- No compilation errors or warnings in first-party code

## Decisions Made

| ID | Decision | Rationale |
|----|----------|-----------|
| D-v2-04-02-01 | Reordered CMake variable definitions | `GENERATED_RAW_DIR` must be defined after `PLATFORM_BOARD_BSP_DIR` since it depends on it |

## Commits

| Hash | Message |
|------|---------|
| 825fb8c5 | refactor(bsp): move generated/ into bsp/boards/ for 4-layer structure |
