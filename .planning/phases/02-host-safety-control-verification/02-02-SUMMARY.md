---
phase: 02-host-safety-control-verification
plan: "02"
subsystem: testing
tags: [host-tests, safety, balance-chassis, actuator-gateway, remote-intent]
requires:
  - phase: 02-host-safety-control-verification
    provides: deterministic current-path host harness and device/profile injection seams from 02-01
provides:
  - explicit SAFE-01 and SAFE-02 host regressions on mapping and sensor validity
  - explicit SAFE-03 host regressions for recover and jump arming transitions
  - explicit SAFE-04 host regressions and oracle flags for wheel, current, and leg saturation
affects: [phase-02, fake-link-validation, host-verification]
tech-stack:
  added: []
  patterns: [current-path safety oracles, explicit actuator constraint oracle flags]
key-files:
  created:
    - robot_platform/runtime/tests/host/test_safety_mapping.c
    - robot_platform/runtime/tests/host/test_safety_sensor_faults.c
    - robot_platform/runtime/tests/host/test_safety_arming.c
    - robot_platform/runtime/tests/host/test_safety_saturation.c
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/control/controllers/balance_controller.c
    - robot_platform/runtime/control/execution/actuator_gateway.c
    - robot_platform/runtime/control/state/ins_state_estimator.c
    - robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c
    - robot_platform/runtime/control/constraints/actuator_constraints.c
key-decisions:
  - "Treat profile-valid control modes as a hard mapping contract in actuator_gateway instead of a soft validity hint."
  - "Keep recover and jump transitions visible via start while blocking closed-loop control_enable and actuator_enable until the path is safe."
  - "Expose saturation verdicts through actuator constraint oracle flags so tests can prove the clamp happened explicitly."
patterns-established:
  - "Pattern 1: Current-path host tests assert both enable bits and actuator payloads for each safety verdict."
  - "Pattern 2: Constraint layers publish explicit oracle flags when they clamp runtime outputs."
requirements-completed: [SAFE-01, SAFE-02, SAFE-03, SAFE-04]
duration: 25min
completed: 2026-03-31
---

# Phase 2 Plan 02: Hard Safety Oracles Summary

**Hard safety oracles for mapping, sensor validity, arming transitions, and actuator saturation on the live `balance_chassis` host path**

## Performance

- **Duration:** 25 min
- **Started:** 2026-03-31T08:16:00Z
- **Completed:** 2026-03-31T08:41:17Z
- **Tasks:** 2
- **Files modified:** 14

## Accomplishments
- Added current-path host regressions that fail explicitly on invalid mapping, stale/invalid sensor state, blocked arming transitions, and saturation behavior.
- Tightened runtime safety gating so controller output depends on profile-valid mapping, sensor readiness, actuator feedback validity, and non-arming transition states.
- Converted silent saturation into explicit oracle evidence through wheel/current/leg constraint flags.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add hard mapping and sensor-validity oracles on the current path** - `1f0957b7` (feat)
2. **Task 2: Add explicit arming-transition and saturation oracles instead of silent behavior** - `780c31f5` (feat)

## Files Created/Modified
- `robot_platform/runtime/tests/host/test_safety_mapping.c` - SAFE-01 regression for invalid current-path actuator mappings.
- `robot_platform/runtime/tests/host/test_safety_sensor_faults.c` - SAFE-02 regression for pre-ready and degraded sensor states plus deterministic estimator warmup coverage.
- `robot_platform/runtime/tests/host/test_safety_arming.c` - SAFE-03 regression for recover and jump transitions that must stay blocked.
- `robot_platform/runtime/tests/host/test_safety_saturation.c` - SAFE-04 regression for live-path bounded output plus direct wheel and leg oracle checks.
- `robot_platform/runtime/control/execution/actuator_gateway.c` - Marks invalid mappings explicitly and blocks dispatch on profile-invalid control modes.
- `robot_platform/runtime/control/controllers/balance_controller.c` - Gates output on intent control bits, INS readiness, and valid actuator feedback.
- `robot_platform/runtime/app/balance_chassis/app_intent/remote_intent.c` - Separates `start` from closed-loop arming for recover/jump/invalid-state cases.
- `robot_platform/runtime/control/constraints/actuator_constraints.c` - Records explicit saturation oracle flags when clamps fire.

## Decisions Made
- Used control-mode validity as the concrete SAFE-01 mapping oracle for the active `balance_chassis` profile: joints must stay in torque mode and wheels in current mode.
- Kept `start` observable for recover and jump intent, but blocked `control_enable` and `actuator_enable` so invalid transitions cannot escalate into live output.
- Made SAFE-04 explicit with constraint oracle flags rather than inferring protection only from bounded final values.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Linked estimator code into the host safety runtime**
- **Found during:** Task 1
- **Issue:** New SAFE-02 estimator warmup coverage could not link because `ins_state_estimator.c` was not part of the host safety runtime library.
- **Fix:** Added the estimator source to `balance_safety_host_runtime` in `robot_platform/CMakeLists.txt`.
- **Files modified:** `robot_platform/CMakeLists.txt`
- **Verification:** `test_safety_sensor_faults` links and passes under CTest.
- **Committed in:** `1f0957b7`

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** Required to execute the planned SAFE-02 coverage. No scope expansion beyond the intended host safety path.

## Issues Encountered
- Parallel configure/build invocations against the same host test tree intermittently raced and produced stale target/test results. Running configure/build/test serially resolved the issue without code changes.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- SAFE-01 through SAFE-04 now have explicit host-side verdict coverage on the current `balance_chassis` path.
- Phase 02-03 can focus on stale-command and wheel-leg danger-signature closure using the same current-path oracle style.

## Verification

- `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R "test_safety_mapping|test_safety_sensor_faults|test_safety_arming|test_safety_saturation"` ✅

## Self-Check: PASSED

- Found summary file: `.planning/phases/02-host-safety-control-verification/02-02-SUMMARY.md`
- Found task commits: `1f0957b7`, `780c31f5`
- Re-ran Phase 2 plan 02 host safety suite successfully before finalizing summary metadata

---
*Phase: 02-host-safety-control-verification*
*Completed: 2026-03-31*
