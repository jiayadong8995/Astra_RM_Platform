---
phase: 02
slug: host-safety-control-verification
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-31
---

# Phase 02 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | CTest 3.23.0 for host C targets; Python `unittest` for CLI and runner regression |
| **Config file** | `robot_platform/CMakeLists.txt` for host C targets; none for Python `unittest` |
| **Quick run command** | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| **Full suite command** | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v && python3 -m unittest robot_platform.sim.tests.test_runner -v && ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| **Estimated runtime** | ~30 seconds |

---

## Sampling Rate

- **After every task commit:** Run `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **After every plan wave:** Run `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v && python3 -m unittest robot_platform.sim.tests.test_runner -v && ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **Before `$gsd-verify-work`:** Full suite must be green
- **Max feedback latency:** 30 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 02-01-01 | 01 | 1 | HOST-02 | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R "test_balance_.*|test_device_profile_.*"` | ❌ W0 | ⬜ pending |
| 02-01-02 | 01 | 1 | HOST-03 | host C + Python orchestration | `python3 -m robot_platform.tools.platform_cli.main verify phase2 --project balance_chassis --report build/verification_reports/phase2_balance_chassis.json` | ❌ W0 | ⬜ pending |
| 02-02-01 | 02 | 2 | SAFE-01 | host C | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_mapping` | ❌ W0 | ⬜ pending |
| 02-02-02 | 02 | 2 | SAFE-02 | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_sensor_faults` | ❌ W0 | ⬜ pending |
| 02-03-01 | 03 | 3 | SAFE-03 | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_arming` | ❌ W0 | ⬜ pending |
| 02-03-02 | 03 | 3 | SAFE-04 | host C | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_saturation` | ❌ W0 | ⬜ pending |
| 02-04-01 | 04 | 4 | SAFE-05 | host C + Python orchestration | `python3 -m robot_platform.tools.platform_cli.main verify phase2 --project balance_chassis --case stale_command` | ❌ W0 | ⬜ pending |
| 02-04-02 | 04 | 4 | SAFE-06 | host C integration | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_safety_wheel_leg` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `robot_platform/runtime/tests/host/test_balance_safety_path.c` — authoritative current-path safety cases
- [ ] `robot_platform/runtime/tests/host/test_device_profile_safety_seams.c` — IMU/remote/profile injection seams
- [ ] `robot_platform/runtime/tests/host/test_safety_mapping.c` — SAFE-01 mapping and direction cases
- [ ] `robot_platform/runtime/tests/host/test_safety_sensor_faults.c` — SAFE-02 stale/invalid sensor cases
- [ ] `robot_platform/runtime/tests/host/test_safety_arming.c` — SAFE-03 invalid arming and state transitions
- [ ] `robot_platform/runtime/tests/host/test_safety_saturation.c` — SAFE-04 saturation oracle coverage
- [ ] `robot_platform/runtime/tests/host/test_safety_wheel_leg.c` — SAFE-06 narrow wheel-leg danger signatures
- [ ] `robot_platform/tools/platform_cli/main.py` — `verify phase2` command support
- [ ] `robot_platform/tools/platform_cli/tests/test_main.py` — Phase 2 report schema and failure-mode coverage
- [ ] `robot_platform/sim/tests/test_runner.py` — verdict artifact parsing coverage if reused by Phase 2 reports

---

## Manual-Only Verifications

All phase behaviors should be automated. No manual-only verification is planned for Phase 2.

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 30s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
