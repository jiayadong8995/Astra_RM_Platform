---
phase: 1
slug: contracts-and-verification-foundation
status: draft
nyquist_compliant: false
wave_0_complete: false
created: 2026-03-30
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Python `unittest` + CMake/CTest host C tests |
| **Config file** | none — Wave 0 adds host test targets to `robot_platform/CMakeLists.txt` |
| **Quick run command** | `ctest --test-dir build/robot_platform_host_tests --output-on-failure` |
| **Full suite command** | `python3 -m robot_platform.tools.platform_cli.main test sim` plus `ctest --test-dir build/robot_platform_host_tests --output-on-failure` plus `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` |
| **Estimated runtime** | ~15 seconds |

---

## Sampling Rate

- **After every task commit:** Run `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **After every wave touching sim/CLI:** Run `python3 -m robot_platform.tools.platform_cli.main test sim`, `python3 -m unittest robot_platform.sim.tests.test_runner -v`, and `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v`
- **After every plan wave:** Run `ctest --test-dir build/robot_platform_host_tests --output-on-failure`
- **Before `$gsd-verify-work`:** Full suite must be green
- **Max feedback latency:** 20 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|-----------|-------------------|-------------|--------|
| 1-01-01 | 01 | 1 | HOST-01 | unit | `cmake -S robot_platform -B build/robot_platform_host_tests -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=robot_platform/cmake/toolchains/linux-gcc.cmake -DPLATFORM_TARGET_HW=OFF -DPLATFORM_TARGET_SIM=OFF -DPLATFORM_HOST_TESTS=ON -DPLATFORM_HOST_TEST_SANITIZERS=ON && cmake --build build/robot_platform_host_tests --target test_message_center -j4` | ✅ | ⬜ pending |
| 1-01-02 | 01 | 1 | HOST-04 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_message_center` | ✅ | ⬜ pending |
| 1-02-01 | 02 | 2 | ARCH-02 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_message_center` | ✅ | ⬜ pending |
| 1-02-02 | 02 | 2 | ARCH-02 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_message_center` | ✅ | ⬜ pending |
| 1-04-01 | 04 | 3 | PIPE-02 | integration | `python3 -m unittest robot_platform.sim.tests.test_runner -v` | ✅ | ⬜ pending |
| 1-04-02 | 04 | 3 | PIPE-02 | integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` | ✅ | ⬜ pending |
| 1-04-03 | 04 | 3 | PIPE-02 | manual integration | `python3 -m robot_platform.tools.platform_cli.main verify phase1 --project balance_chassis --report build/verification_reports/phase1_balance_chassis.json --smoke-duration 1.0` | ✅ | ⬜ pending |
| 1-05-01 | 05 | 4 | PIPE-03 | integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` | ✅ | ⬜ pending |
| 1-05-02 | 05 | 4 | PIPE-03 | integration | `python3 -m unittest robot_platform.tools.platform_cli.tests.test_main -v` | ✅ | ⬜ pending |
| 1-03-01 | 03 | 4 | HOST-01 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_actuator_gateway` | ✅ | ⬜ pending |
| 1-03-02 | 03 | 4 | HOST-04 | unit | `ctest --test-dir build/robot_platform_host_tests --output-on-failure -R test_actuator_gateway` | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Bootstrap Requirements

- [ ] `robot_platform/runtime/tests/host/test_message_center.c` — first-wave host test for transport sizing and registration safety
- [ ] `robot_platform/CMakeLists.txt` — host test target(s), CTest registration, sanitizer flags
- [ ] `build/robot_platform_host_tests/` — dedicated configure/build directory for host test execution
- [ ] `robot_platform/runtime/tests/host/test_actuator_gateway.c` — secondary host seam test added only after the minimum live proof plan is in place
- [ ] `robot_platform/runtime/tests/host/test_support/` — thin stub support only for required actuator seam coverage

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| UDP bridge can open sockets in the intended runtime environment | PIPE-02 | Current sandbox blocks local UDP socket init with `Operation not permitted`, so one live proof must run outside this restricted environment | Run `python3 -m robot_platform.tools.platform_cli.main verify phase1 --project balance_chassis --report build/verification_reports/phase1_balance_chassis.json --smoke-duration 1.0` in the intended dev environment and confirm bridge startup proceeds beyond socket init |
| Minimum live path proves one real observed runtime output | PIPE-02 | Requires environment where SITL + bridge can run fully and emit a passed verification artifact | Inspect `build/verification_reports/phase1_balance_chassis.json` and confirm the `smoke` stage records at least one observed `actuator_command` runtime output, or a machine-readable `blocked` result if the environment still lacks UDP |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Critical path ordering is preserved: 01 -> 02 -> 04 before 03/05
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Bootstrap requirements cover all host-test prerequisites
- [ ] No watch-mode flags
- [ ] Feedback latency < 20s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
