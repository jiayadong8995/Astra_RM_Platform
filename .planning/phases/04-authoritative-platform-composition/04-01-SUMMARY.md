---
phase: 04-authoritative-platform-composition
plan: "01"
subsystem: infra
tags: [startup, freertos, sitl, stm32, cmake, host-tests]
requires:
  - phase: 03-fake-link-runtime-proof
    provides: Runtime-backed SITL proof surfaces and the refreshed host-test build tree used as Phase 4 gates
provides:
  - Shared `balance_chassis_app_startup()` authority for project bring-up
  - Converged hardware-generated and SITL startup wiring for business task registration
  - Host regression coverage for the app-startup seam without a project-owned `MX_FREERTOS_Init()`
affects: [04-02-PLAN.md, 04-03-PLAN.md, balance_chassis_sitl, balance_chassis_hw_seed.elf]
tech-stack:
  added: []
  patterns: [Generated startup owns `MX_FREERTOS_Init`, shared app-startup seam, host-tested startup convergence]
key-files:
  created:
    - robot_platform/runtime/app/balance_chassis/app_startup/balance_chassis_app_startup.h
    - robot_platform/runtime/app/balance_chassis/app_startup/balance_chassis_app_startup.c
    - robot_platform/runtime/tests/host/test_balance_app_startup.c
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.h
    - robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c
    - robot_platform/runtime/bsp/sitl/main_sitl.c
    - robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c
    - robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/main.c
key-decisions:
  - "Keep `balance_chassis_app_startup()` thin by delegating straight to `balance_chassis_start_tasks()` so generated startup stays authoritative and project startup only owns task registration."
  - "Remove SITL dependence on a project-owned `MX_FREERTOS_Init()` and treat `freertos_app.c` as non-authoritative compatibility code only."
patterns-established:
  - "Generated and SITL host entrypoints can differ, but both must converge on the same project app-startup API."
  - "Startup regressions should stub the task-registration seam directly instead of rebuilding FreeRTOS ownership in host tests."
requirements-completed: [ARCH-01, ARCH-03]
duration: 19min
completed: 2026-04-01
---

# Phase 4 Plan 1: Authoritative Platform Composition Summary

**Shared `balance_chassis_app_startup()` authority wired into generated FreeRTOS startup and SITL bring-up with host regression coverage**

## Performance

- **Duration:** 19 min
- **Started:** 2026-04-01T11:47:00Z
- **Completed:** 2026-04-01T12:06:21Z
- **Tasks:** 2
- **Files modified:** 9

## Accomplishments

- Added one authoritative `balance_chassis_app_startup()` seam under `runtime/app/balance_chassis/app_startup/`.
- Wired generated `freertos.c` and SITL `main_sitl.c` to the same project startup API while removing project ownership of `MX_FREERTOS_Init()`.
- Added and verified a real `test_balance_app_startup` host target in the refreshed `build/robot_platform_host_tests` tree.

## Task Commits

Each task was committed atomically:

1. **Task 1: Define the authoritative shared app-startup API and startup regression surface** - `a90c609b` (feat)
2. **Task 2: Converge hardware and SITL startup onto the shared app-startup API** - `eb29e36d` (feat)

## Files Created/Modified

- `robot_platform/runtime/app/balance_chassis/app_startup/balance_chassis_app_startup.h` - Declares the shared project startup contract.
- `robot_platform/runtime/app/balance_chassis/app_startup/balance_chassis_app_startup.c` - Implements the thin startup seam by calling `balance_chassis_start_tasks()`.
- `robot_platform/runtime/tests/host/test_balance_app_startup.c` - Verifies the startup API reaches the task-registration seam exactly once.
- `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/freertos.c` - Keeps generated ownership of `MX_FREERTOS_Init()` and delegates project startup through `balance_chassis_app_startup()`.
- `robot_platform/runtime/generated/stm32h7_ctrl_board_raw/Src/main.c` - Restores the explicit `main.c -> MX_FREERTOS_Init() -> scheduler` hardware handoff in user-code blocks.
- `robot_platform/runtime/bsp/sitl/main_sitl.c` - Starts project bring-up through `balance_chassis_app_startup()` before the scheduler.
- `robot_platform/runtime/app/balance_chassis/app_bringup/freertos_app.c` - Demotes the legacy file to a non-authoritative compatibility wrapper.
- `robot_platform/CMakeLists.txt` - Adds the startup host regression and shares app/runtime sources between startup ownership surfaces.

## Decisions Made

- Kept the new startup API narrowly scoped to project task registration instead of reintroducing RTOS ownership in app code.
- Let generated `freertos.c` remain the only authoritative `MX_FREERTOS_Init()` implementation and moved SITL to the shared app-startup seam directly.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added missing hardware-side runtime source linkage for startup convergence**
- **Found during:** Task 2
- **Issue:** The hardware seed target did not link the project runtime/app sources needed once generated `freertos.c` called `balance_chassis_app_startup()`.
- **Fix:** Moved the shared app/runtime source list to common CMake scope and linked it into the hardware and SITL startup compositions.
- **Files modified:** `robot_platform/CMakeLists.txt`
- **Verification:** `balance_chassis_sitl` builds successfully with the converged startup path.
- **Committed in:** `eb29e36d`

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Required for the new startup authority to link against real runtime task registration. No scope creep beyond planned startup convergence.

## Issues Encountered

- The hardware verification command still fails before link because the current `build/robot_platform_hw_make` configuration uses host `cc`, which rejects Cortex-M flags such as `-mthumb` and `-mfloat-abi=hard`. This was logged to `deferred-items.md` as a pre-existing toolchain/configuration blocker.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Phase 04 plan 02 can now treat `balance_chassis_app_startup()` as the single project bring-up seam shared by generated hardware startup and SITL.
- Hardware verification still needs the repo's cross-compiler configuration corrected before `balance_chassis_hw_seed.elf` can serve as a trustworthy gate.

## Verification

- `cmake -S robot_platform -B build/robot_platform_host_tests -G "Unix Makefiles" -DPLATFORM_TARGET_HW=OFF -DPLATFORM_TARGET_SIM=OFF -DPLATFORM_HOST_TESTS=ON -DPLATFORM_HOST_TEST_SANITIZERS=ON && cmake --build build/robot_platform_host_tests --target test_balance_app_startup -j4 && ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_balance_app_startup` ✅
- `cmake -S robot_platform -B build/robot_platform_sitl_make -G "Unix Makefiles" -DPLATFORM_TARGET_HW=OFF -DPLATFORM_TARGET_SIM=ON && cmake --build build/robot_platform_sitl_make --target balance_chassis_sitl -j4` ✅
- `cmake -S robot_platform -B build/robot_platform_hw_make -G "Unix Makefiles" -DPLATFORM_TARGET_HW=ON -DPLATFORM_TARGET_SIM=OFF && cmake --build build/robot_platform_hw_make --target balance_chassis_hw_seed.elf -j4` ❌ pre-existing toolchain issue (`cc` rejects Cortex-M flags)
- `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` ✅

## Self-Check: PASSED

- Found summary file: `.planning/phases/04-authoritative-platform-composition/04-01-SUMMARY.md`
- Found task commit: `a90c609b`
- Found task commit: `eb29e36d`

---
*Phase: 04-authoritative-platform-composition*
*Completed: 2026-04-01*
