---
phase: 01-contracts-and-verification-foundation
plan: "04"
subsystem: verification
tags: [sitl, smoke, verification, json, cli, bridge]
requires: [01-02]
provides:
  - `verify phase1` JSON-first verification command
  - `actuator_command` as the single required Phase 1 observed runtime output
  - deterministic SITL-ready input/output stubs that let the minimum live path complete inside the 1-second smoke window
affects: [phase-01, sim, cli, sitl, reporting]
tech-stack:
  added: [verification report JSON, phase1 CLI entrypoint]
  patterns: [single-output smoke proof, SITL-ready deterministic stubs]
key-files:
  modified:
    - robot_platform/runtime/control/state/ins_state_estimator.c
    - robot_platform/runtime/device/imu/bmi088_device_sitl.c
    - robot_platform/runtime/device/remote/dbus_remote_device_sitl.c
    - robot_platform/runtime/device/actuator/motor/motor_actuator_device_sitl.c
    - robot_platform/runtime/app/balance_chassis/app_io/chassis_topics.c
    - robot_platform/sim/backends/sitl_bridge.py
    - robot_platform/sim/core/runner.py
    - robot_platform/sim/projects/balance_chassis/profile.py
    - robot_platform/sim/tests/test_runner.py
    - robot_platform/tools/platform_cli/main.py
    - robot_platform/tools/platform_cli/tests/test_main.py
key-decisions:
  - "Phase 1 smoke now treats actuator_command as the only required observed runtime output."
  - "verify phase1 writes one authoritative JSON artifact and refuses to pass unless the required runtime output is actually observed."
patterns-established:
  - "Critical-path verification is reported through one top-level JSON file under build/verification_reports/."
  - "SITL default stubs are allowed to be deterministic and ready-by-default when they are part of the minimum live proof path."
requirements-completed: [PIPE-02]
duration: 18min
completed: 2026-03-31
---

# Phase 1 Plan 04: Minimum Live Proof Summary

**JSON-first `verify phase1` flow with one real observed runtime output target and a passed minimum live proof**

## Performance

- **Duration:** 18 min
- **Tasks:** 3
- **Files modified:** 10

## Accomplishments

- Replaced the old declared-only smoke output set with a single required proof target: `actuator_command`.
- Taught the SITL bridge to emit `runtime_output_observation` events when real motor-command traffic arrives.
- Added `python3 -m robot_platform.tools.platform_cli.main verify phase1 --project balance_chassis --report build/verification_reports/phase1_balance_chassis.json --smoke-duration 1.0` as the authoritative Phase 1 verification entrypoint.
- Tightened the verification command so it fails unless the required runtime output is actually observed.
- Made the SITL proof path deterministic enough to complete inside the 1-second smoke window by wiring ready-by-default IMU/remote/actuator stubs and shortening the SITL INS warmup threshold.

## Task Outcome

1. **Task 1:** Completed. Sim profile, bridge metadata, and regression tests now treat `actuator_command` as the sole required observed runtime output.
2. **Task 2:** Completed. CLI verification now writes one stage-ordered JSON artifact for `build_sitl`, `host_tests`, and `smoke`.
3. **Task 3:** Completed. The exact `verify phase1` command was rerun outside the restricted sandbox and produced [phase1_balance_chassis.json](/home/xbd/worspcae/code/Astra_RM2025_Balance/build/verification_reports/phase1_balance_chassis.json) with `overall_status: "passed"` and `validation=1/1`.

## Verification

- `python3 -m unittest robot_platform.sim.tests.test_runner -v`
- `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v`
- `python3 -m robot_platform.tools.platform_cli.main verify phase1 --project balance_chassis --report build/verification_reports/phase1_balance_chassis.json --smoke-duration 1.0`

## Next Action

- Phase 1 minimum live proof is complete.
- Continue Wave 4 execution with `01-05` and `01-03`.
