---
phase: 03-fake-link-runtime-proof
plan: 01
subsystem: testing
tags: [sitl, udp, device-profile, host-tests, fake-link]
requires:
  - phase: 02-host-safety-control-verification
    provides: authoritative host-tested runtime chain for remote, observe, chassis, and motor tasks
provides:
  - SITL profile binds IMU and remote devices through runtime-backed UDP adapters
  - Dedicated host regression fails on SITL binding drift back to stub-only adapters
  - SITL profile initialization stays host-testable while motor dispatch preserves lazy UDP setup
affects: [03-02, verify-phase3, sitl, device-layer]
tech-stack:
  added: []
  patterns: [config-backed SITL device profiles, host regression for adapter binding drift, lazy UDP socket init for host-safe SITL startup]
key-files:
  created:
    - robot_platform/runtime/tests/host/test_device_profile_sitl_runtime_bindings.c
    - robot_platform/runtime/bsp/sitl/remote_control_sitl.c
  modified:
    - robot_platform/CMakeLists.txt
    - robot_platform/runtime/device/device_profile_sitl.c
    - robot_platform/runtime/device/imu/bmi088_device_sitl.c
    - robot_platform/runtime/device/remote/dbus_remote_device_sitl.c
    - robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c
key-decisions:
  - "Mirror the hardware profile pattern in SITL by binding static config objects instead of null-config stub wrappers."
  - "Keep the Phase 3 proof narrow: verify runtime-backed IMU and remote bindings explicitly, while leaving motor control flow unchanged apart from lazy socket setup needed for host init."
patterns-established:
  - "SITL device bindings should expose concrete runtime-backed names and non-null config contexts so host tests can detect adapter drift."
  - "Host binding regressions may prove init-time adapter truth without requiring active transport traffic."
requirements-completed: [LINK-01]
duration: 5min
completed: 2026-04-01
---

# Phase 03 Plan 01: Fake-Link Runtime Proof Summary

**SITL IMU and remote bindings now use UDP-backed runtime adapters, with a dedicated host regression guarding against stub-only drift**

## Performance

- **Duration:** 5 min
- **Started:** 2026-04-01T01:08:36+08:00
- **Completed:** 2026-04-01T01:12:32+08:00
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments
- Added a dedicated host regression that initializes the SITL device profile and asserts the runtime-backed IMU and remote adapter names and ops tables.
- Replaced the SITL stub-only IMU and remote wrappers with config-backed adapters that use `BMI088_Init`/`BMI088_Read` and `get_remote_control_point`/`RC_data_is_error`.
- Added a UDP-backed SITL remote BSP surface and updated host/SITL build source lists so the fake-link ingress path is prepared for Phase 03 verification work.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add regression coverage for runtime-backed SITL profile binding** - `c3a15719` (`test`)
2. **Task 2: Rebind SITL IMU and remote ingress to runtime-backed adapters** - `d3deb4f8` (`feat`)

## Files Created/Modified
- `robot_platform/runtime/tests/host/test_device_profile_sitl_runtime_bindings.c` - Dedicated host regression for SITL runtime-backed adapter binding.
- `robot_platform/runtime/bsp/sitl/remote_control_sitl.c` - UDP-backed SITL remote ingress exposing `get_remote_control_point` and `RC_data_is_error`.
- `robot_platform/runtime/device/device_profile_sitl.c` - Static SITL config objects binding runtime-backed IMU and remote adapters.
- `robot_platform/runtime/device/imu/bmi088_device_sitl.c` - Runtime-backed SITL BMI088 adapter with UDP-fed reads and timestamped samples.
- `robot_platform/runtime/device/remote/dbus_remote_device_sitl.c` - Runtime-backed SITL remote adapter mirroring the hardware DBUS binding contract.
- `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c` - Lazy socket init during dispatch so host-side profile init remains green.
- `robot_platform/CMakeLists.txt` - Host-test and SITL source lists updated for the new regression and SITL remote BSP.

## Decisions Made
- Used the hardware profile’s config-object binding pattern for SITL so adapter truth is represented in `device_profile_sitl.c` instead of hidden in stub wrappers.
- Kept the guard focused on adapter-binding honesty, not simulator fidelity, so the regression proves `LINK-01` without introducing a parallel controller path.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Deferred SITL motor socket enforcement until command dispatch**
- **Found during:** Task 2 (Rebind SITL IMU and remote ingress to runtime-backed adapters)
- **Issue:** `platform_device_layer_init_profile(..., PLATFORM_DEVICE_BACKEND_PROFILE_SITL)` failed under host tests because the SITL motor device tried to create its UDP socket during init.
- **Fix:** Kept profile init green by treating socket unavailability as non-fatal during motor init and retaining socket setup in the existing write path.
- **Files modified:** `robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c`
- **Verification:** `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_device_profile_sitl_runtime_bindings`
- **Committed in:** `d3deb4f8` (part of Task 2 commit)

---

**Total deviations:** 1 auto-fixed (1 blocking)
**Impact on plan:** The deviation was necessary to let the SITL binding proof initialize on host without changing the intended motor dispatch path.

## Issues Encountered
- The new regression initially failed before reaching the IMU and remote assertions because SITL motor init attempted socket setup during profile initialization in the host-test environment.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- Phase 03-02 can now rely on explicit runtime-backed SITL adapter names and a dedicated regression that fails on binding drift.
- The new SITL remote BSP listens on UDP port `9004`; later verification/orchestration work needs to declare and drive that input explicitly.

## Self-Check: PASSED
- Verified `.planning/phases/03-fake-link-runtime-proof/03-01-SUMMARY.md` exists.
- Verified task commits `c3a15719` and `d3deb4f8` exist in git history.

---
*Phase: 03-fake-link-runtime-proof*
*Completed: 2026-04-01*
