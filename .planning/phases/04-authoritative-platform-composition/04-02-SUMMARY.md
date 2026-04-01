---
phase: 04-authoritative-platform-composition
plan: "02"
subsystem: runtime
tags: [control, app-composition, host-tests, sitl, runtime-chain]
requires:
  - phase: 04-authoritative-platform-composition
    provides: "Authoritative app startup seam shared by hardware and SITL from plan 04-01"
provides:
  - "Control-owned runtime task registration for the observe -> chassis-control -> execution chain"
  - "Control-owned chassis task entrypoint with app-side compatibility shell preserved during migration"
  - "Neutral control-task timing seam that removes direct app-config includes from control task sources"
  - "Host safety and Phase 3 runtime-binding proof kept green on the same balance_chassis chain"
affects: [phase4-plan3, runtime/control, host-safety-verification, phase3-verification]
tech-stack:
  added: []
  patterns: [control-owned task registration, compatibility-shell migration, control config seam]
key-files:
  created:
    - robot_platform/runtime/control/controllers/chassis_control_task.h
    - robot_platform/runtime/control/controllers/chassis_control_task.c
    - robot_platform/runtime/control/task_registry/control_task_registry.h
    - robot_platform/runtime/control/task_registry/control_task_registry.c
    - robot_platform/runtime/control/control_config/control_task_params.h
  modified:
    - robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.c
    - robot_platform/runtime/app/balance_chassis/app_bringup/chassis_task.h
    - robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c
    - robot_platform/runtime/control/state/observe_task.c
    - robot_platform/runtime/control/execution/motor_control_task.c
    - robot_platform/runtime/tests/host/test_support/balance_safety_harness.h
    - robot_platform/CMakeLists.txt
key-decisions:
  - "Keep remote task registration in app composition while moving the observe/chassis/motor registration chain under runtime/control ownership."
  - "Preserve legacy chassis-task helpers through a compatibility shell so existing host harnesses keep driving the authoritative runtime chain."
  - "Route control task timing through a neutral control header that still derives values from the current balance_chassis proving path."
patterns-established:
  - "App composes project-owned ingress tasks, then delegates control-chain registration through platform_control_start_tasks()."
  - "Control-owned task code can migrate without breaking tests by re-exporting the runtime struct and helpers through a temporary app shell."
requirements-completed: [ARCH-01, ARCH-04]
duration: 6min
completed: 2026-04-01
---

# Phase 04 Plan 02: Authoritative Platform Composition Summary

**Control now owns the balance_chassis observation, chassis-control, and execution task chain while app remains a thin composition seam**

## Performance

- **Duration:** 6 min
- **Started:** 2026-04-01T12:08:26Z
- **Completed:** 2026-04-01T12:14:55Z
- **Tasks:** 2
- **Files modified:** 12

## Accomplishments

- Added `platform_control_start_tasks()` and `platform_chassis_control_task()` so the runtime chain has an explicit owner under `runtime/control`.
- Reduced direct control-to-app coupling by introducing `control_task_params.h` and updating the host harness to include the control-owned chassis task surface.
- Kept both the host safety regression and `verify phase3 --case runtime_binding` green on the same `balance_chassis` proof path after the ownership move.

## Task Commits

Each task was committed atomically:

1. **Task 1: Create control-owned runtime task registration and migrate the chassis-control entrypoint** - `00b18ce8` (`feat`)
2. **Task 2: Reduce direct app-config coupling from control-owned runtime tasks and keep the proving path green** - `069d3a03` (`feat`)

## Files Created/Modified

- `robot_platform/runtime/control/task_registry/control_task_registry.c` - Registers the control-owned INS, observe, chassis-control, and motor-control tasks.
- `robot_platform/runtime/control/controllers/chassis_control_task.c` - Holds the authoritative chassis-control task entrypoint and preserved helper flow.
- `robot_platform/runtime/app/balance_chassis/app_bringup/task_registry.c` - Keeps app registration limited to project composition and remote ingress.
- `robot_platform/runtime/control/control_config/control_task_params.h` - Provides the neutral include seam for control task timing values.
- `robot_platform/runtime/tests/host/test_support/balance_safety_harness.h` - Points the host harness at the control-owned chassis task surface.
- `robot_platform/CMakeLists.txt` - Builds the new control-owned sources into SITL and host runtime targets.

## Decisions Made

- Moved runtime-chain task creation into `runtime/control/task_registry` instead of leaving task registration mixed into app bring-up.
- Kept the app-side `Chassis_task()` symbol as a wrapper only, so migration stays low-risk while file placement catches up to ownership.
- Let the new control config seam include current `balance_chassis` timing values rather than inventing a broader config framework during this phase.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Removed control task registry from the host-runtime static library**
- **Found during:** Task 1
- **Issue:** Host harness targets do not compile CMSIS-RTOS thread-definition APIs, so linking the new task registry into the host runtime caused build failures unrelated to the proof path itself.
- **Fix:** Kept `control_task_registry.c` in the authoritative app/SITL source list, but excluded it from the host-runtime library because host tests drive init/prepare/step helpers directly instead of registering FreeRTOS tasks.
- **Files modified:** `robot_platform/CMakeLists.txt`
- **Verification:** `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R "test_balance_safety_path|test_device_profile_sitl_runtime_bindings"`
- **Committed in:** `00b18ce8`

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** The auto-fix kept host verification aligned with the existing deterministic harness while preserving the authoritative runtime-control ownership in compiled app targets.

## Issues Encountered

- The first Task 1 build attempted to compile the new task registry under the host test stub `cmsis_os.h`, which only exposes delay/tick helpers. Narrowing the host-runtime source set resolved that without weakening the runtime ownership change.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 04-03 can now document and reinforce the blessed bring-up path on top of a concrete ownership split in code.
- The authoritative runtime chain remains observable through both the Phase 2 host harness and the Phase 3 SITL verification path.

## Known Stubs

None.

## Self-Check: PASSED

- `FOUND:.planning/phases/04-authoritative-platform-composition/04-02-SUMMARY.md`
- `FOUND:00b18ce8`
- `FOUND:069d3a03`

---
*Phase: 04-authoritative-platform-composition*
*Completed: 2026-04-01*
